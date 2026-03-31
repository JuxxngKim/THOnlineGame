#pragma once

namespace th
{
	class BaseDispatcher
	{
	private:
		using MessageID = int32_t;

	protected:
		std::unordered_map<MessageID, std::function<void(const HostID_t&, const PTR<google::protobuf::Message>&)>> m_handlers;
		bool m_errLog;

	public:
		BaseDispatcher() : m_errLog{ true } {}
		virtual ~BaseDispatcher()
		{
			m_handlers.clear();
		};

		virtual void Clear() = 0;

	protected:
		template <typename DispatcherType, typename MessageType>
		void RegisterHandler(DispatcherType* instance, void (DispatcherType::* handler)(const HostID_t&, const PTR<MessageType>&))
		{
			auto messageID = MessageType().messageid();

			m_handlers.insert({
				messageID,
				[instance, handler](const HostID_t& hostID, const PTR<google::protobuf::Message>& message)
				{ return (instance->*handler)(hostID, std::static_pointer_cast<MessageType>(message)); }
				});
		}

	protected:
		bool Dispatch(const HostID_t& hostID, const MessageID& messageID, const PTR<google::protobuf::Message>& message);
	};
}