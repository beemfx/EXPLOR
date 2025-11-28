/******************************************************************************
File: Timer.h
Class: EGTimer
Purpose: Timer class.

(c) 2011 Beem Software
******************************************************************************/
#pragma once

class EGTimer
{
public:

	struct egSystemTime
	{
		eg_int16 Year = 0;
		eg_int8 Month = 0;
		eg_int8 DayOfWeek = 0;
		eg_int8 Day = 0;
		eg_int8 Hour = 0;
		eg_int8 Minute = 0;
		eg_int8 Second = 0;
		eg_int16 Milliseconds = 0;

		eg_bool operator < ( const egSystemTime& rhs ) const
		{
			if( Year != rhs.Year )
			{
				return Year < rhs.Year;
			}
			if( Month != rhs.Month )
			{
				return Month < rhs.Month;
			}
			if( Day != rhs.Day )
			{
				return Day < rhs.Day;
			}
			if( Hour != rhs.Hour )
			{
				return Hour < rhs.Hour;
			}
			if( Minute != rhs.Minute )
			{
				return Minute < rhs.Minute;
			}
			if( Second != rhs.Second )
			{
				return Second < rhs.Second;
			}
			return Milliseconds < rhs.Milliseconds;
		}
	};

public:

	static egSystemTime GetSysTime();
	
public:

	EGTimer();
	void Init();
	void Shutdown();

	void    Update();                 //Once per frame.
	void    SoftReset();              //Causes elapsed time to become 0.
	eg_uint GetTimeMs()const;         //Time since time was last reset, in milliseconds.
	eg_uint GetElapsedMs()const;      //Time elapsed between calls to Update in milliseconds.
	eg_real GetTimeSec()const;        //Time since time was last reset, in seconds.
	eg_real GetElapsedSec()const;     //Time elapsed since last call to Update in seconds. 
	eg_real GetRawElapsedSec()const;  //Get's the elapsed time since the last call to Update.
	void    SetTimeMs(eg_uint nTime); //Called when loading a save to restore the timer state (should be the value of GetTimeMs when the game was saved).

private:
	struct  egInternalData;
	struct  egInternalData* m_pD;
	eg_byte m_InternalDataMem[64];
};

eg_uint64 Timer_GetRawTime();
eg_real   Timer_GetRawTimeElapsedSec( eg_uint64 Start , eg_uint64 End );