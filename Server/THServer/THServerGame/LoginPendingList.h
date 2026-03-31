#pragma once

namespace th
{
	class SessionPlayer;
	class LogicUnit;
	class LoginPendingList
	{
	private:
		using PID = std::string;
		struct Record
		{
			PTR<SessionPlayer> Player{ nullptr };
			PTR<LogicUnit> OwnerUnit{ nullptr };
			AccountUID_t WaitingSession{ 0 };
		};

	private:
		std::unordered_map<HostID_t, Record> m_waiters;

	public:
		LoginPendingList();
		virtual ~LoginPendingList();

		bool Add(const PTR<SessionPlayer>& player, const PTR<LogicUnit>& unit);
		void Remove(const HostType hostID);
		bool IsExist(HostType hostID) const;

		PTR<LogicUnit> FindOwnerUnit(HostType hostID) const;
		int32_t FindCount() const;
	};
}