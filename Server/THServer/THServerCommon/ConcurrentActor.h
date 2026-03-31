#pragma once
#include "SharedQueue.h"
#include "PacketWrapper.h"

namespace th
{
	class ConcurrentSystem;
	class ConcurrentActorRef;
	class DBSession;
	class ConcurrentActor
	{
	protected:
		const int32_t MessageProcCount = 7;

	protected:
		ActorID_t m_id;
		util::SharedQueue<PTR<PacketWrapper>> m_messageBox;
		PTR<ConcurrentActorRef> m_self;
		std::atomic<bool> m_running;
		int32_t m_msgLimitMs;

	public:
		ConcurrentActor(int32_t limitMs = 300);
		virtual ~ConcurrentActor() = default;

	public:
		void SetSystem(const PTR<ConcurrentSystem>& system, const ActorID_t& actorID);
		virtual void Execute();

		ActorID_t GetActorID() const { return m_id; }
		PTR<ConcurrentActorRef> Self();
		bool IsEmpty();
		bool CompareExchangeRunningFlag(bool expected, bool change);
		void SetMsgLimitMs(const int32_t ms);

	protected:
		virtual void Update() {};
		virtual void Prepare() {};
		virtual void Arrange() {};
		virtual void Run(const PTR<PacketWrapper>& msg) = 0;
		virtual void RegisterSession(const PTR<DBSession>& session) {};

	private:
		//concurrentSystem만 접근. 우체통 역할인 ActorRef가 아닌 직접 Actor에 접근해서 처리하는걸 막기위해.
		friend ConcurrentSystem;
		void Send(const PTR<PacketWrapper>& msg)
		{
			m_messageBox.Push(msg);
		}
	};
}