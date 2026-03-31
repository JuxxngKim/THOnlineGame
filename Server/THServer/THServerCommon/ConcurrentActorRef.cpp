#include "CommonPch.h"
#include "ConcurrentActorRef.h"
#include "ConcurrentSystem.h"
#include "PacketWrapper.h"

namespace th
{
	ConcurrentActorRef::ConcurrentActorRef(const ActorID_t actorID, const PTR<ConcurrentSystem>& system)
		:m_actorID{ actorID },
		m_system(system)
	{
	}

	bool ConcurrentActorRef::Send(const PTR<PacketWrapper> msg)
	{
		if (m_system == nullptr)
		{
			const std::string err = "system is null: " + std::to_string(m_actorID);
			throw std::runtime_error(err);
		}

		return m_system->RoutMessage(shared_from_this(), msg);
	}

	bool ConcurrentActorRef::Send(const HostID_t& hostID, const AccountUID_t accountID, int32_t messageID, const PTR<google::protobuf::Message>& msg, int32_t blockKey)
	{
		return Send(NEW(PacketWrapper, hostID, accountID, messageID, msg, blockKey));
	}
}