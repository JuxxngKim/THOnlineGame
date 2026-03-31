#pragma once

namespace th
{
	class Character;
	class MDateTime;
	class ProtoMemberAdapter
	{
	public:
		static void ConvertReDateTimeToMDateTime(const THDateTime& in, MDateTime* out);
		static void ConvertMDateTimeToReDateTime(const MDateTime& in, THDateTime& out);
		static bool IsClientRequestPacket(const int32_t msgId);
		static bool IsDBPacket(const int32_t msgId);
		//static bool IsInternalPacket(const int32_t msgId);
	};
}
