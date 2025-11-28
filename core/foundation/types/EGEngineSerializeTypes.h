// (c) 2017 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGEngineSerializeTypes.reflection.h"


egreflect enum class eg_light_t
{
	Point,
	Direction,
};

egreflect struct eg_light_ed
{
	egprop eg_light_t Type         = eg_light_t::Point;
	egprop eg_vec3    Direction    = eg_vec3(0.f,0.f,1.f);
	egprop eg_vec3    Position     = eg_vec3(0.f,0.f,0.f);
	egprop eg_color32 Color        = eg_color32::White;
	egprop eg_real    Range        = 5.f;
	egprop eg_real    FallofRadius = 2.5f;
	egprop eg_real    Intensity    = 1.f;

	void RefreshVisibleProperties( egRflEditor& ThisEditor )
	{
		assert( ThisEditor.GetData() == this );

		auto SetPropEditable = [this,&ThisEditor]( eg_cpstr Name , eg_bool bValue ) -> void
		{
			egRflEditor* VarEd = ThisEditor.GetChildPtr( Name );
			if( VarEd )
			{
				VarEd->SetEditable( bValue );
			}
		};

		SetPropEditable( "Direction" , Type == eg_light_t::Direction );
		SetPropEditable( "Position" , Type == eg_light_t::Point );
		SetPropEditable( "Range" , Type == eg_light_t::Point );
		SetPropEditable( "FallofRadius" , Type == eg_light_t::Point );
	}

	void operator = ( const EGLight& rhs )
	{
		Direction = rhs.Dir.ToVec3();
		Position = rhs.Pos.ToVec3();
		Color = eg_color32(rhs.Color);
		Range = EGMath_sqrt( rhs.GetRangeSq() );
		FallofRadius = EGMath_sqrt( rhs.GetFalloffRadiusSq() );
		Intensity = rhs.GetIntensity();
	}

	operator EGLight () const
	{
		EGLight Out( CT_Default );

		Out.Dir = eg_vec4( Direction , 0.f );
		Out.Pos = eg_vec4( Position , 1.f );
		Out.Color = eg_color(Color);
		Out.SetRangeSq( Range*Range , FallofRadius*FallofRadius );
		Out.SetIntensity( Intensity );

		return Out;
	}
};

egreflect struct eg_aabb_ed
{
	egprop eg_vec3 Min = eg_vec3(0.f,0.f,0.f);
	egprop eg_vec3 Max = eg_vec3(0.f,0.f,0.f);

	void operator = ( const eg_aabb& rhs ) { Min = rhs.Min.ToVec3(); Max = rhs.Max.ToVec3(); }
	operator eg_aabb () const { return eg_aabb( eg_vec4(Min,1.f) , eg_vec4(Max,1.f) ); }
};
