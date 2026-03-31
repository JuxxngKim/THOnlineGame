#pragma once
#include "PacketCollector.h"

namespace th
{
	struct PacketWrapper;
	class LogicEventor
	{
	protected:
		using MessageID = int32_t;
		using MessageFunc = std::function<void(const HostID_t&, const AccountUID_t&, const PTR<google::protobuf::Message>&, const uint8_t& flag)>;

	protected:
		std::unordered_map<MessageID, MessageFunc> m_handlers;

		PTR<PacketCollector<PTR<PacketWrapper>>> m_collector;
		std::deque<PTR<PacketWrapper>> m_prepareEvents;
		std::deque<PTR<PacketWrapper>> m_arrangeEvents;

		std::set<int32_t> m_prepareMsgIds;
		std::set<int32_t> m_arrangeMsgIds;

		std::atomic<bool> m_needShutdown;

	private:
		std::atomic<int64_t> m_shutdownCheckSyncTime;

	public:
		LogicEventor() = delete;
		LogicEventor(const PTR<PacketCollector<PTR<PacketWrapper>>>& collector);
		virtual ~LogicEventor() = default;

		void Collect(const std::deque<PTR<PacketWrapper>>& packets);
		void Prepare();
		void Arrange();

		virtual void Event(const int64_t& delta) = 0;
		bool CheckQuit() const;
		bool CheckCrash() const;

	protected:
		template <typename DispatcherType, typename MessageType>
		void RegisterHandler(void (DispatcherType::* handler)(const HostID_t&, const AccountUID_t&, const PTR<MessageType>&, const uint8_t& flag), const uint8_t& flag)
		{
			DispatcherType* derived = static_cast<DispatcherType*>(this);
			auto messageID = MessageType().messageid();

			m_handlers.insert({
				messageID,
				[derived, handler](const HostID_t& hostID, const AccountUID_t& accountID, const PTR<google::protobuf::Message>& message, const uint8_t& flag)
				{ return (derived->*handler)(hostID, accountID, std::dynamic_pointer_cast<MessageType>(message), flag); }
				});

			m_collector->RegisterID(messageID);
			DistributeEventMsgId(flag, messageID);
		}

		void DistributeEventMsgId(const uint8_t& flag, const int32_t& msgId);
		bool IsPrepareEvent(const uint8_t& flag) const;
		bool IsArrangeEvent(const uint8_t& flag) const;
		bool IsPreparePacket(const int32_t& msgId) const;
		bool IsArrangePacket(const int32_t& msgId) const;

		bool Dispatch(const HostID_t& hostID, const AccountUID_t& accountID, const int32_t& messageID, const PTR<google::protobuf::Message>& message, const uint8_t& flag);
		void SendTo(const HostID_t& hostID, const int32_t& messageID, const PTR<google::protobuf::Message>& message);
		void SendTo(std::unordered_set<HostID_t>& hostIDs, const int32_t& messageID, const PTR<google::protobuf::Message>& message);

		void SetShutdownCheckSyncTime(const MDateTime& updateSyncTime, const int64_t& checkTimeMs);
	};
}