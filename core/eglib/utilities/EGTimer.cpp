/******************************************************************************
File: Timer.h
Class: EGTimer
Purpose: Timer class.

(c) 2011 Beem Software
******************************************************************************/
#include "EGTimer.h"

#if defined( __WIN32__ ) || defined( __WIN64__ )
#include "EGWindowsAPI.h"

typedef LONGLONG eg_timer_time;
static const eg_uint TIMER_PREC_MULT = 1000000;

struct EGTimer::egInternalData
{
	eg_timer_time nFreq;     //Frequency of performance timer.
	eg_timer_time nStartTm;  //When the timer was started.
	eg_timer_time nTm;       //Time at update.
	eg_timer_time nLastTm;   //Time at previous call to update.
	eg_timer_time nElapseTm; //Time elapsed between calls to Update.
	//Floating point times in seconds.
	eg_real fTm;
	eg_real fLastTm;
	eg_real fElapseTm;
	//Dword times in milliseconds.
	eg_uint nTm32;
	eg_uint nLastTm32;
	eg_uint nElapseTm32;

	egInternalData()
	: nFreq(0)
	, nTm(0)
	, nStartTm(0) 
	{
		
	}
};


EGTimer::egSystemTime EGTimer::GetSysTime()
{
	egSystemTime Out;

	SYSTEMTIME SysTime;
	zero( &SysTime );
	GetLocalTime( &SysTime );

	Out.Year = EG_To<eg_int16>(SysTime.wYear);
	Out.Month = EG_To<eg_int8>(SysTime.wMonth);
	Out.DayOfWeek = EG_To<eg_int8>(SysTime.wDayOfWeek);
	Out.Day = EG_To<eg_int8>(SysTime.wDay);
	Out.Hour = EG_To<eg_int8>(SysTime.wHour);
	Out.Minute = EG_To<eg_int8>(SysTime.wMinute);
	Out.Second = EG_To<eg_int8>(SysTime.wSecond);
	Out.Milliseconds = EG_To<eg_int16>(SysTime.wMilliseconds);

	return Out;
}

//EGTimer class methods...
EGTimer::EGTimer()
{
	assert( sizeof(m_InternalDataMem) == sizeof(egInternalData));
	m_pD = new (&m_InternalDataMem)egInternalData();

}

void EGTimer::Init()
{
	if(!QueryPerformanceFrequency((LARGE_INTEGER*)&m_pD->nFreq))
	{
		assert(false);
		return;
	}
		
	::QueryPerformanceCounter((LARGE_INTEGER*)&m_pD->nStartTm);
	
	SoftReset();
}
void EGTimer::Shutdown()
{

}

void EGTimer::Update()
{
	//Update the current time.
	m_pD->nLastTm=m_pD->nTm;
	QueryPerformanceCounter((LARGE_INTEGER*)&m_pD->nTm);
	m_pD->nTm-=m_pD->nStartTm;
	m_pD->nElapseTm=m_pD->nTm-m_pD->nLastTm;

	//Calculate floating point times:
	m_pD->fLastTm =   m_pD->fTm;
	m_pD->fTm =       (eg_real)(m_pD->nTm/(eg_real)m_pD->nFreq);
	m_pD->fElapseTm = m_pD->fTm-m_pD->fLastTm;
	
	//Calculate 32 bit times:
	m_pD->nLastTm32 =   m_pD->nTm32;
	m_pD->nTm32 =       (eg_uint)((TIMER_PREC_MULT*m_pD->nTm)/m_pD->nFreq);
	m_pD->nElapseTm32 = m_pD->nTm32-m_pD->nLastTm32;
}

eg_real EGTimer::GetRawElapsedSec()const
{
	eg_timer_time Now;
	QueryPerformanceCounter((LARGE_INTEGER*)&Now);
	Now -= m_pD->nStartTm;
	eg_timer_time ElapsedRaw = (Now - m_pD->nTm)*TIMER_PREC_MULT;

	eg_real ElapsedMs = eg_real(ElapsedRaw/m_pD->nFreq);
	return ElapsedMs/eg_real(TIMER_PREC_MULT);
}

void EGTimer::SoftReset()
{
	Update();
	Update();
}
eg_uint EGTimer::GetTimeMs()const
{
	return m_pD->nTm32;
}

////////////////////////////////////////////////
//	EGTimer::GetElapsedTime returns the amount
//	of milliseconds since the last call to Update
//	so in effect this will report how much time
//	has passed between two frames.
////////////////////////////////////////////////
eg_uint EGTimer::GetElapsedMs()const
{
	return m_pD->nElapseTm32;
}

//Note that the GetF methods return a floating
//point time that represents a decimal number of
//the seconds as opposed to milliseconds for the
//Get methods.

eg_real EGTimer::GetTimeSec()const
{
	return m_pD->fTm;
}

eg_real EGTimer::GetElapsedSec()const
{
	return m_pD->fElapseTm;
}

void EGTimer::SetTimeMs(eg_uint nTime)
{
	//We need to adjust the time for the performance time.
	eg_timer_time nTimeAdj = nTime;
	nTimeAdj *= m_pD->nFreq;
	nTimeAdj /= TIMER_PREC_MULT;
	m_pD->nStartTm = m_pD->nTm-nTimeAdj;
	//A reset insures that the next call to get an elapsed time is zero
	//and also the next call to update won't have funny data.
	SoftReset();
}

eg_uint64 Timer_GetRawTime()
{
	eg_uint64 Out;
	QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>(&Out) );
	return Out;
}
eg_real Timer_GetRawTimeElapsedSec( eg_uint64 Start , eg_uint64 End )
{
	assert( Start <= End );
	eg_uint64 Diff = End - Start;
	eg_uint64 Freq;
	QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>(&Freq) );
	eg_uint64 TimeMs = (Diff*1000)/Freq;
	eg_real TimeSec = TimeMs/1000.f;
	return TimeSec;
}

#else
#pragma __WARNING__("No platform for "__FILE__" class!" )
#endif