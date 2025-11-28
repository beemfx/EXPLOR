// (c) 2018 Beem Media

#include "EGSettings2.h"

EGMutex EGSettingsVar::s_ReadWriteLock;

typedef EGList<EGSettingsVar> EGSettings2RegVars;
static eg_byte EGSettings2_RegVarsMem[sizeof(EGSettings2RegVars)];
static EGSettings2RegVars* EGSettings2_RegVars = nullptr;

void EGSettingsVar::GetAllSettingsMatchingFlags( EGArray<EGSettingsVar*>& Out, eg_flags MatchingFlags )
{
	if( EGSettings2_RegVars )
	{
		for( EGSettingsVar* Var : *EGSettings2_RegVars )
		{
			if( 0 != (Var->m_VarFlags & MatchingFlags) )
			{
				Out.Append( Var );
			}
		}
	}
}

EGSettingsVar* EGSettingsVar::GetSettingByName( eg_cpstr Name )
{
	if( EGSettings2_RegVars )
	{
		for( EGSettingsVar* Var : *EGSettings2_RegVars )
		{
			if( EGString_EqualsI( Name , *Var->m_VarName ) )
			{
				return Var;
			}
		}
	}
	return nullptr;
}

void EGSettingsVar::Register()
{
	if( nullptr == EGSettings2_RegVars )
	{
		EGSettings2_RegVars = new ( EGSettings2_RegVarsMem ) EGSettings2RegVars( EGSettings2RegVars::DEFAULT_ID );
	}

	EGSettings2_RegVars->Insert( this );
}

void EGSettingsVar::Unregister()
{
	assert( EGSettings2_RegVars );
	if( EGSettings2_RegVars )
	{
		EGSettings2_RegVars->Remove( this );

		if( EGSettings2_RegVars->IsEmpty() )
		{
			EGSettings2_RegVars->~EGSettings2RegVars();
			EGSettings2_RegVars = nullptr;
		}
	}
}
