#include "GamePch.h"
#include "WaitingSession.h"

namespace th
{
	WaitingSession::WaitingSession()
	{
		//CommonMetric::GetInstance()->IncrementObject("waiting_session");
	}

	WaitingSession::~WaitingSession()
	{
		m_watingSession.clear();
		m_watingAccountIds.clear();
		//CommonMetric::GetInstance()->DecrementObject("waiting_session");
	}

	bool WaitingSession::RegisterWaiting(const HostType hostID, const AccountType accountID)
	{
		if (IsAlreadyWaiting(accountID)) return false;

		m_watingSession.insert({ hostID, accountID });
		m_watingAccountIds.insert(accountID);

		return true;
	}

	bool WaitingSession::IsWaiting(const HostType hostID, const AccountType accountID) const
	{
		const auto& it = m_watingSession.find(hostID);
		if (it == m_watingSession.end()) return false;

		return it->second == accountID;
	}

	bool WaitingSession::IsAlreadyWaiting(const AccountType accountID) const
	{
		const auto& it = m_watingAccountIds.find(accountID);
		if (it == m_watingAccountIds.end()) return false;

		return true;
	}

	void WaitingSession::Unregister(const HostType hostID)
	{
		if (hostID == 0) return;

		const auto& it = m_watingSession.find(hostID);
		if (it == m_watingSession.end()) return;

		m_watingAccountIds.erase(it->second);
		m_watingSession.erase(it);
	}
}