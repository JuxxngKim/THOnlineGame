#pragma once

namespace th
{
	class Player;
	class LogicUnit;
	class PlayerArchive
	{
		using GameDBID_t = int32_t;
		using ChannelID_t = int32_t;
		using PID_t = std::string;

	private:
		struct Record
		{
			PTR<Player> PlayerPtr{ nullptr };
			PTR<LogicUnit> OwnerUnit{ nullptr };
		};

	private:
		std::unordered_map<AccountUID_t, Record> m_players;
		std::unordered_map<PID_t, AccountUID_t> m_playerPIDs;
		std::unordered_map<GameDBID_t, int32_t> m_DBPlayerCount;
		std::unordered_set<HostID_t> m_hostIDs;
		std::unordered_map<ChannelID_t, std::unordered_set<HostID_t>> m_channelHostIDMap;

	public:
		PlayerArchive();
		virtual ~PlayerArchive();

	public:
		bool Add(const PTR<Player>& player, const PTR<LogicUnit>& unit);
		void Remove(const HostType& hostID, const AccountType& accountID);

		bool IsExist(const AccountType& accountID) const;
		bool IsSame(const HostType& hostID, const AccountType& accountID) const;

		void AddChannelHostID(const ChannelID_t& channelID, const HostID_t& hostID);

	public:
		PTR<Player> FindPlayer(const AccountType accountID);
		PTR<Player> FindPlayer(const PID_t& pid);
		PTR<LogicUnit> FindOwnerUnit(const AccountType accountID);
		int32_t FindTotalCount() const;
		std::unordered_map<GameDBID_t, int32_t> FindDBPlayerCount() const;
		std::vector<PTR<Player>> FindAllPlayer();
		std::unordered_set<HostID_t> FindAllHostID();
		std::unordered_map<ChannelID_t, std::unordered_set<HostID_t>> FindChannelHostIDMap();
		void ChangeBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& prevHostID, const HostID_t& hostID);
		void RegisterBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& hostID);
		void DeregisterBroadCastingHostID(const ChannelID_t& channelID, const HostID_t& hostID);
	};
}