#pragma once
#include "BaseDispatcher.h"

namespace th
{
	struct PacketWrapper;
	class PacketDistributor
	{
	private:
		using HostKeyMap_t = std::unordered_map<HostID_t, std::deque<PTR<PacketWrapper>>>;
		using AccountKeyMap_t = std::unordered_map<AccountUID_t, std::deque<PTR<PacketWrapper>>>;

	private:
		HostKeyMap_t m_resultByHost;
		AccountKeyMap_t m_resultByAccount;

		std::deque <PTR<PacketWrapper>> m_empty;

	public:
		PacketDistributor();
		virtual ~PacketDistributor();

		void Generator(HostKeyMap_t&& hostMsgBox, AccountKeyMap_t&& accountMsgBox);
		std::deque<PTR<PacketWrapper>>& Pick(const HostID_t& hostID);
		std::deque<PTR<PacketWrapper>>& Pick(const AccountUID_t accountUID);
		bool IsEmpty() const;
		void Clear();
	};
}