// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"
#include "EGSettings2Types.h"

class ExSettingsTextRevealSpeed : public EGSettingsVarType<ex_reveal_speed>
{
private:

	static const ex_reveal_speed TOGGLE_ORDER[];

public:

	ExSettingsTextRevealSpeed( eg_cpstr VarName , eg_string_crc DisplayName , ex_reveal_speed DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;

	virtual eg_bool IsToggleType() const override { return true; }
	virtual eg_int GetSelectedToggle() const override;
	virtual void SetSelectedToggle( eg_int NewValue ) override;
	virtual void GetToggleValues( EGArray<eg_loc_text>& Out ) const override;

	static eg_loc_text EnumToLocText( ex_reveal_speed EnumValue );
};

class ExSettingsMoveAnimation : public EGSettingsVarType<ex_move_animation>
{
private:

	static const ex_move_animation TOGGLE_ORDER[];

public:

	ExSettingsMoveAnimation( eg_cpstr VarName , eg_string_crc DisplayName , ex_move_animation DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;

	virtual eg_bool IsToggleType() const override { return true; }
	virtual eg_int GetSelectedToggle() const override;
	virtual void SetSelectedToggle( eg_int NewValue ) override;
	virtual void GetToggleValues( EGArray<eg_loc_text>& Out ) const override;

	static eg_loc_text EnumToLocText( ex_move_animation EnumValue );
};

extern ExSettingsTextRevealSpeed ExGameSettings_DialogTextSpeed;
extern ExSettingsTextRevealSpeed ExGameSettings_CombatTextSpeed;
extern EGSettingsBool ExGameSettings_DebugSaveAllowed;
extern ExSettingsMoveAnimation ExGameSettings_WalkAnimation;
extern ExSettingsMoveAnimation ExGameSettings_TurnAnimation;
