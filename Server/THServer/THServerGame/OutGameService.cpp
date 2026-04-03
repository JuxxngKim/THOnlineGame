#include "GamePch.h"
#include "OutGameService.h"
#include "Configuration.h"
#include "LogicUnitArchive.h"
#include "PacketDistributor.h"
#include "PacketDualMap.h"
#include "PlayerExecutor.h"
#include "OutGameLogicEventor.h"

namespace th
{
	OutGameService::OutGameService() : 
		m_distributor{ NEW(PacketDistributor) }, m_executor{ NEW(PlayerExecutor) },
		/*m_eventor{ nullptr },*/ m_msgBox{ NEW(PacketDualMap) },
		m_nextUpdateTime{ 0 }, m_active{ true },
		m_nextServiceCheckTime{ 0 }
	{
		auto collector = NEW(PacketCollector<PTR<PacketWrapper>>);
		m_msgBox->RegisterCollector(collector);
		m_eventor = NEW(OutGameLogicEventor, collector);
	}

	void OutGameService::Start()
	{
		m_thread = NEW(std::thread, [&]()
			{
				Execute();
			});
	}

	void OutGameService::Send(const PTR<PacketWrapper>& msg)
	{
		if (msg == nullptr) return;
		if (!m_active) return;

		m_msgBox->Push(msg);
	}

	void OutGameService::Stop()
	{
		if (m_active) m_active = false;

		m_executor->Exit();

		if (m_thread != nullptr && m_thread->joinable()) m_thread->join();
	}

	void OutGameService::Execute()
	{
		auto config = &Configuration::GetInstance();
		auto cs = config->Get("ThreadCount", "Logic");
		if (!cs.Valid()) return;

		auto threadCount = 0;
		if (cs.ToLower() == "default") threadCount = std::thread::hardware_concurrency();
		else threadCount = cs.ToInt();

		m_executor->CreateThread(threadCount);
		while (m_active)
		{
			try
			{
				const auto& tickTime = util::TimeUtil::GetInstance().FindTickKSTMs();
				if (tickTime < m_nextUpdateTime)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				m_eventor->Event(tickTime);

				TH_LOG_INFO(0, 0, "=== running ===");

				const auto& messageBox = m_msgBox->Pop();
				if (!messageBox->Empty())
				{
					m_eventor->Collect(messageBox->CollectedPackets);
					m_eventor->Prepare();

					if (0 < LogicUnitArchive::GetInstance().FindCount()) m_distributor->Generator(std::move(messageBox->HostPackets), std::move(messageBox->AccountPackets));
				}

				if (LogicUnitArchive::GetInstance().FindCount() <= 0)
				{
					m_eventor->Arrange();
					messageBox->Clear();
					m_eventor->UpdateServerInfoRedis(tickTime);

					m_nextUpdateTime = tickTime + g_LogicUpdateMs;

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}

				m_executor->Ready(m_distributor);

				m_executor->IsComplete();

				m_eventor->Arrange();
				messageBox->Clear();
				m_distributor->Clear();
				m_eventor->UpdateServerInfoRedis(tickTime);

				m_nextUpdateTime = tickTime + g_LogicUpdateMs;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			catch (const std::runtime_error& err)
			{
				TH_LOG_ERROR(0, 0, "OutGameService runtime error occurred : %", err.what());
			}
		}
	}

	bool OutGameService::IsQuit() const
	{
		if (ServiceProfile::GetInstance().IsLocal()) return false;
		return m_eventor != nullptr && m_eventor->CheckQuit();
	}

	bool OutGameService::IsCrash() const
	{
		if (ServiceProfile::GetInstance().IsLocal()) return false;
		return m_eventor != nullptr && m_eventor->CheckCrash();
	}
}
