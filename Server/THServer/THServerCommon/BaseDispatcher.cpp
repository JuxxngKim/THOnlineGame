#include "CommonPch.h"
#include "BaseDispatcher.h"

namespace th
{
	bool BaseDispatcher::Dispatch(const HostID_t& hostID, const MessageID& messageID, const PTR<google::protobuf::Message>& message)
	{
		const auto it = m_handlers.find(messageID);
		if (it == m_handlers.end())
		{
			if (m_errLog) TH_LOG_ERROR(hostID, 0, "not defined message handler : [id:%]", messageID);
			return false;
		}

		(it->second)(hostID, message);
		return true;
	}
}