// (c) 2014 Beem Media

#include "EGRenderThread.h"
#include "EGRendererInterface.h"
#include "EGNullR_Renderer.h"
#include "EGDebugText.h"
#include "EGGlobalConfig.h"
#include "EGRenderer.h"

EG_CLASS_DECL( EGRendererInterface )

void EGRenderThread::OnThreadMsg(eg_cpstr strParm)
{
	unused( strParm );
}

void EGRenderThread::OnStart()
{
	EGClass* DriverClass = EGClass::FindClassWithBackup( m_Driver , EGNullR_Renderer::GetStaticClass() );
	m_R = EGNewObject<EGRendererInterface>( DriverClass , eg_mem_pool::System );
	if( m_R )
	{
		eg_bool bInitialized = m_R->InitRenderer( nullptr );
		if( !bInitialized )
		{
			EGMem2_SetSysFreeAllowed( true );
			m_R->Release();
			m_R = nullptr;
			EGMem2_SetSysFreeAllowed( false );
		}
	}
}

void EGRenderThread::Update( eg_real DeltaTime )
{
	if( DebugConfig_DrawFPS.GetValueThreadSafe() )
	{
		EGLogToScreen::Get().Log( this , 0 , 1.f , EGString_Format( "V. FPS: %u" , eg_uint(1.0f/DeltaTime) ) );
	}
	if( nullptr == m_R )return;
	m_R->Update_RenderThread( DeltaTime );
}

void EGRenderThread::OnStop()
{
	EG_SafeRelease( m_R );
}

eg_bool EGRenderThread::IsInitialized( void )const
{
	return nullptr != m_R;
}

void EGRenderThread::SetDrawSplash( eg_bool DrawSplash ){ m_R->SetDrawSplash( DrawSplash ); }
egRendererSpecs EGRenderThread::GetSpecs()const{ return m_R->GetSpecs(); }

void EGRenderThread::ToolsInit( eg_uintptr_t hwnd )
{
	VideoConfig_IsWindowed.SetValue( true );
	VideoConfig_VSyncEnabled.SetValue( true );
	EGRenderer_ScreenRes.SetValue( eg_ivec2(1024,1024) );
	DebugConfig_DrawFPS.SetValue( false );

	m_R = EGNewObject<EGRendererInterface>( EGClass::FindClassWithBackup( "EGD11R_Renderer" , EGNullR_Renderer::GetStaticClass() ) ,  eg_mem_pool::System );
	if( m_R )
	{
		m_R->InitRenderer( reinterpret_cast<void*>(hwnd) );
	}
}

void EGRenderThread::ToolsDeinit()
{
	EGMem2_SetSysFreeAllowed( true );
	EG_SafeRelease( m_R );
	EGMem2_SetSysFreeAllowed( false );
}

void EGRenderThread::ToolsOnWindowResized( eg_uint Width, eg_uint Height )
{
	if( m_R )
	{
		m_R->OnToolsWindowResized( Width , Height );
	}
}

void EGRenderThread::ToolsUpdate( eg_real DeltaTime )
{
	Update( DeltaTime );
}

void EGRenderThread::ResetDisplay() { return m_R ? m_R->Reset() : nullptr; }
eg_uint EGRenderThread::GetViewWidth()const{ return m_R->GetViewWidth(); }
eg_uint EGRenderThread::GetViewHeight()const{ return m_R->GetViewHeight(); }
eg_real EGRenderThread::GetAspect()const{ return m_R->GetAspectRatio(); } //Width as a percentage of height, height is 1.0f.
eg_bool EGRenderThread::IsWindowed()const{ return m_R->IsWindowed(); }
void EGRenderThread::GetResolutions(  EGArray<eg_ivec2>& ResOut ) const { return m_R->GetResolutions( ResOut ); }
class EGDisplayList* EGRenderThread::BeginFrame_MainThread(){ return m_R->BeginFrame_MainThread(); }
void EGRenderThread::EndFrame_MainThread( class EGDisplayList* DisplayList ){ return m_R->EndFrame_MainThread( DisplayList ); }
egv_material EGRenderThread::CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId ){ return m_R->CreateMaterial( pMtrlDef , SharedId ); }
void EGRenderThread::ReleaseMaterial(egv_material mtrl){ return m_R->ReleaseMaterial( mtrl ); }
egv_vbuffer  EGRenderThread::CreateVB(eg_string_crc VertexType , eg_uint nNumVerts, const egv_vert_mesh* pVerts){ return m_R->CreateVB( VertexType , nNumVerts , pVerts ); }
void EGRenderThread::ReleaseVB(egv_vbuffer buffer){ return m_R->ReleaseVB( buffer ); }
egv_ibuffer  EGRenderThread::CreateIB(eg_uint nNumInds, const egv_index* pIndices){ return m_R->CreateIB( nNumInds , pIndices ); }
void EGRenderThread::ReleaseIB( egv_ibuffer buffer ) { return m_R->ReleaseIB( buffer ); }

egv_rtarget EGRenderThread::CreateRenderTarget( eg_uint Width, eg_uint Height )
{
	return m_R->CreateRenderTarget( Width , Height );
}

void EGRenderThread::DestroyRenderTarget( egv_rtarget RenderTarget )
{
	return m_R->DestroyRenderTarget( RenderTarget );
}

EGVDynamicBuffer* EGRenderThread::CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize, eg_uint IndexBufferSize )
{
	return m_R->CreateDynamicBuffer( VertexType , VertexBufferSize , IndexBufferSize );
}

void EGRenderThread::DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	return m_R->DestroyDynamicBuffer( DynamicBuffer );
}

void EGRenderThread::UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	return m_R->UpdateDynamicBuffer( DynamicBuffer );
}
