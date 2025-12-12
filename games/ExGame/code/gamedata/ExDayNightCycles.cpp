// (c) 2017 Beem Media

#include "ExDayNightCycles.h"
#include "EGEngineConfig.h"
#include "EGXMLBase.h"

EG_CLASS_DECL( ExDayNightCycles )

ExDayNightCycles* ExDayNightCycles::s_Inst = nullptr;

void ExDayNightCycles::Init( eg_cpstr Filename )
{
	assert( s_Inst == nullptr );
	s_Inst = EGDataAsset::LoadDataAsset<ExDayNightCycles>( EGString_ToWide(Filename) );
	assert( nullptr != s_Inst ); // Will crash.
	Get();
}

void ExDayNightCycles::Deinit()
{
	assert( nullptr != s_Inst );
	if( s_Inst )
	{
		EGDeleteObject( s_Inst );
		s_Inst = nullptr;
	}
}

void ExDayNightCycles::GetCycles( eg_string_crc Id , EGArray<exDayNightCycleState>& Out )
{
	auto GetColorById = [this]( const eg_string_crc& CrcId ) -> exDayNightCycleColor
	{
		for( const exDayNightCycleColor& Color : m_Colors )
		{
			if( Color.Id == CrcId )
			{
				return Color;
			}
		}

		return exDayNightCycleColor();
	};

	Out.Clear();

	for( const exDayNightCycle& Cycle : m_Cycles )
	{
		if( Cycle.Id == Id )
		{
			for( const exDayNightCycleBlock& Block : Cycle.CycleBlocks )
			{
				exDayNightCycleColor Color = GetColorById( Block.CycleColorId );

				exDayNightCycleState NewCycleState;
				NewCycleState.NormalizedTime = EG_Clamp( Block.Hour/24.f , 0.f , 1.f );
				NewCycleState.SunColor = eg_color(Color.SunColor);
				NewCycleState.TorchColor = eg_color(Color.TorchColor);
				NewCycleState.AmbientIntensity = Color.AmbientIntensity;
				NewCycleState.TorchIntensity = Color.TorchIntensity;
				NewCycleState.SunIntensity = Color.SunIntensity;

				Out.Append( NewCycleState );
			}
		}
	}

	Out.Sort( []( const exDayNightCycleState& lhs , const exDayNightCycleState& rhs ){ return lhs.NormalizedTime < rhs.NormalizedTime; } );
}
