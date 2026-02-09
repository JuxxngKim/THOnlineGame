#include "CommonPCH.h"
#include "TimeUtil.h"

namespace util
{
	TimeUtil::TimeUtil() : m_diffSec{ 0 }
	{
	}

	void TimeUtil::ChangeTime(const uint16_t& year, const uint16_t& month, const uint16_t& day
		, const uint16_t& hour, const uint16_t& minute, const uint16_t& second)
	{
		const THDateTime changeTime{ year, month, 0, day, hour, minute, second, 0 };
		m_diffSec = FindCorrectIntervalSeconds(FindSystemKSTDate(), changeTime);
	}

	void TimeUtil::ResetTimeGap()
	{
		m_diffSec = 0;
	}

	int64_t TimeUtil::FindTimeGap()
	{
		return m_diffSec;
	}

	THDateTime TimeUtil::FindSystemKSTDate() const
	{
		const auto now = std::chrono::system_clock::now();
		const auto timt = std::chrono::system_clock::to_time_t(now) + g_KoreaTimeZoneS;

		std::tm tm{};

		gmtime_s(&tm, &timt); //thread safe

		const int32_t& milliSec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % g_SecToMs;
		THDateTime result = ConvertTmToTHDateTime(tm, milliSec);

		return result;
	}

	THDateTime TimeUtil::FindKSTDate() const
	{
		//NOTE:: windows server 2019에는 std::chrono::current_zone()가 적용 안되어 있음.
		// https://developercommunity.visualstudio.com/t/chrono-tzdb-list-creation-fails-on-windows-server/1465261
		/*auto now = std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() }.local_time();
		auto ld = std::chrono::floor<std::chrono::days>(now);

		std::chrono::year_month_day ymd{ ld };
		if (!ymd.ok())
		{
			RE_LOG_ERROR(0, 0, "invlid local date");
			return {};
		}

		std::chrono::hh_mm_ss hms(now - ld);
		std::chrono::year_month_weekday ymw{ ld };

		THDateTime result{};
		result.Year = static_cast<uint16_t>(static_cast<int32_t>(ymd.year()));
		result.Month = static_cast<uint16_t>(static_cast<uint32_t>(ymd.month()));
		result.DayOfWeek = static_cast<uint16_t>(ymw.weekday().c_encoding());
		result.Day = static_cast<uint16_t>(static_cast<uint32_t>(ymd.day()));
		result.Hour = static_cast<uint16_t>(hms.hours().count());
		result.Minute = static_cast<uint16_t>(hms.minutes().count());
		result.Second = static_cast<uint16_t>(hms.seconds().count());
		result.Milliseconds = static_cast<uint16_t>(hms.subseconds().count() / 10000);*/

		const auto now = std::chrono::system_clock::now();
		const auto timt = std::chrono::system_clock::to_time_t(now) + g_KoreaTimeZoneS + m_diffSec;

		std::tm tm{};

		gmtime_s(&tm, &timt); //thread safe

		const int32_t& milliSec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % g_SecToMs;
		THDateTime result = ConvertTmToTHDateTime(tm, milliSec);

		return result;
	}

	int64_t TimeUtil::FindTickKSTMs(const int64_t& spanMs) const
	{
		const auto now = std::chrono::system_clock::now();
		auto resultMS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
		resultMS += g_KoreaTimeZoneMS + spanMs;

		return resultMS;
	}

	int64_t TimeUtil::FindKSTMs() const
	{
		return FindUTCMs() + g_KoreaTimeZoneMS;
	}

	int64_t TimeUtil::FindKSTMs(const int64_t& spanMs) const
	{
		return FindKSTMs() + spanMs;
	}

	//int64_t TimeUtil::GetKSTMs()
	//{
	//	// https://developercommunity.visualstudio.com/t/chrono-tzdb-list-creation-fails-on-windows-server/1465261
	//	//auto now = std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() }.local_time();
	//	//return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

	//	return static_cast<int64_t>(FindUTCMs() + KoreaTimeZoneMS);
	//}

	int64_t TimeUtil::FindIntervalSeconds(const THDateTime& start, const THDateTime& end) const
	{
		const auto startEpochSeconds = FindEpochTime(start);
		const auto endEpochSeconds = FindEpochTime(end);

		const int64_t interval = endEpochSeconds - startEpochSeconds;
		return (interval < 0) ? 0 : interval;
	}

