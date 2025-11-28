/******************************************************************************
ServerInterface - The globally accessible server.
(c) 2015 Beem Software
******************************************************************************/
#include "EGServerInterface.h"
#include "EGThread.h"
#include "EGServer.h"

class EGServerThread : private IThreadProc
{
public:

	void Init( EGServer* Server , EGThread* OwnerThread )
	{
		m_OwnerThread = OwnerThread;
		m_Server = Server;
		m_OwnerThread->RegisterProc( this );
	}

	void Deinit()
	{
		m_OwnerThread->UnregisterProc( this );
	}

	void QueMsg( eg_cpstr Msg ){ m_Server->QueMsg( Msg ); }

private:

	EGThread* m_OwnerThread = nullptr;
	EGServer* m_Server = nullptr;

private:

	virtual void OnStart() override{ }
	virtual void Update( eg_real DeltaTime ) override{ m_Server->Update( DeltaTime ); }
	virtual void OnStop() override{ }
	virtual void OnThreadMsg( eg_cpstr sMsg ) override{ m_Server->QueMsg( sMsg ); }
};

static struct egGlobalServerData
{
	EGServer*      Server;
	EGThread       ServerThread;
	EGServerThread ServerThreadProc;
	SERVER_START_T StartType;
	eg_bool        Inited:1;
	eg_bool        IsStarted:1;

	egGlobalServerData()
	: StartType( SERVER_START_THREAD )
	, Inited(false)
	, IsStarted(false)
	, ServerThread( EGThread::egInitParms( "ServerThread" , EGThread::eg_init_t::FastAsPossible ) )
	{ 
	
	}

} GlobalServer_Data;


void Server_Init(class EGClass* ServerClass , class EGClass* GameClass)
{
	if( !GlobalServer_Data.Inited )
	{ 
		GlobalServer_Data.Inited = true;
		GlobalServer_Data.Server = EGNewObject<EGServer>( ServerClass , eg_mem_pool::System );
		GlobalServer_Data.Server->InitGameClass( GameClass );
		GlobalServer_Data.ServerThreadProc.Init( GlobalServer_Data.Server , &GlobalServer_Data.ServerThread );
	}
	else
	{
		assert( false ); //Should call Server_Init only once!
	}
}
void Server_Deinit()
{
	if( GlobalServer_Data.Inited )
	{
		Server_Stop();

		GlobalServer_Data.ServerThreadProc.Deinit();
		EGDeleteObject( GlobalServer_Data.Server );
		GlobalServer_Data.Server = nullptr;
		GlobalServer_Data.Inited = false;
	}
	else
	{
		assert( false ); //Called Server_Deinit without call to Init.
	}
}

void Server_Start( SERVER_START_T StartType )
{
	if( !GlobalServer_Data.Inited ){ assert( false ); return; } //Can't start server wihtout initializing it.

	//If the server is already started we will effectively restart it.
	if( GlobalServer_Data.IsStarted )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ " called on a running server, restarting." );
		Server_Stop();
	}

	GlobalServer_Data.StartType = StartType;

	GlobalServer_Data.Server->Init();

	switch( StartType )
	{
		case SERVER_START_THIS_THREAD:
			break;
		case SERVER_START_THREAD:
			GlobalServer_Data.ServerThread.Start();
			break;
	}
	GlobalServer_Data.IsStarted = true;
}

void Server_Stop()
{ 
	if( !GlobalServer_Data.Inited ){ assert( false ); return; } //Can't start server wihtout initializing it.

	if( !GlobalServer_Data.IsStarted )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ ": Already stopped." );
		return;
	}

	switch( GlobalServer_Data.StartType )
	{
		case SERVER_START_THIS_THREAD:
			break;
		case SERVER_START_THREAD:
			GlobalServer_Data.ServerThread.Stop();
			break;
	}

	GlobalServer_Data.Server->Deinit();

	GlobalServer_Data.IsStarted = false;
}

void Server_UpdatePart1( eg_real DeltaTime )
{ 
	if( GlobalServer_Data.IsStarted && GlobalServer_Data.StartType == SERVER_START_THIS_THREAD )
	{
		GlobalServer_Data.Server->Update( DeltaTime );
	}
}

void Server_UpdatePart2( eg_real DeltaTime )
{
	unused( DeltaTime );
	if( GlobalServer_Data.IsStarted && GlobalServer_Data.StartType == SERVER_START_THIS_THREAD )
	{
		//In the future we may want to do a 2nd update so that the game can do something else while the physics simulation is running.
	}
}

eg_bool Server_IsRunning(){ return GlobalServer_Data.IsStarted; }
void Server_PostMsg( eg_cpstr StrMsg )
{ 
	if( !GlobalServer_Data.IsStarted ){ assert( false ); return; } //Can't post a server message to a non-running server.

	GlobalServer_Data.Server->QueMsg( StrMsg );
}