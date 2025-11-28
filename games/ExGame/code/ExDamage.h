// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"

struct exDamage
{
	ex_attr_value Physical;
	ex_attr_value Fire;
	ex_attr_value Water;
	ex_attr_value Earth;
	ex_attr_value Air;

	exDamage()
		: exDamage( CT_Preserve )
	{

	}

	exDamage( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			Physical = 0;
			Fire = 0;
			Water = 0;
			Earth = 0;
			Air = 0;
		}
	}

	ex_attr_value operator- ( const exDamage& rhs ) const
	{
		ex_attr_value PhysicalHit = EG_Max<ex_attr_value>( Physical - rhs.Physical, 0 );
		ex_attr_value FireHit = EG_Max<ex_attr_value>( Fire - rhs.Fire, 0 );
		ex_attr_value WaterHit = EG_Max<ex_attr_value>( Water - rhs.Water, 0 );
		ex_attr_value EarthHit = EG_Max<ex_attr_value>( Earth - rhs.Earth, 0 );
		ex_attr_value AirHit = EG_Max<ex_attr_value>( Air - rhs.Air, 0 );

		ex_attr_value TotalHit = PhysicalHit + FireHit + WaterHit + EarthHit + AirHit;
		return TotalHit;
	}

	ex_attr_value GetRawTotal() const
	{
		ex_attr_value Total = Physical + Fire + Water + Earth + Air;
		return Total;
	}

	exDamage GetMagicMultiplyLevelGrowth( ex_attr_value Magic , ex_attr_value Level ) const
	{
		exDamage Out( CT_Clear );

		Out.Physical = ExCore_GetRampedValue_Level( Magic*Physical , Level );
		Out.Fire     = ExCore_GetRampedValue_Level( Magic*Fire , Level );
		Out.Water    = ExCore_GetRampedValue_Level( Magic*Water , Level );
		Out.Earth    = ExCore_GetRampedValue_Level( Magic*Earth , Level );
		Out.Air      = ExCore_GetRampedValue_Level( Magic*Air , Level );

		return Out;
	}

	exDamage GetMagicMultiplied( ex_attr_value Magic ) const
	{
		exDamage Out( CT_Clear );

		Out.Physical = Magic*Physical;
		Out.Fire     = Magic*Fire;
		Out.Water    = Magic*Water;
		Out.Earth    = Magic*Earth;
		Out.Air      = Magic*Air;

		return Out;
	}

	ex_attr_t GetResistType() const
	{
		// Basically if one resist type is higher than the others we treat it
		// as the resistance, but we ignore PDEF since this is only for elemental
		// resistance.
		if( Fire > Water && Fire > Earth && Fire > Air )
		{
			return ex_attr_t::FDEF;
		}

		if( Water > Fire && Water > Earth && Water > Air )
		{
			return ex_attr_t::WDEF;
		}

		if( Earth > Water && Earth > Fire && Earth > Air )
		{
			return ex_attr_t::EDEF;
		}

		if( Air > Water && Air > Earth && Air > Fire )
		{
			return ex_attr_t::ADEF;
		}
		
		return ex_attr_t::DEF_;
	}

	ex_attr_value GetAttrValue( ex_attr_t Type )
	{
		switch( Type )
		{
			case ex_attr_t::PDMG:
			case ex_attr_t::PDEF:
				return Physical;
			case ex_attr_t::FDMG:
			case ex_attr_t::FDEF:
				return Fire;
			case ex_attr_t::WDMG:
			case ex_attr_t::WDEF:
				return Water;
			case ex_attr_t::EDMG:
			case ex_attr_t::EDEF:
				return Earth;
			case ex_attr_t::ADMG:
			case ex_attr_t::ADEF:
				return Air;
			default:
				break;
		}

		return GetRawTotal();
	}

	static const ex_attr_t GetComplimentaryDmgOrDef( ex_attr_t Type )
	{
		switch( Type )
		{	
			case ex_attr_t::DMG_: return ex_attr_t::DEF_;
			case ex_attr_t::DEF_: return ex_attr_t::DMG_;
			case ex_attr_t::PDMG: return ex_attr_t::PDEF;
			case ex_attr_t::PDEF: return ex_attr_t::PDMG;
			case ex_attr_t::FDMG: return ex_attr_t::FDEF;
			case ex_attr_t::FDEF: return ex_attr_t::FDMG;
			case ex_attr_t::WDMG: return ex_attr_t::WDEF;
			case ex_attr_t::WDEF: return ex_attr_t::WDMG;
			case ex_attr_t::EDMG: return ex_attr_t::EDEF;
			case ex_attr_t::EDEF: return ex_attr_t::EDMG;
			case ex_attr_t::ADMG: return ex_attr_t::ADEF;
			case ex_attr_t::ADEF: return ex_attr_t::ADMG;
			default:
				break;
		}

		return ex_attr_t::DEF_;
	}
};