	int32_t TimeUtil::FindCompareTimeMs(const int64_t& t1, const int64_t& t2) const
	{
		if (t1 > t2) return 1;
		if (t1 < t2) return -1;
		return 0;
	}

	/// <summary>
	/// 일간 리셋 체크
	/// </summary>
	/// <param name="befMs">KST</param>
	/// <param name="curMs">KST</param>
	/// <param name="resetHour">KST</param>
	/// <returns></returns>
	bool TimeUtil::NeedToDailyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour)
	{
		const auto nextDailyResetTime = FindNextDailyResetTime(befMs, resetHour);

		return nextDailyResetTime <= ConvertKSTTimeMsToTHDateTime(curMs);
	}

	/// <summary>
	/// 주간 리셋 체크
	/// </summary>
	/// <param name="befMs">KST</param>
	/// <param name="curMs">KST</param>
	/// <param name="resetHour">KST</param>
	/// <param name="resetDayOfWeek">KST</param>
	/// <returns></returns>
	bool TimeUtil::NeedToWeeklyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDayOfWeek)
	{
		const auto nextWeeklyResetTime = FindNextWeeklyResetTime(befMs, resetHour, resetDayOfWeek);

		return nextWeeklyResetTime <= ConvertKSTTimeMsToTHDateTime(curMs);
	}

	/// <summary>
	/// 월간 리셋 체크
	/// </summary>
	/// <param name="befMs">KST</param>
	/// <param name="curMs">KST</param>
	/// <param name="resetHour">KST</param>
	/// <param name="resetDay">KST</param>
	/// <returns></returns>
	bool TimeUtil::NeedToMonthlyRenewal(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDay)
	{
		const auto nextDailyResetTime = FindNextMonthlyResetTime(befMs, resetHour, resetDay);

		return nextDailyResetTime <= ConvertKSTTimeMsToTHDateTime(curMs);
	}

	THDateTime TimeUtil::AddHourTHDateTimeToKSTDate(const THDateTime& time, const int32_t& addHour) const
	{
		return AddSecondsTHDateTimeToKSTDate(time, std::chrono::seconds(std::chrono::hours(addHour)).count());
	}

	THDateTime TimeUtil::AddDayTHDateTimeToKSTDate(const THDateTime& time, const int32_t& addDay) const
	{
		return AddSecondsTHDateTimeToKSTDate(time, std::chrono::seconds(std::chrono::hours(addDay * 24)).count());
	}

	THDateTime TimeUtil::ConvertTHDateTimeFromString(const std::string& timeString) const
	{
		std::tm tm{};
		std::istringstream stream(timeString);

		const std::regex reg("([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])");
		if (std::regex_match(timeString, reg))
		{
			const auto format = "%H:%M:%S";
			stream >> std::get_time(&tm, format);
		}
		else
		{
			const auto format = "%Y-%m-%d %H:%M:%S";
			stream >> std::get_time(&tm, format);
		}

		// 요일 세팅을위한 mktime, 반환값은 따로 사용하지않음
		mktime(&tm);

		THDateTime result{};
		result.Year = tm.tm_year + g_InitializeYearValue;
		result.Month = tm.tm_mon + g_InitializeMonthValue;
		result.DayOfWeek = tm.tm_wday;
		result.Day = tm.tm_mday;
		result.Hour = tm.tm_hour;
		result.Minute = tm.tm_min;
		result.Second = tm.tm_sec;

		return result;
	}

	THDateTime TimeUtil::ConvertKSTDateFromISO8601(const std::string& timeString) const
	{
		std::tm tm{};
		std::istringstream stream(timeString);
		THDateTime result = FindMinDateTime();

		const std::regex regISO8601("(2[0-9]{3})-([0-1][0-9])-([0-3][0-9])T([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])Z");
		if (std::regex_match(timeString, regISO8601))
		{
			const auto format = "%Y-%m-%dT%H:%M:%SZ";
			stream >> std::get_time(&tm, format);

			// 입력되는 tm이 utc 이기에 mktime을 쓰면 안된다.
			// 다른곳에서 사용하는 mktime의 경우 입력되는 tm이 kst임.
			int64_t utcMS = _mkgmtime(&tm) * g_SecToMs;
			int64_t kstMS = ConvertUTCMsToKSTMs(utcMS);
			result = ConvertKSTTimeMsToTHDateTime(kstMS);
		}

		return result;
	}

	THDateTime TimeUtil::ConvertKSTDateFromISO8601WithMS(const std::string& timeString) const
	{
		std::tm tm{};
		std::istringstream stream(timeString);
		THDateTime result = FindMinDateTime();

		const std::regex regISO8601("(2[0-9]{3})-([0-1][0-9])-([0-3][0-9])T([0-1][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])[.]([0-9]+)Z");
		if (std::regex_match(timeString, regISO8601))
		{
			const auto format = "%Y-%m-%dT%H:%M:%SZ";
			stream >> std::get_time(&tm, format);

			// 입력되는 tm이 utc 이기에 mktime을 쓰면 안된다.
			// 다른곳에서 사용하는 mktime의 경우 입력되는 tm이 kst임.
			int64_t utcMS = _mkgmtime(&tm) * g_SecToMs;
			int64_t kstMS = ConvertUTCMsToKSTMs(utcMS);
			result = ConvertKSTTimeMsToTHDateTime(kstMS);
		}

		return result;
	}

	/// <summary>
	/// pivotTime이 time1과 time2 사이에 있는지 확인
	/// </summary>
	/// <param name="time1"></param>
	/// <param name="time2"></param>
	/// <param name="pivotTime"></param>
	/// <returns></returns>
	bool TimeUtil::IsInPeriod(const THDateTime& time1, const THDateTime& time2, const THDateTime& pivotTime) const
	{
		if (time1 < time2)
		{
			return time1 <= pivotTime && pivotTime < time2;
		}
		else
		{
			return time2 <= pivotTime && pivotTime < time1;
		}
	}

	THDateTime TimeUtil::AddSecondsTHDateTimeToKSTDate(const THDateTime& time, const int64_t& addSeconds) const
	{
		auto tp = std::chrono::system_clock::from_time_t(FindEpochTime(time));
		// 시간 더함
		tp += std::chrono::seconds(addSeconds);
		tp += std::chrono::seconds(g_KoreaTimeZoneS);

		// 타임 포인트
		const auto resultTime = std::chrono::system_clock::to_time_t(tp);
		std::tm tm{};

		if (gmtime_s(&tm, &resultTime) != 0)
		{
			// NOTE(iskim): 무시하면 되나?
		}

		THDateTime result = ConvertTmToTHDateTime(tm, time.Milliseconds);

		return result;
	}

	THDateTime TimeUtil::AddMinutesTHDateTimeToKSTDate(const THDateTime& time, const int64_t& addMinutes) const
	{
		return AddSecondsTHDateTimeToKSTDate(time, addMinutes * 60);
	}

	THDateTime TimeUtil::FindAfterSecondsTHDateTimeToKSTDate(const int64_t& afterSeconds) const
	{
		return AddSecondsTHDateTimeToKSTDate(FindKSTDate(), afterSeconds);
	}

	std::string TimeUtil::ToString(const THDateTime& time)
	{
		std::tm tm{};
		tm.tm_year = time.Year - g_InitializeYearValue;
		tm.tm_mon = time.Month - g_InitializeMonthValue;
		tm.tm_mday = time.Day;
		tm.tm_hour = time.Hour;
		tm.tm_min = time.Minute;
		tm.tm_sec = time.Second;
		tm.tm_isdst = -1;

		char buffer[80];
		if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm) == 0)
		{
			return {};
		}

		return std::string{ buffer };
	}

	std::string TimeUtil::SystemTimeToString(const _SYSTEMTIME& time)
	{
		std::tm tm{};
		tm.tm_year = time.wYear - g_InitializeYearValue;
		tm.tm_mon = time.wMonth - g_InitializeMonthValue;
		tm.tm_mday = time.wDay;
		tm.tm_hour = time.wHour;
		tm.tm_min = time.wMinute;
		tm.tm_sec = time.wSecond;
		tm.tm_isdst = -1;

		char buffer[80];
		if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm) == 0)
		{
			return {};
		}

		return std::string{ buffer };
	}

	std::string TimeUtil::ToISO8601Format(const THDateTime& time)
	{
		auto epochTime = GetInstance().FindEpochTime(time);
		std::string datetime(100, 0);
		std::tm tm{};
		gmtime_s(&tm, &epochTime);
		datetime.resize(std::strftime(datetime.data(), datetime.size(), "%FT%T", &tm));
		std::string msTime(10, 0);
		sprintf_s(msTime.data(), msTime.size(), ".%03dZ", time.Milliseconds);
		datetime.append(msTime);
		return datetime;
	}

	int64_t TimeUtil::ConvertTHDateTimeToKSTTimeMs(const THDateTime& time) const
	{
		return ConvertTHDateTimeToUTCTimeMs(time) + g_KoreaTimeZoneMS;
	}

	int64_t TimeUtil::FindKSTTime_t() const
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::system_clock::to_time_t(now);
	}

	THDateTime TimeUtil::ConvertKSTTimeMsToTHDateTime(const int64_t& timeMilli) const
	{
		const auto timeSec = timeMilli / g_SecToMs;
		std::tm tm{};

		gmtime_s(&tm, &timeSec); //thread safe

		return ConvertTmToTHDateTime(tm, timeMilli % g_SecToMs);
	}

	THDateTime TimeUtil::ConvertTmToTHDateTime(const std::tm& tm, const int32_t& milliSec) const
	{
		THDateTime result{};
		result.Year = static_cast<uint16_t>(tm.tm_year + g_InitializeYearValue);
		result.Month = static_cast<uint16_t>(tm.tm_mon + g_InitializeMonthValue);
		result.DayOfWeek = static_cast<uint16_t>(tm.tm_wday);
		result.Day = static_cast<uint16_t>(tm.tm_mday);
		result.Hour = static_cast<uint16_t>(tm.tm_hour);
		result.Minute = static_cast<uint16_t>(tm.tm_min);
		result.Second = static_cast<uint16_t>(tm.tm_sec);
		result.Milliseconds = milliSec;

		return result;
	}

	THDateTime TimeUtil::FindDateTimeByDayAndHour(const THDateTime& curDateTime, const int32_t& dayOfWeek, const int32_t& hour) const
	{
		THDateTime result{};

		const auto dayOffset = dayOfWeek - curDateTime.DayOfWeek;
		result = AddHourTHDateTimeToKSTDate(curDateTime, dayOffset * g_oneDayHours);

		// 시간 단위 밑으로는 절삭
		result.Hour = hour;
		result.Minute = 0;
		result.Second = 0;
		result.Milliseconds = 0;

		return result;
	}

	THDateTime TimeUtil::FindNextDateTimeByHour(const THDateTime& curDateTime, const int32_t& hour) const
	{
		auto result = curDateTime;
		if (hour <= curDateTime.Hour)
		{
			result = AddHourTHDateTimeToKSTDate(curDateTime, g_oneDayHours);
		}

		// 시간 단위 밑으로는 절삭
		result.Hour = hour;
		result.Minute = 0;
		result.Second = 0;
		result.Milliseconds = 0;

		return result;
	}

	THDateTime TimeUtil::FindNextDateTimeByDayAndHour(const THDateTime& curDateTime, const int32_t& dayOfWeek, const int32_t& hour) const
	{
		THDateTime result{};

		result = FindDateTimeByDayAndHour(curDateTime, dayOfWeek, hour);

		// 결과가 현재시간보다 작으면 일주일을 더한다
		if (result < curDateTime)
		{
			result = AddHourTHDateTimeToKSTDate(result, g_MaxDayOfWeek * g_oneDayHours);
		}

		return result;
	}

	//////////////////
	// UTC 함수들은 클라이언트에 시간 값을 보내줄 때에만 사용하도록 한다.
	// 이외의 시간 계산은 모두 KST로 하자
	/////////////////

	int64_t TimeUtil::FindUTCMs() const
	{
		//auto now = std::chrono::utc_clock::now();
		//return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());

		const auto now = std::chrono::system_clock::now();
		auto resultMS = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
		resultMS += (m_diffSec * g_SecToMs);

		return resultMS;
	}

	int64_t TimeUtil::FindUTCMs(const int64_t& spanMs) const
	{
		return FindUTCMs() + spanMs;
	}

	int64_t TimeUtil::FindCorrectIntervalSeconds(const THDateTime& start, const THDateTime& end) const
	{
		const auto startEpochSeconds = FindEpochTime(start);
		const auto endEpochSeconds = FindEpochTime(end);

		return endEpochSeconds - startEpochSeconds;
	}

	int64_t TimeUtil::FindCorrectIntervalDays(const THDateTime& start, const THDateTime& end) const
	{
		const auto startEpochDays = FindEpochTime(start) / 60 / 60 / 24;
		const auto endEpochDays = FindEpochTime(end) / 60 / 60 / 24;

		return endEpochDays - startEpochDays;
	}

	int64_t TimeUtil::FindCorrectIntervalWeeklys(const THDateTime& start, const THDateTime& end) const
	{
		const auto startEpochWeeklys = FindEpochTime(start) / 60 / 60 / 24 / 7;
		const auto endEpochWeeklys = FindEpochTime(end) / 60 / 60 / 24 / 7;

		return endEpochWeeklys - startEpochWeeklys;
	}

	int64_t TimeUtil::ConvertTHDateTimeToUTCTimeMs(const THDateTime& time) const
	{
		return FindEpochTime(time) * g_SecToMs + time.Milliseconds;
	}

	THDateTime TimeUtil::ConvertKSTDateTimeToUTCDateTime(const THDateTime& kstDateTime) const
	{
		// kst - 9 = utc
		return AddHourTHDateTimeToKSTDate(kstDateTime, static_cast<int32_t>(g_UTCToKSTOffset * -1));
	}

	int64_t TimeUtil::FindUTCTime_t() const
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::system_clock::to_time_t(now) + m_diffSec;
	}

	int64_t TimeUtil::FindEpochTime(const THDateTime& time) const
	{
		std::tm tm{};
		tm.tm_year = time.Year - g_InitializeYearValue;
		tm.tm_mon = time.Month - g_InitializeMonthValue;
		tm.tm_mday = time.Day;
		tm.tm_hour = time.Hour;
		tm.tm_min = time.Minute;
		tm.tm_sec = time.Second;
		tm.tm_isdst = -1;

		return std::mktime(&tm);
	}

	int64_t TimeUtil::FindDailyRenewalUTCTimeMS(const int32_t& resetHour) const
	{
		const auto now = std::chrono::system_clock::now();
		const auto timt = std::chrono::system_clock::to_time_t(now) + g_KoreaTimeZoneS + m_diffSec - static_cast<int64_t>(resetHour * 3600);

		const auto tomorrowTimt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::from_time_t(timt) + std::chrono::hours(24));
		std::tm nextTm{};
		gmtime_s(&nextTm, &tomorrowTimt);

		nextTm.tm_hour = 0;
		nextTm.tm_min = 0;
		nextTm.tm_sec = 0;
		nextTm.tm_isdst = -1;

		const auto nextRenewalTimeSec = std::mktime(&nextTm) + static_cast<int64_t>(resetHour * 3600);	// NOTE::일자 리셋타임을 체크를 위해 뺀 시간을 보정.

		return nextRenewalTimeSec * 1000;
	}

	// NOTE(iskim): start 시간 이후 seconds 주기로 시간이 흘렀을 때 현재 시간 이후의 시간이 반환되는 것을 보장함
	THDateTime TimeUtil::FindTimeAfterRepeatedSeconds(const THDateTime& start, const int64_t& seconds) const
	{
		if (seconds <= 0) return start;
		const auto& intervalSecs = FindCorrectIntervalSeconds(start, FindKSTDate());
		if (intervalSecs < 0) return start;
		return AddSecondsTHDateTimeToKSTDate(start, (intervalSecs / seconds + 1) * seconds);
	}

	THDateTime TimeUtil::MakeScheduleTime(const THDateTime& defaultDateTime, const int32_t& addDay, const THDateTime& settingTime) const
	{
		if (settingTime.Year != 1900) return{};

		const auto& defaultMs = ConvertTHDateTimeToKSTTimeMs(defaultDateTime);
		const auto& settingTimeMs = (settingTime.Hour * 3600 + settingTime.Minute * 60) * 1000;
		const auto& dayMs = addDay * 86400 * 1000;

		return ConvertKSTTimeMsToTHDateTime(defaultMs + settingTimeMs + dayMs);
	}

	int64_t TimeUtil::ConvertKSTMsToUTCMs(const int64_t& time) const
	{
		return time - g_KoreaTimeZoneMS;
	}

	int64_t TimeUtil::ConvertUTCMsToKSTMs(const int64_t& time) const
	{
		return time + g_KoreaTimeZoneMS;
	}

	THDateTime TimeUtil::FindLastDailyResetTime(const int32_t& resetHour) const
	{
		auto truncateHourDateTime = FindKSTDate();
		// 일 밑으로 절삭
		truncateHourDateTime.Hour = 0;
		truncateHourDateTime.Minute = 0;
		truncateHourDateTime.Second = 0;
		truncateHourDateTime.Milliseconds = 0;

		// resetHour시간 더한 시간이 미래면 하루 빼서 계산
		THDateTime result;
		result = AddHourTHDateTimeToKSTDate(truncateHourDateTime, resetHour);
		if (FindKSTDate() <= result)
		{
			result = AddHourTHDateTimeToKSTDate(result, g_oneDayHours * -1);
		}

		return result;
	}

	THDateTime TimeUtil::FindLastWeeklyResetTime(const int32_t& resetDay, const int32_t& resetHour) const
	{
		auto truncateHourDateTime = FindKSTDate();
		// 일 밑으로 절삭
		truncateHourDateTime.Hour = 0;
		truncateHourDateTime.Minute = 0;
		truncateHourDateTime.Second = 0;
		truncateHourDateTime.Milliseconds = 0;

		// 해당 주의 초기화 시간 계산
		auto compareDayOfWeekResult = truncateHourDateTime.DayOfWeek - resetDay;
		// 차이 일수 만큼 시간을 빼줌
		auto lastResetDateTime = AddHourTHDateTimeToKSTDate(truncateHourDateTime, compareDayOfWeekResult * g_oneDayHours * -1);

		// resetHour시간 더한 시간이 미래면 일주일 빼서 계산
		THDateTime result;
		result = AddHourTHDateTimeToKSTDate(lastResetDateTime, resetHour);
		if (FindKSTDate() < result)
		{
			result = AddHourTHDateTimeToKSTDate(result, g_oneDayHours * g_DaysPerWeek * -1);
		}

		return result;
	}

	THDateTime TimeUtil::FindLastMonthlyResetTime(const int32_t& resetDay, const int32_t& resetHour) const
	{
		auto targetResetTime = FindKSTDate();
		targetResetTime.Day = static_cast<uint16_t>(resetDay);
		targetResetTime.Hour = 0;
		targetResetTime.Minute = 0;
		targetResetTime.Second = 0;
		targetResetTime.Milliseconds = 0;

		// resetHour시간 더한 시간이 미래면 한달을 빼서 계산
		THDateTime result;
		result = AddHourTHDateTimeToKSTDate(targetResetTime, resetHour);
		if (FindKSTDate() < result)
		{
			result.Month = result.Month - 1;
			if (result.Month == 0)
			{
				result.Year = result.Year - 1;
				result.Month = 12;
			}
		}

		return result;
	}

	THDateTime TimeUtil::FindNextDailyResetTime(const int64_t& curMs, const int32_t& resetHour) const
	{
		return FindNextDailyResetTime(ConvertKSTTimeMsToTHDateTime(curMs), resetHour);
	}

	THDateTime TimeUtil::FindNextWeeklyResetTime(const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDayOfWeek) const
	{
		return FindNextWeeklyResetTime(ConvertKSTTimeMsToTHDateTime(curMs), resetDayOfWeek, resetHour);
	}

	THDateTime TimeUtil::FindNextMonthlyResetTime(const int64_t& curMs, const int32_t& resetHour, const int32_t& resetDay) const
	{
		return FindNextMonthlyResetTime(ConvertKSTTimeMsToTHDateTime(curMs), resetDay, resetHour);
	}

	THDateTime TimeUtil::FindNextDailyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetHour) const
	{
		auto truncateHourDateTime = lastUpdateTime.TruncateHoursAndLess();
		const auto& result = AddHourTHDateTimeToKSTDate(truncateHourDateTime, resetHour);
		if (lastUpdateTime < result) return result;
		return AddHourTHDateTimeToKSTDate(result, g_HoursPerDay);
	}

	THDateTime TimeUtil::FindNextWeeklyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDayOfWeek, const int32_t& resetHour) const
	{
		auto truncateHourDateTime = lastUpdateTime.TruncateHoursAndLess();
		auto diffDayOfWeek = lastUpdateTime.DayOfWeek - resetDayOfWeek;
		if (0 < diffDayOfWeek)
		{
			diffDayOfWeek = g_DaysPerWeek - diffDayOfWeek;
		}
		else if (diffDayOfWeek < 0)
		{
			diffDayOfWeek = std::abs(diffDayOfWeek);
		}

		const auto& result = AddHourTHDateTimeToKSTDate(truncateHourDateTime, diffDayOfWeek * g_HoursPerDay + resetHour);
		if (lastUpdateTime < result) return result;
		return AddHourTHDateTimeToKSTDate(result, g_DaysPerWeek * g_HoursPerDay);
	}

	THDateTime TimeUtil::FindNextMonthlyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDay, const int32_t& resetHour) const
	{
		auto targetResetTime = lastUpdateTime.TruncateHoursAndLess();
		targetResetTime.Day = static_cast<uint16_t>(resetDay);
		targetResetTime.Hour = static_cast<uint16_t>(resetHour);
		if (lastUpdateTime < targetResetTime) return targetResetTime;

		const bool isOverYear = targetResetTime.Month == 12;
		targetResetTime.Year = isOverYear ? targetResetTime.Year + 1 : targetResetTime.Year;
		targetResetTime.Month = isOverYear ? 1 : targetResetTime.Month + 1;
		return targetResetTime;
	}

	THDateTime TimeUtil::FindLastDailyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetHour) const
	{
		auto targetResetTime = lastUpdateTime.TruncateHoursAndLess();
		targetResetTime.Hour = static_cast<uint16_t>(resetHour);

		const auto timeDiff = FindCorrectIntervalSeconds(targetResetTime, lastUpdateTime);
		if (timeDiff <= 0)
		{
			targetResetTime = AddHourTHDateTimeToKSTDate(targetResetTime, -g_oneDayHours);
		}

		return targetResetTime;
	}

	THDateTime TimeUtil::FindLastWeeklyResetTime(const THDateTime& lastUpdateTime, const int32_t& resetDayOfWeek, const int32_t& resetHour) const
	{
		auto truncateHourDateTime = lastUpdateTime.TruncateHoursAndLess();
		auto diffDayOfWeek = lastUpdateTime.DayOfWeek - resetDayOfWeek;
		if (diffDayOfWeek < 0)
		{
			diffDayOfWeek = g_DaysPerWeek + diffDayOfWeek;
		}

		const auto& result = AddHourTHDateTimeToKSTDate(truncateHourDateTime, diffDayOfWeek * -g_HoursPerDay + resetHour);
		if (result < lastUpdateTime) return result;
		return AddHourTHDateTimeToKSTDate(result, g_DaysPerWeek * -g_HoursPerDay);
	}

	THDateTime TimeUtil::FindMinDateTime() const
	{
		return ConvertTHDateTimeFromString("2000-01-01 00:00:00");
	}

	THDateTime TimeUtil::FindMaxDateTime() const
	{
		return ConvertTHDateTimeFromString("3000-01-01 00:00:00");
	}

	int32_t TimeUtil::FindDayAfterNextResetHour(const int64_t& befMs, const int64_t& curMs, const int32_t& resetHour) const
	{
		const auto& nextDailyResetTime = FindNextDailyResetTime(befMs, resetHour);
		const auto& curDate = ConvertKSTTimeMsToTHDateTime(curMs);

		if (curDate < nextDailyResetTime)	return 0;

		const auto& intervalSec = util::TimeUtil::GetInstance().FindIntervalSeconds(nextDailyResetTime, curDate);
		const auto& periodDay = static_cast<int32_t>(intervalSec / g_secToDay) + 1;

		return periodDay;
	}
}
