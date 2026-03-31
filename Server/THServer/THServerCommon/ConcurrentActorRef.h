#pragma once

namespace th
{
	struct PacketWrapper;
	class ConcurrentSystem;
	class ConcurrentActorRef : public std::enable_shared_from_this<ConcurrentActorRef>
	{
	private:
		const ActorID_t m_actorID;
		PTR<ConcurrentSystem> m_system;

	public:
		ConcurrentActorRef() = delete;
		ConcurrentActorRef(const ActorID_t actorID, const PTR<ConcurrentSystem>& system);
		virtual ~ConcurrentActorRef() = default;

		bool Send(const PTR<PacketWrapper> msg);
		bool Send(const HostID_t& hostID, const AccountUID_t accountID, int32_t messageID, const PTR<google::protobuf::Message>& msg, int32_t blockKey = 0);

	public:
		ActorID_t GetActorID() const { return m_actorID; }
	};
}