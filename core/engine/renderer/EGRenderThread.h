// (c) 2014 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGThread.h"
#include "EGEngineConfig.h"

class EGVDynamicBuffer;

class EGRenderThread: public IThreadProc
{
public:
	EGRenderThread(): m_Thread( EGThread::egInitParms( "RenderThread" , EGThread::eg_init_t::FastAsPossible ) ), m_R(nullptr), m_Driver(""){ }
	~EGRenderThread(){ }

	void Start( eg_string Driver )
	{
		m_Driver = Driver;
		m_Thread.RegisterProc( this );
		m_Thread.Start();
	}

	void Stop()
	{
		m_Thread.Stop();
		m_Thread.UnregisterProc( this );
	}

	void WaitForInitialization()
	{
		m_Thread.WaitForInitialization();
	}

	void ToolsInit( eg_uintptr_t hwnd );
	void ToolsDeinit();
	void ToolsOnWindowResized( eg_uint Width , eg_uint Height );
	void ToolsUpdate( eg_real DeltaTime );

	////////////////////////////////
	// Reinitialization (Any Thread)
	////////////////////////////////
	void ResetDisplay();
	void SetDrawSplash( eg_bool DrawSplash );

	/////////////////////////////////////
	// Device State Querying (Any Thread)
	/////////////////////////////////////
	eg_bool IsInitialized( void )const;
	eg_uint GetViewWidth()const;
	eg_uint GetViewHeight()const;
	eg_real GetAspect()const; //Width as a percentage of height, height is 1.0f.
	eg_bool IsWindowed()const;
	void    GetResolutions( EGArray<eg_ivec2>& ResOut )const;
	egRendererSpecs GetSpecs()const;

	////////////////////////
	// Drawing (Main Thread)
	////////////////////////
	class EGDisplayList* BeginFrame_MainThread();
	void EndFrame_MainThread( class EGDisplayList* DisplayList );

	///////////////////////////////
	// Asset Creation (Main Thread)
	///////////////////////////////
	egv_material CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId );
	void         ReleaseMaterial(egv_material mtrl);
	egv_vbuffer  CreateVB(eg_string_crc VertexType , eg_uint nNumVerts, const egv_vert_mesh* pVerts);
	void         ReleaseVB(egv_vbuffer buffer);
	egv_ibuffer  CreateIB(eg_uint nNumInds, const egv_index* pIndices);
	void         ReleaseIB(egv_ibuffer buffer);
	egv_rtarget  CreateRenderTarget( eg_uint Width , eg_uint Height );
	void         DestroyRenderTarget( egv_rtarget RenderTarget );
	EGVDynamicBuffer* CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize , eg_uint IndexBufferSize );
	void DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer );
	void UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer );

private:
	virtual void OnThreadMsg(eg_cpstr strParm) override;
	virtual void OnStart() override;
	virtual void Update( eg_real DeltaTime ) override;
	virtual void OnStop() override;
private:
	EGThread                   m_Thread;
	eg_string                  m_Driver;
	class EGRendererInterface* m_R;
};