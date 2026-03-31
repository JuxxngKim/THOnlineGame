#pragma once
#include "NetworkManager.h"
#include "BlockDuplicatePacket.h"
#include "ConcurrentActorRef.h"

namespace th
{
	class PlayerPacketSender
	{
	public:
		struct SendToVariable
		{
			HostID_t HostID{ 0 };
			AccountUID_t AccountUID{ 0 };
			const PTR<Mailbox_t>& DbChannel;
			const PTR<BlockDuplicatePacket>& Checker;
			std::string PID;
			bool Authenticated{ false };
			ELanguageCode LanguageID{ ELanguageCode::None };

			SendToVariable(const HostID_t& hostID, const AccountUID_t& accountUID, const PTR<Mailbox_t>& dbChannel,
				const PTR<BlockDuplicatePacket>& checker, const std::string& pid, const bool& authenticated, const ELanguageCode& languageID)
				: HostID{ hostID }, AccountUID{ accountUID }, DbChannel{ dbChannel }, Checker{ checker }, PID{ pid },
				Authenticated{ authenticated }, LanguageID{ languageID } {
			}
		};
	public:
		static void SendToHost(const HostID_t& hostID, int32_t messageID, const PTR<google::protobuf::Message>& message)
		{
			network::NetworkManager::GetInstance().Send(hostID, messageID, message);
		}

		template<typename T, typename Tr, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value&& std::is_base_of<google::protobuf::Message, Tr>::value >>
		static void SendToDB(const SendToVariable& var, const PTR<T>& msg, const PTR<Tr>& requestMsg, bool isInfinite = false)
		{
			if (var.DbChannel == nullptr || var.Checker == nullptr) return;
			auto messageID = msg->messageid();
			auto blockKey = RegisterBlockDuplicatePacket(var.Checker, requestMsg->messageid(), isInfinite);
			var.DbChannel->Send(var.HostID, var.AccountUID, messageID, msg, blockKey);
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value>>
		static void SendToDB(const SendToVariable& var, const PTR<T>& msg, const EMessageID& reqMsgID, bool isInfinite = false)
		{
			if (var.DbChannel == nullptr || var.Checker == nullptr) return;
			auto messageID = msg->messageid();
			auto blockKey = RegisterBlockDuplicatePacket(var.Checker, reqMsgID, isInfinite);
			var.DbChannel->Send(var.HostID, var.AccountUID, messageID, msg, blockKey);
		}

	private:
		static int32_t RegisterBlockDuplicatePacket(const PTR<BlockDuplicatePacket>& checker, const EMessageID& clientMsgID, const bool& isInfinite)
		{
			auto blockKey = 0;
			if (clientMsgID != NULL_MESSAGE)
			{
				blockKey = checker->GenerateKey(clientMsgID);
				checker->PacketProcess(blockKey, clientMsgID, isInfinite);
			}
			return blockKey;
		}
	};
}
