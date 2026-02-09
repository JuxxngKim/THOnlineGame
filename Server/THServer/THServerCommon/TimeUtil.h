#pragma once
#include <iomanip>
#include "THDateTime.h"

namespace util
{
	class TimeUtil : public Singleton<TimeUtil>
	{
	private:
		std::atomic<int64_t> m_diffSec;

	public:
		TimeUtil();
		virtual ~TimeUtil() = default;

		//////////////////
		// 기본 적으로 서버에서는 KST를 사용하도록한다.
		/////////////////

		void ChangeTime(const uint16_t& year, const uint16_t& month, const uint16_t& day, const uint16_t& hour, const uint16_t& minute, const uint16_t& second);
		void ResetTimeGap();
		int64_t FindTimeGap();

		THDateTime FindSystemKSTDate() const;
		THDateTime FindKSTDate() const;
		int64_t FindTickKSTMs(const int64_t& spanMs = 0) const;
		int64_t FindKSTMs() const;
		int64_t FindKSTMs(const int64_t& spanMs) const;
		int64_t FindIntervalSeconds(const THDateTime& start, const THDateTime& end) const;
		int32_t FindCompareTimeMs(const int64_t& t1, const int64_t& t2) const;
		bool NeedToDailyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour);
		bool NeedToWeeklyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDayOfWeek);
		bool NeedToMonthlyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDay);
		THDateTime AddHourTHDateTimeToKSTDate(const THDateTime& time, const int32_t& addHour) const;
		THDateTime AddDayTHDateTimeToKSTDate(const THDateTime& time, const int32_t& addDay) const;
		THDateTime ConvertTHDateTimeFromString(const std::string& timeString) const;
		THDateTime ConvertKSTDateFromISO8601(const std::string& timeString) const;
		THDateTime ConvertKSTDateFromISO8601WithMS(const std::string& timeString) const;
		bool IsInPeriod(const THDateTime& time1, const THDateTime& time2, const THDateTime& pivotTime) const;
		THDateTime AddSecondsTHDateTimeToKSTDate(const THDateTime& time, const int64_t& addSeconds) const;
		THDateTime AddMinutesTHDateTimeToKSTDate(const THDateTime& time, const int64_t& addMinutes) const;
		THDateTime FindAfterSecondsTHDateTimeToKSTDate(const int64_t& afterSeconds) const;
		static std::string ToString(const THDateTime& time);
		static std::string SystemTimeToString(const _SYSTEMTIME& time);
		static std::string ToISO8601Format(const THDateTime& time);
		int64_t ConvertTHDateTimeToKSTTimeMs(const THDateTime& time) const;
		THDateTime ConvertKSTTimeMsToTHDateTime(const int64_t& timeMilli) const;
		int64_t FindDailyRenewalUTCTimeMS(const int32_t& resetHour) const;
		THDateTime FindTimeAfterRepeatedSeconds(const THDateTime& start, const int64_t& seconds) const;
		THDateTime FindDateTimeByDayAndHour(const THDateTime& curDateTime, const int32_t& dayOfWeek, const int32_t& hour) const;
		THDateTime FindNextDateTimeByDayAndHour(const THDateTime& curDateTime, const int32_t& dayOfWeek, const int32_t& hour) const;
		THDateTime FindNextDateTimeByHour(const THDateTime& curDateTime, const int32_t& hour) const;
		THDateTime MakeScheduleTime(const THDateTime& defaultDateTime, const int32_t& addDay, const THDateTime& settingTime) const;
		int64_t ConvertKSTMsToUTCMs(const int64_t& time) const;
		int64_t ConvertUTCMsToKSTMs(const int64_t& time) const;
		THDateTime FindLastDailyResetTime(const int32_t& resetHour) const;
		THDateTime FindLastWeeklyResetTime(const int32_t& resetDay, const int32_t& resetHour) const;
		THDateTime FindLastMonthlyResetTime(const int32_t& resetDay, const int32_t& resetHour) const;
		THDateTime FindNextDailyResetTime(const int64_t& curMs, const int32_t& resetHour) const;
		THDateTime FindNextWeeklyResetTime(const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDayOfWeek) const;
		THDateTime FindNextMonthlyResetTime(const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDay) const;
		THDateTime FindNextDailyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetHour) const;
		THDateTime FindNextWeeklyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDayOfWeek, const int32_t& resetHour) const;
		THDateTime FindNextMonthlyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDay, const int32_t& resetHour) const;
		THDateTime FindLastDailyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetHour) const;
		THDateTime FindLastWeeklyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDayOfWeek, const int32_t& resetHour) const;
		THDateTime FindMinDateTime() const;
		THDateTime FindMaxDateTime() const;
		int32_t FindDayAfterNextResetHour(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour) const;

	private:
		int64_t FindKSTTime_t() const;
		THDateTime ConvertTmToTHDateTime(const std::tm& tm, const int32_t& milliSec) const;

		//////////////////
		// UTC 함수들은 클라이언트에 시간 값을 보내줄 때에만 사용하도록 한다.
		// 이외의 시간 계산은 모두 KST로 하자
		/////////////////
	public:
		int64_t FindUTCMs() const;
		int64_t FindUTCMs(const int64_t& spanMs) const;
		int64_t FindCorrectIntervalSeconds(const THDateTime& start, const THDateTime& end) const;
		int64_t FindCorrectIntervalDays(const THDateTime& start, const THDateTime& end) const;
		int64_t FindCorrectIntervalWeeklys(const THDateTime& start, const THDateTime& end) const;
		int64_t ConvertTHDateTimeToUTCTimeMs(const THDateTime& time) const;
		THDateTime ConvertKSTDateTimeToUTCDateTime(const THDateTime& kstDateTime) const;

	private:
		int64_t FindUTCTime_t() const;
		int64_t FindEpochTime(const THDateTime& time) const;


	public:
		friend std::ostream& operator<<(std::ostream& os, TimeUtil* time)
		{
			if (time == nullptr) return os;

			auto curTime = time->FindKSTDate();
			os << curTime.Year << "-"
				<< std::setfill('0') << std::setw(2) << curTime.Month << "-"
				<< std::setfill('0') << std::setw(2) << curTime.Day << " "
				<< std::setfill('0') << std::setw(2) << curTime.Hour << ":"
				<< std::setfill('0') << std::setw(2) << curTime.Minute << ":"
				<< std::setfill('0') << std::setw(2) << curTime.Second;

			return os;
		}
	};
}