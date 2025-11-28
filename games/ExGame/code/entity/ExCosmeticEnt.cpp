// (c) 2020 Beem Media. All Rights Reserved.

#include "ExCosmeticEnt.h"
#include "EGClient.h"
#include "EGMenuStack.h"

EG_CLASS_DECL( ExCosmeticEnt )

void ExCosmeticEnt::OnCreate( const egEntCreateParms& Parms )
{
	Super::OnCreate( Parms );

	if( GetRole() == eg_ent_role::Authority && Parms.Reason == eg_spawn_reason::Spawned )
	{
		ParseInitString( Parms.InitString );
	}
}

void ExCosmeticEnt::OnEnterWorld()
{
	Super::OnEnterWorld();

	if( IsClient() || GetRole() == eg_ent_role::EditorPreview )
	{
		RunEvent( eg_crc("Init") );
	}

	if( IsServer() )
	{
		ReplicateDataToClient( &m_MenuOpenedEvent , sizeof(m_MenuOpenedEvent) );
		ReplicateDataToClient( &m_MenuClosedEvent , sizeof(m_MenuClosedEvent) );
	}

	if( IsClient() )
	{
		EGClient* Client = GetOwnerClient();
		EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
		if( MenuStack )
		{
			m_MenuCount = GetAudioDisablingMenus( MenuStack );
			OnMenuStackChanged( MenuStack );
			MenuStack->MenuStackChangedDelegate.AddUnique( this , &ThisClass::OnMenuStackChanged );
		}
	}
}

void ExCosmeticEnt::OnLeaveWorld()
{
	if( IsClient() )
	{
		if( ExGame* Game = GetGameData() )
		{

		}

		EGClient* Client = GetOwnerClient();
		EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
		if( MenuStack )
		{
			MenuStack->MenuStackChangedDelegate.RemoveAll( this );
		}
	}

	Super::OnLeaveWorld();
}

void ExCosmeticEnt::ParseInitString( const eg_d_string& InitString )
{
	EGParse_ProcessFnCallScript( *InitString , InitString.Len() , [this]( const egParseFuncInfo& InfoStr )->void
	{
		if( EGString_EqualsI( InfoStr.FunctionName , "SetMenuOpenedEvent" ) && InfoStr.NumParms >= 1 )
		{
			m_MenuOpenedEvent = EGCrcDb::StringToCrc( InfoStr.Parms[0] );
		}
		else if( EGString_EqualsI( InfoStr.FunctionName , "SetMenuClosedEvent" ) && InfoStr.NumParms >= 1 )
		{
			m_MenuClosedEvent = EGCrcDb::StringToCrc( InfoStr.Parms[0] );
		}
	} );
}

void ExCosmeticEnt::OnMenuStackChanged( EGMenuStack* MenuStack )
{
	const eg_int OldMenuCount = m_MenuCount;
	m_MenuCount = GetAudioDisablingMenus( MenuStack );

	if( m_MenuCount != 0 && OldMenuCount == 0 )
	{
		if( m_MenuOpenedEvent.IsNotNull() )
		{
			RunEvent( m_MenuOpenedEvent );
		}
	}

	if( m_MenuCount == 0 && OldMenuCount != 0 )
	{
		if( m_MenuClosedEvent.IsNotNull() )
		{
			RunEvent( m_MenuClosedEvent );
		}
	}
}

eg_int ExCosmeticEnt::GetAudioDisablingMenus( EGMenuStack* MenuStack )
{
	eg_int Count = 0;

	static eg_cpstr MenusToIgnore[] =
	{
		"ExConversationMenu" ,
		"ExDialog" ,
	};

	auto ShouldIgnore = []( EGMenu* Menu ) -> eg_bool
	{
		if( Menu )
		{
			for( eg_cpstr MenuId : MenusToIgnore )
			{
				if( EGString_EqualsI( Menu->GetObjectClass()->GetName() , MenuId ) )
				{
					return true;
				}
			}
		}

		return false;
	};

	if( MenuStack )
	{
		for( eg_size_t i=0; i<MenuStack->Len(); i++ )
		{
			if( EGMenu* Menu = MenuStack->GetMenuByIndex( i ) )
			{
				if( !ShouldIgnore( Menu ) )
				{
					Count++;
				}
			}

		}
	}

	return Count;
}
