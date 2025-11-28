// (c) 2017 Beem Media

#pragma once

#include "EGTextFormat.h"
#include "EGTimer.h"

struct exDateInfo
{
	eg_int  Year;
	eg_uint MonthOfYear;
	eg_uint DayOfMonth;
	eg_uint DayOfWeek;

	eg_uint HourOfDay;
	eg_uint MinuteOfHour;
	eg_uint SecondOfMinute;
};

struct exTime
{
	eg_int Day; // 0 is the first day of the first year of the game.
	eg_int MilliSecondOfDay; // 0 is the start of the day 1 is 1 millisecond in, etc...

	static const eg_uint DAYS_IN_YEAR = 365;

	static const eg_int MILLISECONDS_IN_DAY = 24*60*60*1000;
	static const eg_int SECONDS_IN_DAY = 24*60*60;
	static const eg_int MINUTES_IN_DAY = 24*60;
	static const eg_int HOURS_IN_DAY = 24;
	static const eg_int SECONDS_IN_MINUTE = 60;
	static const eg_int MINUTES_IN_HOUR = 60;

	exTime( eg_ctor_t Ct );
	void AddDays( eg_int NumDays );
	void AddMilliseconds( eg_int NumMilliseconds );
	void AddSeconds( eg_int NumSeconds );
	void AddMinutes( eg_int NumMinutes ){ AddSeconds( NumMinutes*SECONDS_IN_MINUTE ); }
	void AddHours( eg_int NumHours ){ AddMinutes( NumHours*MINUTES_IN_HOUR ); }

	eg_real GetTimeOfDayNormalized() const;

	exDateInfo GetDateInfo() const;

private:

	
};

struct exTimeSmoother
{
public:

	void Update( eg_real DeltaTime );
	void HandleTimeChange( const exTime& Time );
	eg_real GetSmoothTimeOfDayNormalized() const;

private:

	eg_bool m_bSmoothTimeActive = false;
	eg_real m_SmoothTimeDuration = .5f;
	eg_real m_SmoothTimeStart = 0.f;
	eg_real m_SmoothTimeEnd = 0.f;
	eg_real m_SmoothTimeCurrent = 0.f;
	eg_real m_SmoothTimeElapsed = 0.f;
	eg_bool m_bHasNextSmoothTime = false;
	eg_real m_SmoothTimeNext = 0.f;
};

class ExTimeFormatter : public IEGCustomFormatHandler
{
private:

	const exTime*const m_Time;
	const EGTimer::egSystemTime*const m_SysTime;
	eg_cpstr m_Flags;

public:

	ExTimeFormatter( const exTime* InTime , eg_cpstr Flags ): m_Time( InTime ) , m_SysTime(nullptr) , m_Flags(Flags){ }
	ExTimeFormatter( const EGTimer::egSystemTime* InTime , eg_cpstr Flags ): m_Time(nullptr) , m_SysTime(InTime) , m_Flags(Flags){ }

	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final;

	static eg_string_crc MonthToName( eg_uint Month , eg_bool bAbbr );
	static eg_string_crc DayOfWeekToName( eg_uint DayOfWeek , eg_bool bAbbr );
};
