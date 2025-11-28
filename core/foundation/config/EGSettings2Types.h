// (c) 2018 Beem Media

#pragma once

#include "EGSettings2.h"

class EGSettingsBool : public EGSettingsVarType<eg_bool>
{
public:

	EGSettingsBool( eg_cpstr VarName , eg_string_crc DisplayName , eg_bool DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;

	virtual eg_bool IsBoolType() const { return true; }
	virtual eg_bool GetBoolValue() const { return GetValue(); }
};

class EGSettingsInt : public EGSettingsVarType<eg_int>
{
public:

	EGSettingsInt( eg_cpstr VarName , eg_string_crc DisplayName , eg_int DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;
};

class EGSettingsReal : public EGSettingsVarType<eg_real>
{
public:

	EGSettingsReal( eg_cpstr VarName , eg_string_crc DisplayName , eg_real DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;
};

class EGSettingsString : public EGSettingsVarType<eg_s_string_sml8>
{
public:

	EGSettingsString( eg_cpstr VarName , eg_string_crc DisplayName , eg_cpstr DefaultValue , eg_flags VarFlags );

	virtual void SetFromString( eg_cpstr NewValue ) override;
	virtual eg_d_string ToString() const override;
	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;
};

class EGSettingsVolume : public EGSettingsInt
{
private:

	eg_int m_Min = 0;
	eg_int m_Max = 100;
	eg_int m_Inc = 5;

public:

	EGSettingsVolume( eg_cpstr VarName , eg_string_crc DisplayName , eg_int DefaultValue , eg_flags VarFlags );

	virtual eg_loc_text ToLocText() const override;
	virtual void Inc() override;
	virtual void Dec() override;

	virtual eg_bool CanInc() const override;
	virtual eg_bool CanDec() const override;

	eg_real GetAudioValue() const;
};

class EGSettingsClampedReal : public EGSettingsReal
{
private:

	eg_real m_Min = 0;
	eg_real m_Max = 100;
	eg_real m_Inc = 1;

public:

	EGSettingsClampedReal( eg_cpstr VarName, eg_string_crc DisplayName, eg_real DefaultValue, eg_flags VarFlags , eg_real InMin , eg_real InMax , eg_real InInc );

	virtual void Inc() override;
	virtual void Dec() override;
};

class EGSettingsClampedInt : public EGSettingsInt
{
private:

	eg_int m_Min = 0;
	eg_int m_Max = 100;
	eg_int m_Inc = 1;

public:

	EGSettingsClampedInt( eg_cpstr VarName, eg_string_crc DisplayName, eg_int DefaultValue, eg_flags VarFlags , eg_int InMin , eg_int InMax , eg_int InInc );

	virtual void Inc() override;
	virtual void Dec() override;
};