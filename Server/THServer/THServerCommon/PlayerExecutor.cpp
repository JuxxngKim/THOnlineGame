#include "CommonPch.h"
#include "PlayerExecutor.h"
#include "LogicUnitArchive.h"
#include "LogicUnit.h"
#include "PacketDistributor.h"

namespace th
{
	PlayerExecutor::PlayerExecutor() : m_ready{ false }, m_unitIdx{ -1 }, m_completeCount{ 0 }, m_distributor{ nullptr }, m_active{ true }
	{
	}

	PlayerExecutor::~PlayerExecutor()
	{
	}

	void PlayerExecutor::Ready(const PTR<PacketDistributor>& distributor)
	{
		if (distributor == nullptr) return;

		std::lock_guard lck(m_lck);
		m_unitIdx = -1;
		m_distributor = distributor;
		m_completeCount = 0;
		m_ready = true;

		m_fieldWait.notify_all();
	}

	void PlayerExecutor::CreateThread(int32_t threadCnt)
	{
		for (int32_t i = 0; i < threadCnt; ++i)
		{
			const auto result = NEW(std::thread, [&]()
				{
					Loop();
				});

			m_threadPool.push_back(result);
		}
	}

	void PlayerExecutor::Loop()
	{
		while (m_active)
		{
			{
				std::unique_lock lck(m_lck);
				m_fieldWait.wait(lck, [&]() { return m_ready || !m_active; });

				if (LogicUnitArchive::GetInstance().FindCount() <= 0)
				{
					m_ready = false;
					m_logicWait.notify_one();
					continue;
				}
			}

			int32_t completionCount = -1;

			const auto& units = LogicUnitArchive::GetInstance().FindAllUnits();
			const auto unitSize = static_cast<int32_t>(units.size());

			auto idx = -1;
			while (true)
			{
				idx = m_unitIdx.fetch_add(1) + 1;
				if (unitSize <= idx || idx < 0) break;

				units[idx]->Execute(m_distributor);
				completionCount = m_completeCount.fetch_add(1) + 1;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (unitSize <= completionCount)
			{
				//TH_LOG_ERROR(0, 0, "complete [%:%]", unitSize, completionCount);

				{
					std::lock_guard lck(m_lck);
					m_ready = false;
				}

				m_logicWait.notify_one();
			}
		}
	}

	bool PlayerExecutor::IsComplete()
	{
		std::unique_lock lck(m_lck);
		m_logicWait.wait(lck, [&]() { return !m_ready; });

		m_distributor = nullptr;
		return true;
	}

	void PlayerExecutor::Exit()
	{
		if (m_active) m_active = false;

		m_fieldWait.notify_all();

		for (const auto& thread : m_threadPool)
		{
			if (thread->joinable()) thread->join();
		}
	}
}
