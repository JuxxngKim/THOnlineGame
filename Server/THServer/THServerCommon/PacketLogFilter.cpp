#include "CommonPch.h"
#include "PacketLogFilter.h"
#include "Configuration.h"
#include "StrUtil.h"

namespace util
{
	void PacketLogFilter::Write(const HostID_t& hostID, const th::AccountUID_t& auid, const std::string& body
		, const th::EMessageID& msgId, const google::protobuf::Message& msg)
	{
		const auto level = static_cast<th::ELogLevel>(th::Configuration::GetInstance().Get("Log", "Level").ToInt());
		if (th::ELogLevel::Info < level) return;

		if (body.empty()) return;
		if (PacketFilter(msgId)) return;

		const auto& msgIDName = th::EMessageID_Name(msgId);
		if (msgIDName.find("NAK") != std::string::npos)
		{
			if (msgId == th::AC_COMMON_NAK)
			{
				auto commonNak = NEW(th::ACCommonNak);
				commonNak->CopyFrom(msg);
				//if (commonNak->error() == th::E_GAME_SESSION_END)
				//{
				//	TH_LOG_INFO(hostID, auid, "[%] : [message:%, detail:%]", body, msgIDName, th::StrUtil::ToJson(msg));
				//	return;
				//}
			}

			TH_LOG_ERROR(hostID, auid, "[%] : [message:%, detail:%]", body, msgIDName, th::StrUtil::ToJson(msg));
		}
		else
		{
			TH_LOG_INFO(hostID, auid, "[%] : [message:%, detail:%]", body, msgIDName, th::StrUtil::ToJson(msg));
		}
	}

	bool PacketLogFilter::PacketFilter(const th::EMessageID& msgId)
	{
		switch (msgId)
		{
		case th::NET_ALIVE_REQ:
		case th::NET_ALIVE_ACK:
		case th::NET_CONNECT:
		case th::NET_DISCONNECT:
		{
			return true;
		}		
		default: { return false; }
		}
	}

	bool PacketLogFilter::LoginDBPacket(const th::EMessageID& msgId)
	{
		switch (msgId)
		{
		case th::AD_PLAYER_INFO_REQ:
		case th::DA_PLAYER_INFO_ACK:
		case th::AD_PLAYER_HERO_REQ:
		case th::DA_PLAYER_HERO_ACK:
		case th::AD_PLAYER_EQUIP_ITEM_REQ:
		case th::DA_PLAYER_EQUIP_ITEM_ACK:
		{
			return true;
		}
		default: { return false; }
		}
	}

	bool PacketLogFilter::LoginClientPacket(const th::EMessageID& msgId)
	{
		switch (msgId)
		{
		case th::AC_GET_PLAYER_INFO_NOTI:
		case th::AC_GET_PLAYER_HERO_NOTI:
		case th::AC_GET_PLAYER_FORMATION_NOTI:
		case th::AC_GET_PLAYER_DECK_NOTI:
		case th::AC_GET_PLAYER_ETC_NOTI:
		case th::AC_GET_PLAYER_TICKET_NOTI:
		case th::AC_GET_PLAYER_STAGE_NOTI:
		case th::AC_GET_PLAYER_USER_OPTION_NOTI:
		case th::AC_GET_PLAYER_DUNGEON_NOTI:
		case th::AC_GET_PLAYER_HERO_GROWTH_STORAGE_NOTI:
		case th::AC_GET_PLAYER_HERO_NON_GROWTH_STORAGE_NOTI:
		{
			return true;
		}
		default: { return false; }
		}
	}

	bool PacketLogFilter::BattlePacket(const th::EMessageID& msgId)
	{
		return false;
	}

	bool PacketLogFilter::LogicCyclePacket(const th::EMessageID& msgId)
	{
		return false;
	}
}
