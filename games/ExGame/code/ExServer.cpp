// (c) 2016 Beem Media. All Rights Reserved.

#include "ExServer.h"
#include "ExGame.h"
#include "EGEntWorld.h"
#include "ExSaveMgr.h"

EG_CLASS_DECL( ExServer )

void ExServer::Update( eg_real DeltaTime )
{
	Super::Update( DeltaTime );

	if( m_bOutOfDateSave )
	{
		m_bOutOfDateSave = false;
		if( m_EntWorld )
		{
			m_EntWorld->ResetGame();
			ExGame* AsExGame = EGCast<ExGame>(m_EntWorld->GetGame());
			if( AsExGame )
			{
				AsExGame->BeginLostGame();
			}
		}
	}
}

void ExServer::OnRemoteEvent( const eg_lockstep_id& SenderId, const egRemoteEvent& Event )
{
	Super::OnRemoteEvent( SenderId , Event );

	// Attempt to handle the event on the server...
	if( m_BhHandler.ExecuteEvent( this , Event ) )
	{
		return;
	}

	// If not pass it on to the game...
	ExGame* GameData = GetGameData();
	if( GameData )
	{
		GameData->Server_HandleEvent( SenderId, Event );
	}
}

void ExServer::BhResetGame( const eg_event_parms& Parms )
{
	unused( Parms );
	if( m_EntWorld )
	{
		m_EntWorld->ClearWorld();
	}
	SDK_ClearGlobalData();
	SDK_LoadLevel( "" );
}

void ExServer::BhCreateNewGame( const eg_event_parms& Parms )
{
	SDK_ClearGlobalData();
	ExGame* NewGameData = GetGameData();
	assert( NewGameData );
	if( NewGameData )
	{
		exPackedCreateGameInfo NewGameInfo;
		NewGameInfo.AsEventParm = Parms.as_int64();
		NewGameData->BeginNewGame( NewGameInfo );
	}
}

void ExServer::BhLoadSaveSlot( const eg_event_parms& Parms )
{
	SDK_ClearGlobalData();
	SDK_LoadLevel( *ExSaveMgr::GetSaveSlotFilename(EG_To<eg_uint>(Parms.as_int64())) , true );
}

void ExServer::OnConsoleCmd( eg_cpstr Cmd )
{
	Super::OnConsoleCmd( Cmd );

	GetGameData()->Server_OnConsoleCmd( Cmd );
}

void ExServer::SDK_LoadWorld( eg_cpstr Filename )
{
	ExGame* Game = GetGameData();
	if( Game )
	{
		Game->DebugLoadMapByWoldFilename( Filename );
	}
}

ExGame* ExServer::GetGameData()
{
	return EGCast<ExGame>( SDK_GetGame() );
}
