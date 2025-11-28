// (c) 2018 Beem Media

#pragma once

#include "EGDelegate.h"
#include "EGLocText.h"

EG_DECLARE_FLAG( EGS_F_USER_SAVED     , 1 );
EG_DECLARE_FLAG( EGS_F_SYS_SAVED      , 2 );
EG_DECLARE_FLAG( EGS_F_EDITABLE       , 3 );
EG_DECLARE_FLAG( EGS_F_DEBUG_EDITABLE , 4 );
EG_DECLARE_FLAG( EGS_F_NEEDSVRESTART  , 5 );

typedef eg_s_string_base<eg_char8,48> eg_setting_var_name;

class EGSettingsVar : public IListable
{
public:

	EGSimpleMCDelegate OnChangedDelegate;

protected:

	const eg_setting_var_name m_VarName;
	const eg_string_crc       m_DisplayName;
	const eg_flags            m_VarFlags;

protected:

	static EGMutex s_ReadWriteLock;

protected:

	EGSettingsVar() = delete;
	EGSettingsVar( const EGSettingsVar& rhs ) = delete;
	const EGSettingsVar& operator = ( const EGSettingsVar& rhs ) = delete;

	EGSettingsVar( eg_cpstr VarName , eg_string_crc DisplayName , eg_flags VarFlags )
	: m_VarName( VarName )
	, m_DisplayName( DisplayName )
	, m_VarFlags( VarFlags )
	{
		Register();
	}

	~EGSettingsVar()
	{
		assert( !OnChangedDelegate.HasListeners() ); // All listeners must be cleared since this happens during global destruction so the array should have been cleared.
		Unregister();
	}

public:

	eg_cpstr GetVarName() const { return *m_VarName; }
	eg_string_crc GetDisplayName() const { return m_DisplayName; }
	const eg_flags& GetFlags() const { return m_VarFlags; }

	virtual void SetFromString( eg_cpstr NewValue ) = 0;
	virtual eg_d_string ToString() const = 0;
	virtual eg_loc_text ToLocText() const = 0;
	virtual void Inc() = 0;
	virtual void Dec() = 0;
	virtual eg_bool CanInc() const { return true; }
	virtual eg_bool CanDec() const { return true; }

	virtual eg_bool IsBoolType() const { return false; }
	virtual eg_bool GetBoolValue() const { return false; }

	virtual eg_bool IsToggleType() const { return false; }
	virtual eg_int GetSelectedToggle() const { return 0; }
	virtual void SetSelectedToggle( eg_int NewValue ) { unused( NewValue ); }
	virtual void GetToggleValues( EGArray<eg_loc_text>& Out ) const { unused( Out ); }

	static void GetAllSettingsMatchingFlags( EGArray<EGSettingsVar*>& Out , eg_flags MatchingFlags );
	static EGSettingsVar* GetSettingByName( eg_cpstr Name );
	eg_bool IsEditable() const { return m_VarFlags.IsSet( EGS_F_EDITABLE ); }
	eg_bool IsDebugEditable() const { return m_VarFlags.IsSet( EGS_F_DEBUG_EDITABLE ); }

private:

	void Register();
	void Unregister();
};

template<typename VType>
class EGSettingsVarType : public EGSettingsVar
{
protected:

	VType m_Value;
	
public:

	EGSettingsVarType() = delete;
	EGSettingsVarType( const EGSettingsVar& rhs ) = delete;

	EGSettingsVarType( eg_cpstr VarName , eg_string_crc DisplayName , const VType& DefaultValue , eg_flags VarFlags )
	: EGSettingsVar( VarName , DisplayName , VarFlags )
	, m_Value( DefaultValue )
	{
	}

	const VType GetValue() const { return m_Value; }
	const VType GetValueThreadSafe() const { EGFunctionLock Lock( &s_ReadWriteLock ); return m_Value; }
	void SetValue( const VType& NewValue ) { EGFunctionLock Lock( &s_ReadWriteLock ); m_Value = NewValue; OnChangedDelegate.Broadcast(); }
};
