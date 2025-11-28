// (c) 2020 Beem Media. All Rights Reserved.

#include "ExGameSettings.h"
#include "ExGameTypes.h"

ExSettingsTextRevealSpeed ExGameSettings_DialogTextSpeed( "ExGameSettings.DialogTextSpeed" , eg_loc("SettingsDialogTextSpeed","Dialog Text Speed") , ex_reveal_speed::Fast , EGS_F_USER_SAVED|EGS_F_EDITABLE );
ExSettingsTextRevealSpeed ExGameSettings_CombatTextSpeed( "ExGameSettings.CombatTextSpeed" , eg_loc("SettingsCombatTextSpeed","Combat Text Speed") , ex_reveal_speed::SuperFast , EGS_F_USER_SAVED|EGS_F_EDITABLE );
EGSettingsBool ExGameSettings_DebugSaveAllowed( "ExGameSettings.DebugSaveAllowed" , eg_loc("SettingsDebugSaveAllowed","DEBUG: Save Anywhere") , false , EGS_F_USER_SAVED|EGS_F_DEBUG_EDITABLE );
ExSettingsMoveAnimation ExGameSettings_WalkAnimation( "ExGameSettings.WalkAnimation" , eg_loc("ExSettingsWalkAnimation","Movement Animation") , ex_move_animation::Smooth , EGS_F_USER_SAVED|EGS_F_EDITABLE );
ExSettingsMoveAnimation ExGameSettings_TurnAnimation( "ExGameSettings.TurnAnimation" , eg_loc("ExSettingsTurnAnimation","Turn Animation") , ex_move_animation::Smooth , EGS_F_USER_SAVED|EGS_F_EDITABLE );

//
// ExSettingsTextRevealSpeed
//

const ex_reveal_speed ExSettingsTextRevealSpeed::TOGGLE_ORDER[] =
{
	ex_reveal_speed::SuperFast ,
	ex_reveal_speed::Fast ,
	ex_reveal_speed::Normal ,
	ex_reveal_speed::Slow ,
};

ExSettingsTextRevealSpeed::ExSettingsTextRevealSpeed( eg_cpstr VarName , eg_string_crc DisplayName , ex_reveal_speed DefaultValue , eg_flags VarFlags )
: EGSettingsVarType<ex_reveal_speed>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void ExSettingsTextRevealSpeed::SetFromString( eg_cpstr NewValue )
{
	if( EGString_EqualsI( NewValue , "Slow" ) )
	{
		SetValue( ex_reveal_speed::Slow );
	}
	else if( EGString_EqualsI( NewValue , "Normal" ) )
	{
		SetValue( ex_reveal_speed::Normal );
	}
	else if( EGString_EqualsI( NewValue , "Fast" ) )
	{
		SetValue( ex_reveal_speed::Fast );
	}
	else if( EGString_EqualsI( NewValue , "SuperFast" ) )
	{
		SetValue( ex_reveal_speed::SuperFast );
	}
}

eg_d_string ExSettingsTextRevealSpeed::ToString() const
{
	switch( GetValue() )
	{
		case ex_reveal_speed::Slow: return "Slow";
		case ex_reveal_speed::Normal: return "Normal";
		case ex_reveal_speed::Fast: return "Fast";
		case ex_reveal_speed::SuperFast: return "SuperFast";
	}

	return "";
}

eg_loc_text ExSettingsTextRevealSpeed::ToLocText() const
{
	return EnumToLocText( GetValue() );
}

void ExSettingsTextRevealSpeed::Inc()
{
	assert( false ); // Needs drop down.
}

void ExSettingsTextRevealSpeed::Dec()
{
	assert( false ); // Needs drop down.
}

eg_int ExSettingsTextRevealSpeed::GetSelectedToggle() const
{
	const ex_reveal_speed CurrentValue = GetValue();

	for( eg_int i=0; i<countof(TOGGLE_ORDER); i++ )
	{
		if( TOGGLE_ORDER[i] == CurrentValue )
		{
			return i;
		}
	}

	return 0;
}

void ExSettingsTextRevealSpeed::SetSelectedToggle( eg_int NewValue )
{
	if( EG_IsBetween<eg_int>( NewValue , 0 , countof(TOGGLE_ORDER)-1 ) )
	{
		SetValue( TOGGLE_ORDER[NewValue] );
	}
}

