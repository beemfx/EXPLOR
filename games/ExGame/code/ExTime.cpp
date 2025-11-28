// (c) 2017 Beem Media

#include "ExTime.h"
#include "EGDebugText.h"

exDateInfo exTime::GetDateInfo() const
{
	exDateInfo Out;
	zero( &Out );

	auto DayOfYearToMonth =  []( eg_uint DayOfYear , eg_uint* DayOfMonthOut ) -> eg_uint
	{
		static const struct exMonthMap
		{
			eg_uint Month;
			eg_uint DaysInMonth;
		}
		MonthMap[] =
		{
			{ 1  , 31 },
			{ 2  , 28 },
			{ 3  , 31 },
			{ 4  , 30 },
			{ 5  , 31 },
			{ 6  , 30 },
			{ 7  , 31 },
			{ 8  , 31 },
			{ 9  , 30 },
			{ 10 , 31 },
			{ 11 , 30 },
			{ 12 , 31 },
		};

		eg_uint DaysPast = 0;
		for( const exMonthMap& Item : MonthMap )
		{
			DaysPast += Item.DaysInMonth;
			if( DayOfYear <= DaysPast )
			{
				if( DayOfMonthOut )
				{
					*DayOfMonthOut = Item.DaysInMonth + (DayOfYear - DaysPast);
				}
				return Item.Month;
			}
		};

		assert( false );

		return 0;
	};

	// For reference Time.Day == 0 is Day 1 of year 1 CE...

	eg_bool bIsBCE = Day < 0;
	eg_uint DayOfYear = 0;

	if( bIsBCE )
	{
		eg_uint DayFromEndOfYearOneBCE = EG_Abs(Day+1);
		Out.Year = -static_cast<eg_int>((DayFromEndOfYearOneBCE/exTime::DAYS_IN_YEAR+1));
		DayOfYear = exTime::DAYS_IN_YEAR  - (DayFromEndOfYearOneBCE%exTime::DAYS_IN_YEAR);
		Out.DayOfWeek = (7-EG_Abs(Day)%7);
		// Since day 0 is the first day of CE and the last day of BCE we need
		// to adjust the day of week by 1
		Out.DayOfWeek = (((Out.DayOfWeek-1)+1)%7) + 1;
	}
	else
	{
		Out.Year = (Day/exTime::DAYS_IN_YEAR) + 1;
		DayOfYear = (Day%exTime::DAYS_IN_YEAR) + 1;	
		Out.DayOfWeek = (Day%7) + 1;
	}

	// Adjustment to make the game day of week line up with 2017...
	Out.DayOfWeek = (((Out.DayOfWeek-1)+6)%7) + 1;

	Out.MonthOfYear = DayOfYearToMonth( DayOfYear , &Out.DayOfMonth );

	// Time of day.
	assert( EG_IsBetween<eg_int>( MilliSecondOfDay , 0 , exTime::MILLISECONDS_IN_DAY-1 ) );
	eg_int SecondOfDay = MilliSecondOfDay/1000;
	Out.SecondOfMinute = SecondOfDay%60;
	eg_int MinuteOfDay = SecondOfDay/60;
	Out.MinuteOfHour = MinuteOfDay%60;
	eg_int HourOfDay = MinuteOfDay/60;
	Out.HourOfDay = HourOfDay%24;

	return Out;
}

class ExYearFormatter : public IEGCustomFormatHandler
{
private:
	
	eg_int m_Year;

public:

	ExYearFormatter( eg_int Year ): m_Year( Year ){ }

	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final
	{
		unused( Flags );
		if( m_Year <= 0 )
		{
			Formatter->SetText( EGFormat( L"{0} {1}" , static_cast<const eg_loc_char*>(EGString_ToWide(EGString_Format( "%d" , EG_Abs(m_Year) ))) , eg_loc("BCE","BCE") ).GetString() );
		}
		else
		{
			Formatter->SetText( EGFormat( L"{0} {1}" , static_cast<const eg_loc_char*>(EGString_ToWide(EGString_Format( "%d" , m_Year ))) , eg_loc("CE","CE") ).GetString() );
		}
	}
};

class ExTimeOfDayFormatter : public IEGCustomFormatHandler
{
private:

	eg_uint m_Second;
	eg_uint m_Minute;
	eg_uint m_Hour;

public:

