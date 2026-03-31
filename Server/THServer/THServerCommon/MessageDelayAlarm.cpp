#include "CommonPch.h"
#include "MessageDelayAlarm.h"
#include "TimeUtil.h"

namespace th
{
	MessageDelayAlarm::MessageDelayAlarm(const AccountUID_t& accountUID, const HostID_t& hostID, const int32_t& messageID, const int32_t& limitMs)
		: m_start{ 0 }, m_end{ 0 }, m_messageId{ messageID }, m_limitMs{ limitMs }, m_accountUID{ accountUID }, m_hostID{ hostID }
	{
		m_start = util::TimeUtil::GetInstance().FindTickKSTMs();
	}

	MessageDelayAlarm::~MessageDelayAlarm()
	{
		m_end = util::TimeUtil::GetInstance().FindTickKSTMs();

		Alarm();
	}

	void MessageDelayAlarm::Alarm() const
	{
		const auto& executeTime = m_end - m_start;
		const auto isAlarm = m_limitMs <= executeTime;

		if (!isAlarm) return;
		TH_LOG_ERROR(m_hostID, m_accountUID, "message process time is over : [msgId:%, time:%]", m_messageId, executeTime);
	}
}
