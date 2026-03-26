#include "CommonPch.h"
#include "PacketDistributor.h"

namespace th
{
	PacketDistributor::PacketDistributor()
	{
		//CommonMetric::GetInstance()->IncrementObject("packet_distributor");
	}

	PacketDistributor::~PacketDistributor()
	{
		//CommonMetric::GetInstance()->DecrementObject("packet_distributor");
	}

	void PacketDistributor::Generator(HostKeyMap_t&& hostMsgBox, AccountKeyMap_t&& accountMsgBox)
	{
		m_resultByHost = std::move(hostMsgBox);
		m_resultByAccount = std::move(accountMsgBox);
	}

	std::deque<PTR<PacketWrapper>>& PacketDistributor::Pick(const HostID_t& hostID)
	{
		const auto& it = m_resultByHost.find(hostID);
		if (it == m_resultByHost.end()) return m_empty;

		return it->second;
	}

	std::deque<PTR<PacketWrapper>>& PacketDistributor::Pick(const AccountUID_t accountUID)
	{
		const auto& it = m_resultByAccount.find(accountUID);
		if (it == m_resultByAccount.end()) return m_empty;

		return it->second;
	}

	bool PacketDistributor::IsEmpty() const
	{
		return m_resultByHost.empty() && m_resultByAccount.empty();
	}

	void PacketDistributor::Clear()
	{
		m_resultByHost.clear();
		m_resultByAccount.clear();
		m_empty.clear();
	}
}
