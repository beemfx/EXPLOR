// (c) 2015 Beem Media

#pragma once

void EGWorkerThreads_Init();
void EGWorkerThreads_Deinit(); //May cause a halt while waiting for threads to shut down.
void EGWorkerThreads_Update( eg_real DeltaTime ); //For processing threads that aren't actually on a real thread.
eg_bool EGWorkerThreads_IsThisMainThread();

class EGRenderThread* EGWorkerThreads_GetRenderThread( eg_bool bDontGetToolsRenderer = false );
void EGWorkerThreads_SetToolCreatedRenderer( class EGRenderThread* ToolRenderThread );

extern class EGLoader* MainLoader;
extern class EGThreadRequest* WorkerThread;
