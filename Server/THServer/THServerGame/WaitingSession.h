#pragma once

namespace th
{
	class WaitingSession
	{
	private:
		std::unordered_map<HostID_t, AccountUID_t> m_watingSession;
		std::set<AccountUID_t> m_watingAccountIds;
	public:
		WaitingSession();
		virtual ~WaitingSession();

	public:
		bool RegisterWaiting(const HostType hostID, const AccountType accountID);
		bool IsWaiting(const HostType hostID, const AccountType accountID) const;
		void Unregister(const HostType hostID);

	private:
		bool IsAlreadyWaiting(const AccountType accountID) const;
	};
}