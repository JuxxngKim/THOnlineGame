#include "CommonPch.h"
#include "LogicUnit.h"
#include "PacketDistributor.h"
#include "ILogicUnit.h"

namespace th
{
	LogicUnit::LogicUnit(const Int64Key id, const int32_t maxLogicUnitPlayerCount) : m_id{ id }, m_maxLogicUnitPlayerCount{ maxLogicUnitPlayerCount }
	{
		//CommonMetric::GetInstance()->IncrementObject("logic_unit");
	}

	LogicUnit::~LogicUnit()
	{
		m_players.clear();
		//CommonMetric::GetInstance()->DecrementObject("logic_unit");
	}

	void LogicUnit::Add(const PTR<IPlayerLogicUnit>& player)
	{
		if (player == nullptr) return;

		m_players.push_back(player);
	}

	void LogicUnit::Remove(const HostID_t& hostID)
	{
		for (size_t i = 0; i < m_players.size(); ++i)
		{
			const auto& player = m_players[i];
			if (player == nullptr) continue;
			if (player->FindHostID() != hostID) continue;

			m_players.erase(m_players.begin() + i);
			break;
		}
	}

	void LogicUnit::Execute(const PTR<PacketDistributor>& distributor) const
	{
		if (distributor == nullptr) return;

		for (const auto& player : m_players)
		{
			if (player == nullptr) continue;
			player->Execute(distributor);
		}
	}

	bool LogicUnit::IsEmpty() const
	{
		return m_players.empty();
	}

	bool LogicUnit::IsMax() const
	{
		return m_maxLogicUnitPlayerCount <= m_players.size();
	}

	Int64Key LogicUnit::FindId() const
	{
		return m_id;
	}
}
