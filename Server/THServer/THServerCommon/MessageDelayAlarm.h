#pragma once

namespace th
{
	class MessageDelayAlarm
	{
	protected:
		int64_t m_start;
		int64_t m_end;
		int32_t m_messageId;
		int32_t m_limitMs;

		AccountUID_t m_accountUID;
		HostID_t m_hostID;

	public:
		MessageDelayAlarm(const AccountUID_t& accountUID, const HostID_t& hostID, const int32_t& messageID, const int32_t& limitMs);
		virtual ~MessageDelayAlarm();

		void Alarm() const;
	};
}