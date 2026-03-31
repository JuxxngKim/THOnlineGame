#pragma once
#include "ConcurrentSystem.h"
#include "Singleton.h"

namespace th
{
	class ConcurrentSystem; struct ConcurrentMessage; class ConcurrentActor;
	enum class EConcurrentSystemType;
	class ConcurrentCenter : public Singleton<ConcurrentCenter>
	{
	private:
		PTR<ConcurrentSystem> m_dbSystem;
		PTR<Mailbox_t> m_systemMailbox;
		std::vector<PTR<Mailbox_t>> m_noAccountMailbox;

	public:
		ConcurrentCenter();
		virtual ~ConcurrentCenter() = default;

	public:
		bool Init();
		bool Start();
		void Exit();

	public:
		void RegisterDBActor(const PTR<ConcurrentActor>& actor);
		void DeregisterDBActor(const ActorID_t id);

		void SendToLogicService(const PTR<PacketWrapper>& msg);
		void SendToSystemDB(const HostID_t& hostID, const PTR<PacketWrapper>& msg);
	};
}
