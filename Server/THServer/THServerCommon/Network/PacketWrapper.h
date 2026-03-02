#pragma once
#include "DefineType.h"

namespace th
{
	struct PacketWrapper
	{
		HostID_t HostID{ 0 };
		AccountUID_t AccountUID{ 0 };
		int32_t MessageID{ 0 };
		PTR<google::protobuf::Message> Msg{ nullptr };
		int32_t BlockKey{ 0 };

		explicit PacketWrapper(const HostID_t& hostID, AccountUID_t accountID, int32_t messageID, PTR<google::protobuf::Message> msg, int32_t blockKey = 0)
			: HostID{ hostID }, AccountUID{ accountID }, MessageID{ messageID }, Msg{ msg }, BlockKey{ blockKey }
		{
			//CommonMetric::GetInstance()->IncrementObject("packet_wrapper");
		}

		virtual ~PacketWrapper()
		{
			//CommonMetric::GetInstance()->DecrementObject("packet_wrapper");
		}

		void Clear()
		{
			HostID = 0;
			AccountUID = 0;
			MessageID = 0;

			if (Msg != nullptr)
			{
				Msg->Clear();
				Msg = nullptr;
			}

			BlockKey = 0;
		}
	};
}