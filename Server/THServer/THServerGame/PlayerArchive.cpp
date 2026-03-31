#include "GamePch.h"
#include "PlayerArchive.h"
#include "LogicUnit.h"
#include "Player.h"
// TODO
#include "ConcurrentCenter.h"
//#include "DBService.h"

namespace th
{
	PlayerArchive::PlayerArchive()
	{
	}

	PlayerArchive::~PlayerArchive()
	{
		m_players.clear();
		m_playerPIDs.clear();
		m_DBPlayerCount.clear();
		m_hostIDs.clear();
		for (auto& [_, set] : m_channelHostIDMap)
		{
			set.clear();
		}
		m_channelHostIDMap.clear();
	}

	bool PlayerArchive::Add(const PTR<Player>& player, const PTR<LogicUnit>& unit)
	{
		if (player == nullptr || unit == nullptr) return false;
		if (m_players.contains(player->FindAccountUID()))
		{
			TH_LOG_ERROR(player->FindHostID(), player->FindAccountUID(), "already exist player");
			return false;
		}

		Record record;
		record.PlayerPtr = player;
		record.OwnerUnit = unit;
		m_players[player->FindAccountUID()] = record;
		m_playerPIDs[player->FindPID()] = player->FindAccountUID();

		// TODO
		// const auto dbActor = NEW(DBService, player->FindAccountUID(), player->FindGameDBID(), [&](const PTR<PacketWrapper>& msg) { ConcurrentCenter::GetInstance()->SendToLogicService(msg); });
		// ConcurrentCenter::GetInstance()->RegisterDBActor(dbActor);
		// player->SetDBChannel(dbActor->Self());

		if (!m_DBPlayerCount.contains(player->FindGameDBID()))
		{
			m_DBPlayerCount.insert({ player->FindGameDBID(), 1 });
		}
		else
		{
			m_DBPlayerCount[player->FindGameDBID()] += 1;
		}

		TH_LOG_INFO(player->FindHostID(), player->FindAccountUID(), "add player");

		return true;
	}

	void PlayerArchive::Remove(const HostType& hostID, const AccountType& accountID)
	{
		const auto& it = m_players.find(accountID);
		if (it == m_players.end()) return;

		const auto player = it->second.PlayerPtr;
		if (player != nullptr)
		{
			m_playerPIDs.erase(player->FindPID());

			DeregisterBroadCastingHostID(0, hostID);

			m_DBPlayerCount[player->FindGameDBID()] = max(m_DBPlayerCount[player->FindGameDBID()] - 1, 0);
			player->Clear();
		}

		m_players.erase(it);

		ConcurrentCenter::GetInstance().DeregisterDBActor(accountID);

		TH_LOG_INFO(hostID, accountID, "remove player");
	}

	bool PlayerArchive::IsExist(const AccountType& accountID) const
	{
		const auto& it = m_players.find(accountID);
		if (it == m_players.end()) return false;

		return true;
	}

	bool PlayerArchive::IsSame(const HostType& hostID, const AccountType& accountID) const
	{
		const auto& it = m_players.find(accountID);
		if (it == m_players.end()) return false;

		const auto& player = it->second.PlayerPtr;
		if (player == nullptr) return false;

		if (player->FindHostID() != hostID)
		{
			TH_LOG_ERROR(hostID, accountID, "not matched hostID : [curHostId:%]", player->FindHostID());
			return false;
		}

		return true;
	}

	void PlayerArchive::AddChannelHostID(const ChannelID_t& channelID, const HostID_t& hostID)
	{
		m_channelHostIDMap[channelID].insert(hostID);
	}

	PTR<Player> PlayerArchive::FindPlayer(const AccountType accountID)
	{
		const auto it = m_players.find(accountID);
		if (it == m_players.end()) return nullptr;

		return it->second.PlayerPtr;
	}

	PTR<Player> PlayerArchive::FindPlayer(const PID_t& pid)
	{
		const auto& pidIt = m_playerPIDs.find(pid);
		if (pidIt == m_playerPIDs.cend()) return nullptr;

		const auto& playerIt = m_players.find(pidIt->second);
		if (playerIt == m_players.cend()) return nullptr;

		return playerIt->second.PlayerPtr;
	}

	PTR<LogicUnit> PlayerArchive::FindOwnerUnit(const AccountType accountID)
	{
		const auto it = m_players.find(accountID);
		if (it == m_players.end()) return nullptr;

		return it->second.OwnerUnit;
	}

	int32_t PlayerArchive::FindTotalCount() const
	{
		return static_cast<int32_t>(m_players.size());
	}

	std::unordered_map<PlayerArchive::GameDBID_t, int32_t> PlayerArchive::FindDBPlayerCount() const
	{
		return m_DBPlayerCount;
	}

	std::vector<PTR<Player>> PlayerArchive::FindAllPlayer()
	{
		std::vector<PTR<Player>> outputPlayers;
		outputPlayers.reserve(m_players.size());

		for (const auto& [accountUID, record] : m_players) {
			if (record.PlayerPtr != nullptr) {
				outputPlayers.push_back(record.PlayerPtr);
			}
		}

		return outputPlayers;
	}

	std::unordered_set<HostID_t> PlayerArchive::FindAllHostID()
	{
		return m_hostIDs;
	}

	std::unordered_map<PlayerArchive::ChannelID_t, std::unordered_set<HostID_t>> PlayerArchive::FindChannelHostIDMap()
	{
		return m_channelHostIDMap;
	}

	void PlayerArchive::ChangeBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& prevHostID, const HostID_t& hostID)
	{
		const auto& hostIt = m_hostIDs.find(prevHostID);
		if (hostIt != m_hostIDs.end()) m_hostIDs.erase(prevHostID);

		m_hostIDs.emplace(hostID);

		if (0 < channelID)
		{
			const auto& channelIt = m_channelHostIDMap.find(channelID);
			if (channelIt != m_channelHostIDMap.end())
			{
				const auto& channelHostIt = channelIt->second.find(prevHostID);
				if (channelHostIt != channelIt->second.end()) channelIt->second.erase(prevHostID);
			}
			channelIt->second.emplace(hostID);
		}
	}

	void PlayerArchive::RegisterBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& hostID)
	{
		if (hostID <= 0) return;

		m_hostIDs.insert(hostID);
		if (0 < channelID) m_channelHostIDMap[channelID].insert(hostID);
	}

	void PlayerArchive::DeregisterBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& hostID)
	{
		if (hostID <= 0) return;

		const auto& it = m_hostIDs.find(hostID);
		if (it != m_hostIDs.end()) m_hostIDs.erase(it);

		if (0 < channelID)
		{
			const auto& channelMapIt = m_channelHostIDMap.find(channelID);
			if (channelMapIt == m_channelHostIDMap.end()) return;

			const auto& channelHostIt = channelMapIt->second.find(hostID);
			if (channelHostIt == channelMapIt->second.end()) return;

			channelMapIt->second.erase(channelHostIt);
		}
	}
}