	ExTimeOfDayFormatter( eg_uint Hour , eg_uint Minute , eg_uint Second )
	: m_Hour(Hour)
	, m_Minute(Minute)
	, m_Second(Second)
	{

	}

	virtual void FormatText( eg_cpstr Flags , class EGTextParmFormatter* Formatter ) const override final
	{
		unused( Flags );

		auto FormatNumber = []( eg_uint Number , eg_bool bIsHour ) -> eg_string_small
		{
			assert( 0 <= Number && Number <= 99 );
			eg_uint Tens = Number / 10;
			eg_uint Ones = (Number-Tens*10);
			eg_string_small Out( CT_Clear );
			if( !bIsHour || Tens != 0 )
			{
				Out.Append( static_cast<eg_char>(Tens) + '0' );
			}
			Out.Append( static_cast<eg_char>(Ones) + '0' );
			return Out;
		};

		eg_bool bIsPM = m_Hour >= 12;
		eg_uint AdjHour = 0;
		
		if( 0 == m_Hour )
		{
			AdjHour = 12;
		}
		else if( 1 <= m_Hour && m_Hour <= 12 )
		{
			AdjHour = m_Hour;
		}
		else  if( 13 <= m_Hour && m_Hour <= 23 )
		{
			AdjHour = m_Hour - 12;
		}

		eg_string_small StrHour = FormatNumber( AdjHour , true );
		eg_string_small StrMinute = FormatNumber( m_Minute , false );
		eg_string_small StrSecond = FormatNumber( m_Second , false );

		eg_string_crc Suffix = bIsPM ? eg_loc("PM","PM") : eg_loc("AM","AM");
		Formatter->SetText( EGFormat( L"{0}:{1}:{2} {3}" , static_cast<const eg_loc_char*>(EGString_ToWide(StrHour)) , static_cast<const eg_loc_char*>(EGString_ToWide(StrMinute)) , static_cast<const eg_loc_char*>(EGString_ToWide(StrSecond)) , Suffix ).GetString() );
	}

};

void ExTimeFormatter::FormatText( eg_cpstr Flags, class EGTextParmFormatter* Formatter ) const
{
	eg_cpstr FlagsToUse = nullptr != m_Flags ? m_Flags : Flags;
	
	const eg_string_crc BaseFlag = Formatter->GetNextFlag( &FlagsToUse );

	exDateInfo DateInfo;
	if( m_Time )
	{
		DateInfo = m_Time->GetDateInfo();
	}
	else if( m_SysTime )
	{
		DateInfo.Year = m_SysTime->Year;
		DateInfo.MonthOfYear = m_SysTime->Month;
		DateInfo.DayOfMonth = m_SysTime->Day;
		DateInfo.DayOfWeek = m_SysTime->DayOfWeek;
		if( DateInfo.DayOfWeek == 0 )
		{
			// Adjust cuz system time uses 0 as sunday.
			DateInfo.DayOfWeek = 7;
		}
		DateInfo.HourOfDay = m_SysTime->Hour;
		DateInfo.MinuteOfHour = m_SysTime->Minute;
		DateInfo.SecondOfMinute = m_SysTime->Second;
	}
	else
	{
		DateInfo = exTime( CT_Default ).GetDateInfo();
	}

	switch_crc( BaseFlag )
	{
		case_crc("WEEKDAY"):
		{
			const eg_string_crc FullFlag = Formatter->GetNextFlag( &FlagsToUse );
			const eg_bool bAbbr = eg_crc("ABBR") == FullFlag;

			Formatter->SetText( eg_loc_text(DayOfWeekToName(DateInfo.DayOfWeek,bAbbr)).GetString() );
		} break;
		case_crc("MONTH"):
		{
			const eg_string_crc FullFlag = Formatter->GetNextFlag( &FlagsToUse );
			const eg_bool bAbbr = eg_crc("ABBR") == FullFlag;

			Formatter->SetText( eg_loc_text(MonthToName(DateInfo.MonthOfYear,bAbbr)).GetString() );
		} break;
		case_crc("DAY"):
		{
			Formatter->SetNumber( DateInfo.DayOfMonth );
		} break;
		case_crc("YEAR"):
		{
			Formatter->SetText( EGFormat( L"{0}" , &ExYearFormatter(DateInfo.Year) ).GetString() );
		} break;
		case_crc("FULL"):
		{
			const eg_string_crc FullFlag = Formatter->GetNextFlag( &FlagsToUse );
			const eg_bool bAbbr = eg_crc("ABBR") == FullFlag;

			const eg_loc_text Text = EGFormat( L"{0} {1} {2}, {3}" , DayOfWeekToName(DateInfo.DayOfWeek,bAbbr) , MonthToName(DateInfo.MonthOfYear,bAbbr) , static_cast<eg_uint>(DateInfo.DayOfMonth) , &ExYearFormatter(DateInfo.Year) );
			Formatter->SetText( Text.GetString() );
		} break;
		case_crc("TIME"):
		{
			eg_loc_text Text = EGFormat( L"{0}" , &ExTimeOfDayFormatter( DateInfo.HourOfDay , DateInfo.MinuteOfHour , DateInfo.SecondOfMinute) );
			Formatter->SetText( Text.GetString() );
		} break;
	}
}

