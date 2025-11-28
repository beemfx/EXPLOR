// (c) 2017 Beem Media

#pragma once

#include "EGParse.h"
#include "EGTween.h"

struct egTimelinePlayState
{
public:

	egTimelinePlayState() = default;
	egTimelinePlayState( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			NextToPlay = 0;
			Time = 0.f;
			bIsDone = false;
		}
	}

	void operator += ( eg_real DeltaTime )
	{
		Time += DeltaTime;
	}

	eg_bool IsDone() const { return bIsDone; }

private:
	friend class EGTimeline;

	eg_uint NextToPlay;
	eg_real Time;
	eg_bool bIsDone:1;
};

struct egTimelineKeyframe
{
	// Raw Data
	eg_real     Time = 0.f;
	eg_d_string Script;

	// Compiled Data
	eg_uint FirstAction = 0;
	eg_uint NumActions = 0;

	friend eg_bool operator < ( const egTimelineKeyframe& l , const egTimelineKeyframe& r )
	{
		return l.Time < r.Time;
	}
};

typedef egParseFuncT<128,10> egTimelineFnCall;

struct egTimelineAction
{
	egTimelineFnCall FnCall;

	eg_real RealParm( eg_size_t Index ) const	{ return EGString_ToReal(FnCall.Parms[Index]); }
	eg_string_crc StrCrcParm( eg_size_t Index ) const { return eg_string_crc(FnCall.Parms[Index]); }
	eg_bool BoolParm( eg_size_t Index ) const { return EGString_ToBool(FnCall.Parms[Index]); }
	eg_tween_animation_t TweenTypeParm( eg_size_t Index ) const;
	eg_int IntParm( eg_size_t Index ) const { return EGString_ToInt(FnCall.Parms[Index]); }
	eg_cpstr StrParm( eg_size_t Index ) const { return FnCall.Parms[Index]; }
};

class IEGTimelineHandler
{
public:

	virtual void OnTimelineAction( const egTimelineAction& Action ) = 0;
};
