#include "CommonPch.h"
#include "ProtoMemberAdapter.h"
#include "THDateTime.h"

namespace th
{
	void ProtoMemberAdapter::ConvertReDateTimeToMDateTime(const THDateTime& in, MDateTime* out)
	{
		if (!in.IsValid())
		{
			TH_LOG_ERROR(0, 0, "invalid ReDateTime : [processingMsgID:%, year:%, month:%, day:%, dayofweek:%, hour:%, min:%, sec:%, ms:%]"
				, TL_ProcessingMsgID, in.Year, in.Month, in.Day, in.DayOfWeek, in.Hour, in.Minute, in.Second, in.Milliseconds);
		}

		out->set_year(in.Year);
		out->set_month(in.Month);
		out->set_day(in.Day);
		out->set_hour(in.Hour);
		out->set_minute(in.Minute);
		out->set_second(in.Second);
		out->set_millisecond(in.Milliseconds);
		out->set_dayofweek(in.DayOfWeek);
	}

	void ProtoMemberAdapter::ConvertMDateTimeToReDateTime(const MDateTime& in, THDateTime& out)
	{
		out.Year = in.year();
		out.Month = in.month();
		out.Day = in.day();
		out.Hour = in.hour();
		out.Minute = in.minute();
		out.Second = in.second();
		out.Milliseconds = in.millisecond();
		out.DayOfWeek = in.dayofweek();

		if (!out.IsValid())
		{
			TH_LOG_ERROR(0, 0, "invalid ReDateTime : [processingMsgID:%, year:%, month:%, day:%, dayofweek:%, hour:%, min:%, sec:%, ms:%]"
				, TL_ProcessingMsgID, out.Year, out.Month, out.Day, out.DayOfWeek, out.Hour, out.Minute, out.Second, out.Milliseconds);
		}
	}

	bool ProtoMemberAdapter::IsClientRequestPacket(const int32_t msgId)
	{
		return CA_CLIENT_ADVENTURE_BEGIN < msgId && msgId < AD_LOGIN_REQ;
	}

	bool ProtoMemberAdapter::IsDBPacket(const int32_t msgId)
	{
		return AD_ADVENTURE_DBSERVICE_BEGIN < msgId && msgId < AD_ADVENTURE_DBSERVICE_END;
	}

	//bool ProtoMemberAdapter::IsInternalPacket(const int32_t msgId)
	//{
	//	return INTERNAL_BEGIN < msgId && msgId < INTERNAL_END;
	//}
}