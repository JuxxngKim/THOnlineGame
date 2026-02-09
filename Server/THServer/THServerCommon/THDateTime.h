#pragma once

struct THDateTime
{
    uint16_t Year = 0;
    uint16_t Month = 0;
    uint16_t DayOfWeek = 0;
    uint16_t Day = 0;
    uint16_t Hour = 0;
    uint16_t Minute = 0;
    uint16_t Second = 0;
    uint16_t Milliseconds = 0;

    bool IsEmpty() const
    {
        return Year == 0;
    }

    bool IsValid() const
    {
        // 검증: Year (1601년 이상)
        if (Year < 1601) return false;
        if (Month < 1 || 12 < Month) return false;

        // 검증: Day (1 ~ 월의 최대 날짜)
        static const int32_t daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        int32_t maxDay = daysInMonth[Month - 1];

        // 윤년 체크 (2월 처리)
        if (Month == 2 && ((Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0))) {
            maxDay = 29; // 윤년의 2월은 29일까지 가능
        }

        if (Day < 1 || maxDay < Day) return false;
        if (DayOfWeek < 0 || 6 < DayOfWeek) return false;
        if (Hour < 0 || 23 < Hour) return false;
        if (Minute < 0 || 59 < Minute) return false;
        if (Second < 0 || 59 < Second) return false;
        if (Milliseconds < 0 || 999 < Milliseconds) return false;

        return true;
    }

    THDateTime TruncateHoursAndLess() const
    {
        THDateTime result = *this;
        result.Hour = 0;
        result.Minute = 0;
        result.Second = 0;
        result.Milliseconds = 0;
        return result;
    }

    bool IsMaxDate() const
    {
        return Year == 3000 &&
            Month == 1 &&
            Day == 1 &&
            Hour == 0 &&
            Minute == 0 &&
            Second == 0 &&
            Milliseconds == 0;
    }

    void SetMaxDate()
    {
        Year = 3000;
        Month = 1;
        Day = 1;
        Hour = 0;
        Minute = 0;
        Second = 0;
        Milliseconds = 0;
    }

    bool operator <(const THDateTime& rhs) const
    {
        return std::tie(Year, Month, Day, Hour, Minute, Second, Milliseconds) <
            std::tie(rhs.Year, rhs.Month, rhs.Day, rhs.Hour, rhs.Minute, rhs.Second, rhs.Milliseconds);
    }

    bool operator >=(const THDateTime& rhs) const
    {
        return !(*this < rhs);
    }

    bool operator ==(const THDateTime& rhs) const
    {
        return Year == rhs.Year &&
            Month == rhs.Month &&
            Day == rhs.Day &&
            Hour == rhs.Hour &&
            Minute == rhs.Minute &&
            Second == rhs.Second &&
            Milliseconds == rhs.Milliseconds;
    }

    bool operator >(const THDateTime& rhs) const
    {
        return *this >= rhs && *this != rhs;
    }

    bool operator !=(const THDateTime& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator <=(const THDateTime& rhs) const
    {
        return *this < rhs || *this == rhs;
    }

    bool IsSameDay(const THDateTime& rhs) const
    {
        return Year == rhs.Year &&
            Month == rhs.Month &&
            Day == rhs.Day;
    }
};