eg_string_crc ExTimeFormatter::MonthToName( eg_uint Month , eg_bool bAbbr )
{
	if( bAbbr )
	{
		switch( Month )
		{
			case 1 : return eg_loc("TimeAbbrMonthJan","JAN");
			case 2 : return eg_loc("TimeAbbrMonthFeb","FEB");
			case 3 : return eg_loc("TimeAbbrMonthMar","MAR");
			case 4 : return eg_loc("TimeAbbrMonthApr","APR");
			case 5 : return eg_loc("TimeAbbrMonthMay","MAY");
			case 6 : return eg_loc("TimeAbbrMonthJun","JUN");
			case 7 : return eg_loc("TimeAbbrMonthJul","JUL");
			case 8 : return eg_loc("TimeAbbrMonthAug","AUG");
			case 9 : return eg_loc("TimeAbbrMonthSep","SEP");
			case 10: return eg_loc("TimeAbbrMonthOct","OCT");
			case 11: return eg_loc("TimeAbbrMonthNov","NOV");
			case 12: return eg_loc("TimeAbbrMonthDec","DEC");
		}
	}
	else
	{
		switch( Month )
		{
			case 1 : return eg_loc("TimeMonthJan","January");
			case 2 : return eg_loc("TimeMonthFeb","February");
			case 3 : return eg_loc("TimeMonthMar","March");
			case 4 : return eg_loc("TimeMonthApr","April");
			case 5 : return eg_loc("TimeMonthMay","May");
			case 6 : return eg_loc("TimeMonthJun","June");
			case 7 : return eg_loc("TimeMonthJul","July");
			case 8 : return eg_loc("TimeMonthAug","August");
			case 9 : return eg_loc("TimeMonthSep","September");
			case 10: return eg_loc("TimeMonthOct","October");
			case 11: return eg_loc("TimeMonthNov","November");
			case 12: return eg_loc("TimeMonthDec","December");
		}
	}

	return eg_loc("TimeAbbrMonthUnk","Unknown Month");
}

eg_string_crc ExTimeFormatter::DayOfWeekToName( eg_uint DayOfWeek , eg_bool bAbbr )
{
	if( bAbbr )
	{
		switch( DayOfWeek )
		{
			case 1: return eg_loc("TimeAbbrWeekDayMon","MON");
			case 2: return eg_loc("TimeAbbrWeekDayTue","TUE");
			case 3: return eg_loc("TimeAbbrWeekDayWed","WED");
			case 4: return eg_loc("TimeAbbrWeekDayThu","THU");
			case 5: return eg_loc("TimeAbbrWeekDayFri","FRI");
			case 6: return eg_loc("TimeAbbrWeekDaySat","SAT");
			case 7: return eg_loc("TimeAbbrWeekDaySun","SUN");
		}
	}
	else
	{
		switch( DayOfWeek )
		{
			case 1: return eg_loc("TimeWeekDayMon","Monday");
			case 2: return eg_loc("TimeWeekDayTue","Tuesday");
			case 3: return eg_loc("TimeWeekDayWed","Wednesday");
			case 4: return eg_loc("TimeWeekDayThu","Thursday");
			case 5: return eg_loc("TimeWeekDayFri","Friday");
			case 6: return eg_loc("TimeWeekDaySat","Saturday");
			case 7: return eg_loc("TimeWeekDaySun","Sunday");
		}
	}

	return eg_loc("DayAbbrWeekDayUnk","Unknown Day");
}

exTime::exTime( eg_ctor_t Ct )
{
	if( Ct == CT_Clear || Ct == CT_Default )
	{
		Day = 0;
		MilliSecondOfDay = 0;
	}
}

