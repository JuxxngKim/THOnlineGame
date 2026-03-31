#include "GamePch.h"
#include "LoginPendingList.h"
#include "SessionPlayer.h"

namespace th
{
	LoginPendingList::LoginPendingList()
	{
	}

	LoginPendingList::~LoginPendingList()
	{
		m_waiters.clear();
	}

	bool LoginPendingList::Add(const PTR<SessionPlayer>& player, const PTR<LogicUnit>& unit)
	{
		if (player == nullptr || unit == nullptr) return false;
		if (IsExist(player->FindHostID())) return false;

		Record record;
		record.Player = player;
		record.OwnerUnit = unit;

		m_waiters[player->FindHostID()] = record;
		return true;
	}

	void LoginPendingList::Remove(HostType hostID)
	{
		const auto it = m_waiters.find(hostID);
		if (it != m_waiters.end())
		{
			if (it->second.Player != nullptr) it->second.Player->Clear();
			m_waiters.erase(hostID);
		}
	}

	bool LoginPendingList::IsExist(HostType hostID) const
	{
		return m_waiters.contains(hostID);
	}

	PTR<LogicUnit> LoginPendingList::FindOwnerUnit(HostType hostID) const
	{
		const auto it = m_waiters.find(hostID);
		if (it == m_waiters.end()) return nullptr;

		return it->second.OwnerUnit;
	}

	int32_t LoginPendingList::FindCount() const
	{
		return static_cast<int32_t>(m_waiters.size());
	}
}