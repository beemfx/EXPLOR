#pragma once

#include "EGServer.h"
#include "EGRemoteEvent.h"

class ExGame;

class ExServer : public EGServer
{
	EG_CLASS_BODY( ExServer , EGServer )

private:

	EGRemoteEventHandler<ExServer> m_BhHandler;
	eg_bool                        m_bOutOfDateSave;

public:

	ExServer()
	: EGServer()
	{
		m_BhHandler.RegisterEvent( eg_crc("ResetGame") , &ExServer::BhResetGame );
		m_BhHandler.RegisterEvent( eg_crc("CreateNewGame") , &ExServer::BhCreateNewGame );
		m_BhHandler.RegisterEvent( eg_crc("LoadSaveSlot") , &ExServer::BhLoadSaveSlot );
	}

	virtual void Update( eg_real DeltaTime ) override final;
	virtual void OnRemoteEvent( const eg_lockstep_id& SenderId, const egRemoteEvent& Event ) override final;
	void BhResetGame( const eg_event_parms& Parms );
	void BhCreateNewGame( const eg_event_parms& Parms );
	void BhLoadSaveSlot( const eg_event_parms& Parms );
	virtual void OnConsoleCmd( eg_cpstr Cmd ) override final;
	virtual void SDK_LoadWorld( eg_cpstr Filename ) override final;

	void OnSaveIsOutOfDate(){ m_bOutOfDateSave = true; }

	ExGame* GetGameData();
};