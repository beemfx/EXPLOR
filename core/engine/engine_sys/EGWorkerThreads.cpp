// (c) 2015 Beem Media

#include "EGWorkerThreads.h"
#include "EGEngine.h"
#include "EGRenderer.h"
#include "EGRenderThread.h"
#include "EGLoader.h"
#include "EGPlatform.h"
#include "EGThreadRequest.h"

EGLoader* MainLoader = nullptr;
EGThreadRequest* WorkerThread = nullptr;

class EGWorkerThread : public EGThreadRequest
{
public:

	EGWorkerThread(): EGThreadRequest( "EGWorkerThread" ){ }
	void PurgePendingRequests(){ EGThreadRequest::PurgeRequests(); }
};

static class EGWorkerThreads
{
private:
	EGThread       m_WorkThread;
	EGWorkerThread m_WorkerProc;
	EGRenderThread m_RenderThread;
	EGLoader       m_MainLoader;

public:
	EGWorkerThreads()
	: m_WorkThread( EGThread::egInitParms( "MainWorker" , EGThread::eg_init_t::LowPriority ) )
	, m_MainLoader( "MainLoader" , &m_WorkThread )
	{ 
		assert( nullptr == WorkerThread);
		assert( nullptr == MainLoader); 
		WorkerThread = &m_WorkerProc; 
		MainLoader = &m_MainLoader;
	}

	~EGWorkerThreads(){ }

	void Init()
	{
		assert( EGPlatform_IsMainThread() );
		m_WorkThread.RegisterProc( &m_WorkerProc );
		m_WorkThread.Start();
	}

	void Deinit( void )
	{
		assert( EGPlatform_IsMainThread() );
		m_WorkThread.Stop();
		m_WorkerProc.PurgePendingRequests();
		m_WorkThread.UnregisterProc( &m_WorkerProc );
	}

	void Update( eg_real DeltaTime )
	{
		unused( DeltaTime );

		m_WorkerProc.ProcessCompletedRequests();
	}


	class EGRenderThread* GetRenderThread()
	{
		return &m_RenderThread;
	}

	eg_bool IsThisMainThread()
	{
		return EGPlatform_IsMainThread();
	}
} GlobalEngineThreads;


void EGWorkerThreads_Init(){ GlobalEngineThreads.Init(); }
void EGWorkerThreads_Deinit(){ GlobalEngineThreads.Deinit(); }
void EGWorkerThreads_Update( eg_real DeltaTime ){ GlobalEngineThreads.Update( DeltaTime ); }
eg_bool EGWorkerThreads_IsThisMainThread(){ return GlobalEngineThreads.IsThisMainThread(); }


static EGRenderThread* EngineThreads_ToolRendererOverride = nullptr;

class EGRenderThread* EGWorkerThreads_GetRenderThread( eg_bool bDontGetToolsRenderer )
{ 
	if( !bDontGetToolsRenderer && EngineThreads_ToolRendererOverride )
	{
		return EngineThreads_ToolRendererOverride;
	}
	return GlobalEngineThreads.GetRenderThread(); 
}

void EGWorkerThreads_SetToolCreatedRenderer( class EGRenderThread* ToolRenderThread )
{
	EngineThreads_ToolRendererOverride = ToolRenderThread;
}