void exTime::AddDays( eg_int NumDays )
{
	Day += NumDays;
}

void exTime::AddMilliseconds( eg_int NumMilliseconds )
{
	MilliSecondOfDay += NumMilliseconds;
	if( MilliSecondOfDay >= MILLISECONDS_IN_DAY )
	{
		while( MilliSecondOfDay >= MILLISECONDS_IN_DAY )
		{
			MilliSecondOfDay -= MILLISECONDS_IN_DAY;
			AddDays( 1 );
		}
	}
	else if( MilliSecondOfDay < 0 )
	{
		while( MilliSecondOfDay < 0 )
		{
			MilliSecondOfDay += MILLISECONDS_IN_DAY;
			AddDays( -1 );
		}
	}

	assert( EG_IsBetween<eg_int>(MilliSecondOfDay,0,MILLISECONDS_IN_DAY-1) );
}

void exTime::AddSeconds( eg_int NumSeconds )
{
	// to avoid overflow we never add more than a day's worth of seconds at
	// a time.
	eg_bool bIsSubtracting = NumSeconds < 0;
	eg_int SecondsLeft = EG_Abs(NumSeconds);

	while( SecondsLeft > 0 )
	{
		eg_int DeltaSeconds = SecondsLeft < SECONDS_IN_DAY ? SecondsLeft : SECONDS_IN_DAY;

		if( bIsSubtracting )
		{
			AddMilliseconds( -SecondsLeft*1000 );
		}
		else
		{
			AddMilliseconds( SecondsLeft*1000 );
		}
		SecondsLeft -= DeltaSeconds;
	}
}

eg_real exTime::GetTimeOfDayNormalized() const
{
	return EGMath_GetMappedRangeValue( static_cast<eg_real>(MilliSecondOfDay) , eg_vec2(0.f,static_cast<eg_real>(MILLISECONDS_IN_DAY)) , eg_vec2(0.f,1.f) );
}

void exTimeSmoother::Update( eg_real DeltaTime )
{
	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Time of Day: {0} Smooth Time: {1} Next: {2} {3}-{4}" , 0.f , m_SmoothTimeCurrent , m_SmoothTimeNext , m_SmoothTimeStart , m_SmoothTimeEnd ) );
	if( m_bSmoothTimeActive )
	{
		m_SmoothTimeElapsed += DeltaTime;
		if( m_SmoothTimeElapsed >= m_SmoothTimeDuration )
		{
			m_bSmoothTimeActive = false;
			m_SmoothTimeElapsed = 0.f;
			if( m_SmoothTimeEnd >= 1.f )
			{
				m_SmoothTimeEnd -= 1.f;
			}
			m_SmoothTimeCurrent = m_SmoothTimeEnd;
			m_SmoothTimeStart = m_SmoothTimeEnd;


			if( m_bHasNextSmoothTime )
			{
				m_bHasNextSmoothTime = false;
				m_bSmoothTimeActive = true;
				m_SmoothTimeEnd = m_SmoothTimeNext;
				if( m_SmoothTimeEnd < m_SmoothTimeStart )
				{
					//Special case where we are actually going into the next day.
					m_SmoothTimeEnd += 1.f;
					assert( m_SmoothTimeEnd <= 2.f );
				}
			}
		}
		else
		{
			m_SmoothTimeCurrent = EGMath_GetMappedRangeValue( m_SmoothTimeElapsed , eg_vec2(0.f,m_SmoothTimeDuration) , eg_vec2(m_SmoothTimeStart,m_SmoothTimeEnd) );
		}

		if( m_SmoothTimeCurrent > 1.f )
		{
			// If we are overlapping into the next day correct the time.
			m_SmoothTimeCurrent -= 1.f;
			assert( m_SmoothTimeCurrent <= 1.f );
		}
	}
}

void exTimeSmoother::HandleTimeChange( const exTime& Time )
{
	m_SmoothTimeNext = Time.GetTimeOfDayNormalized();
	m_bHasNextSmoothTime = true;
	if( !m_bSmoothTimeActive )
	{
		m_bSmoothTimeActive = true;
		m_SmoothTimeElapsed = m_SmoothTimeDuration;
	}
}

eg_real exTimeSmoother::GetSmoothTimeOfDayNormalized() const
{
	return m_SmoothTimeCurrent;
}