void ExSettingsTextRevealSpeed::GetToggleValues( EGArray<eg_loc_text>& Out ) const
{
	for( eg_int i=0; i<countof(TOGGLE_ORDER); i++ )
	{
		Out.Append( EnumToLocText( TOGGLE_ORDER[i] ) );
	}
}

eg_loc_text ExSettingsTextRevealSpeed::EnumToLocText( ex_reveal_speed EnumValue )
{
	switch( EnumValue )
	{
		case ex_reveal_speed::Slow: return eg_loc_text(eg_loc("TextRevealSettingSlow","Slow"));
		case ex_reveal_speed::Normal: return eg_loc_text(eg_loc("TextRevealSettingNormal","Moderate"));
		case ex_reveal_speed::Fast: return eg_loc_text(eg_loc("TextRevealSettingFast","Fast"));
		case ex_reveal_speed::SuperFast: return eg_loc_text(eg_loc("TextRevealSettingSuperFast","Really Fast"));
	}

	return CT_Clear;
}


//
// ExSettingsMoveAnimation
//

const ex_move_animation ExSettingsMoveAnimation::TOGGLE_ORDER[] =
{
	ex_move_animation::Smooth ,
	ex_move_animation::HalfStep ,
	ex_move_animation::None ,
};

ExSettingsMoveAnimation::ExSettingsMoveAnimation( eg_cpstr VarName , eg_string_crc DisplayName , ex_move_animation DefaultValue , eg_flags VarFlags )
	: EGSettingsVarType<ex_move_animation>( VarName , DisplayName , DefaultValue , VarFlags )
{

}

void ExSettingsMoveAnimation::SetFromString( eg_cpstr NewValue )
{
	if( EGString_EqualsI( NewValue , "Smooth" ) )
	{
		SetValue( ex_move_animation::Smooth );
	}
	else if( EGString_EqualsI( NewValue , "HalfStep" ) )
	{
		SetValue( ex_move_animation::HalfStep );
	}
	else if( EGString_EqualsI( NewValue , "None" ) )
	{
		SetValue( ex_move_animation::None );
	}
}

eg_d_string ExSettingsMoveAnimation::ToString() const
{
	switch( GetValue() )
	{
	case ex_move_animation::Smooth: return "Smooth";
	case ex_move_animation::HalfStep: return "HalfStep";
	case ex_move_animation::None: return "None";
	}

	return "";
}

eg_loc_text ExSettingsMoveAnimation::ToLocText() const
{
	return EnumToLocText( GetValue() );
}

void ExSettingsMoveAnimation::Inc()
{
	assert( false ); // Needs drop down.
}

void ExSettingsMoveAnimation::Dec()
{
	assert( false ); // Needs drop down.
}

eg_int ExSettingsMoveAnimation::GetSelectedToggle() const
{
	const ex_move_animation CurrentValue = GetValue();

	for( eg_int i=0; i<countof(TOGGLE_ORDER); i++ )
	{
		if( TOGGLE_ORDER[i] == CurrentValue )
		{
			return i;
		}
	}

	return 0;
}

void ExSettingsMoveAnimation::SetSelectedToggle( eg_int NewValue )
{
	if( EG_IsBetween<eg_int>( NewValue , 0 , countof(TOGGLE_ORDER)-1 ) )
	{
		SetValue( TOGGLE_ORDER[NewValue] );
	}
}

void ExSettingsMoveAnimation::GetToggleValues( EGArray<eg_loc_text>& Out ) const
{
	for( eg_int i=0; i<countof(TOGGLE_ORDER); i++ )
	{
		Out.Append( EnumToLocText( TOGGLE_ORDER[i] ) );
	}
}

eg_loc_text ExSettingsMoveAnimation::EnumToLocText( ex_move_animation EnumValue )
{
	switch( EnumValue )
	{
	case ex_move_animation::Smooth: return eg_loc_text(eg_loc("MoveAnimationSettingSmooth","Smooth"));
	case ex_move_animation::HalfStep: return eg_loc_text(eg_loc("MoveAnimationSettingHalfStep","Half-Step"));
	case ex_move_animation::None: return eg_loc_text(eg_loc("MoveAnimationSettingNone","None"));
	}

	return CT_Clear;
}
