#pragma once

#include "Base/BasicTypes.h"

#include <eastl/tuple.h>

namespace SG
{

	class Second
	{
	public:
		Second()
			:mSecond((UInt16)(-1))
		{}
		Second(UInt16 second)
			:mSecond(second)
		{
			// invalid time format
			if (second > 60)
				mSecond = (UInt16)(-1);
		}

		bool IsValid() const { return mSecond != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mSecond; }
	private:
		UInt16 mSecond;
	};

	class Minute
	{
	public:
		Minute()
			:mMinute((UInt16)(-1))
		{}
		Minute(UInt16 minute)
			:mMinute(minute)
		{
			// invalid time format
			if (minute > 60)
				mMinute = (UInt16)(-1);
		}

		bool IsValid() const { return mMinute != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mMinute; }
	private:
		UInt16 mMinute;
	};

	class Hour
	{
	public:
		Hour()
			:mHour((UInt16)(-1))
		{}
		Hour(UInt16 hour)
			:mHour(hour)
		{
			// invalid time format
			if (hour > 24)
				mHour = (UInt16)(-1);
		}

		bool IsValid() const { return mHour != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mHour; }
	private:
		UInt16 mHour;
	};

	class Day
	{
	public:
		Day()
			:mDay((UInt16)(-1))
		{}
		Day(UInt16 day)
			:mDay(day)
		{
			// invalid time format
			if (day == 0 || day > 31)
				mDay = (UInt16)(-1);
		}

		bool IsValid() const { return mDay != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mDay; }
	private:
		UInt16 mDay;
	};

	class Month
	{
	public:
		Month()
			:mMonth((UInt16)(-1))
		{}
		Month(UInt16 month)
			:mMonth(month)
		{
			// invalid time format
			if (month == 0 || month > 12)
				mMonth = (UInt16)(-1);
		}

		bool IsValid() const { return mMonth != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mMonth; }
	private:
		UInt16 mMonth;
	};

	class Year
	{
	public:
		Year()
			:mYear((UInt16)(-1))
		{}
		Year(UInt16 year)
			:mYear(year)
		{
		}

		bool IsValid() const { return mYear != (UInt16)(-1); }

		// implicit cast
		operator UInt16() const { return mYear; }
	private:
		UInt16 mYear;
	};

	//! Represent a point of time.
	struct TimePoint
	{
		Year   year;
		Month  month;
		Day    day;
		Hour   hour;
		Minute minute;
		Second second;

		bool IsValid()
		{
			return year.IsValid() &&
				month.IsValid() &&
				day.IsValid() &&
				hour.IsValid() &&
				minute.IsValid() &&
				second.IsValid();
		}
	};

	SG_INLINE bool operator>(const TimePoint& lhs, const TimePoint& rhs)
	{
		return eastl::tie(lhs.year, lhs.month, lhs.day, lhs.hour, lhs.minute, lhs.second) >
			eastl::tie(rhs.year, rhs.month, rhs.day, rhs.hour, rhs.minute, rhs.second);
	}

	SG_INLINE bool operator>=(const TimePoint& lhs, const TimePoint& rhs)
	{
		return eastl::tie(lhs.year, lhs.month, lhs.day, lhs.hour, lhs.minute, lhs.second) >=
			eastl::tie(rhs.year, rhs.month, rhs.day, rhs.hour, rhs.minute, rhs.second);
	}

	SG_INLINE bool operator<(const TimePoint& lhs, const TimePoint& rhs)
	{
		return !operator>=(lhs, rhs);
	}

	SG_INLINE bool operator<=(const TimePoint& lhs, const TimePoint& rhs)
	{
		return !operator>(lhs, rhs);
	}

	SG_INLINE bool operator==(const TimePoint& lhs, const TimePoint& rhs)
	{
		return eastl::tie(lhs.year, lhs.month, lhs.day, lhs.hour, lhs.minute, lhs.second) ==
			eastl::tie(rhs.year, rhs.month, rhs.day, rhs.hour, rhs.minute, rhs.second);
	}

	SG_INLINE bool operator!=(const TimePoint& lhs, const TimePoint& rhs)
	{
		return !(operator==(lhs, rhs));
	}

}