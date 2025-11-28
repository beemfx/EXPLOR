/******************************************************************************
(c) 2011 Beem Software
******************************************************************************/
#pragma comment( lib , "d3d11.lib" )
#pragma comment( lib , "dxgi.lib" )

#include "EGD11R_Renderer.h"
#include "EGD11R_MtrlMgr2.h"
#include "EGD11R_Base.h"
#include "EGD11R_BufferMgr.h"
#include "EGLoader.h"
#include "EGStringMap.h"
#include "EGFileData.h"
#include "EGAlwaysLoaded.h"
#include "EGD11R_RenderTarget.h"
#include "EGVDynamicBuffer.h"
#include "EGGlobalConfig.h"
#include "EGRenderer.h"
#include "EGEngine.h"

EG_CLASS_DECL( EGD11R_Renderer )

extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }
extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }

static EGSettingsBool EGD11R_Renderer_DebugPickAdapter( "EGD11R_Renderer.DebugPickAdapter" , CT_Clear , false , EGS_F_SYS_SAVED );
static EGSettingsReal EGD11R_Renderer_MotionBlurSkipDistance( "EGD11R_Renderer.MotionBlurSkipDistance" , CT_Clear , 5.f , 0 );
static EGSettingsString EGD11R_Renderer_Adapter( "EGD11R_Renderer.Adapter" , CT_Clear , "" , EGS_F_SYS_SAVED );


EGD11R_Renderer::EGD11R_Renderer()
: EGBaseR_Core()
, m_Device(nullptr)
, m_Context(nullptr)
, m_SwapChain(nullptr)
, m_fAspect(1.0f)
, m_nInternalID(1)
, m_bClipPlaneOn(false)
, m_pFXCur()
, m_pFXState()
, m_hwnd(nullptr)
, m_nVBs(0)
, m_nIBs(0)
, m_TextureFilter(FILTER_ANISOTROPIC)
, m_MtrlMgr( &m_ShaderMgr )
, m_ResetRequested(false)
, m_RtCur(FSRT_COUNT)
, m_DsCur(nullptr)
, m_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
, m_PsConstsBuffer(nullptr)
, m_bManualDrawZ( false )
, m_MainSceneRT(FSRT_SCREEN)
, m_bIsChildWnd(false)
{
	zero(&m_RTs);
	zero(&m_DefShaders);
	zero(&m_ip);
	zero(&m_RasterizerStates);
	zero(&m_SamplerStates);
	zero(&m_BlendStates);
	zero(&m_DepthStencilStates);
	zero(&m_DepthStencilRef);
	zero(&m_VsConstsBuffers);
}

eg_bool EGD11R_Renderer::InitRenderer( void* Context )
{
	m_DataLock.Lock();
	m_InitStatus = INIT_IN_PROGRESS;
	Init( reinterpret_cast<HWND>(Context) );
	m_DataLock.Unlock();

	return IsInitialized();
}

EGD11R_Renderer::~EGD11R_Renderer()
{
	Deinit();	
}

void EGD11R_Renderer::DrawSplash( void )
{
	m_CurSamplerState.Push( eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT );
	m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	SetMaterial( m_DefShaders[ISHADER_SPLASH] );
	SetViewTF( eg_mat::I );
	SetWorldTF( eg_mat::I );
	eg_mat ProjMat;
	static const eg_real SPLASH_SCALE=1.05f;
	// The splash texture should have been created 16x9
	ProjMat = eg_mat::I;
	ProjMat.r11 = SPLASH_SCALE*(1.f/m_fAspect)*(16.f/9.f);
	ProjMat.r22 = SPLASH_SCALE;
	SetProjTF( ProjMat );
	DrawBasicQuad();

	m_CurDepthStencilState.Pop();
	m_CurSamplerState.Pop();
}

void EGD11R_Renderer::Update_RenderThread( eg_real DeltaTime )
{
	Window_Update( DeltaTime );

	const eg_bool bNeedsResetSplash = m_ResetTimer > 0.f;
	if( bNeedsResetSplash )
	{
		m_ResetTimer -= DeltaTime;
	}

	{
		EGFunctionLock Lock( &m_DlLock );
		if( m_DlDraw )
		{
			m_DlDraw->List.DeinitDisplayList();
			m_DlAvailable.Push( m_DlDraw );
		}

		m_DlDraw = m_DlNext;
		m_DlNext = nullptr;
	}

	if( !m_Context || !m_Device || !m_SwapChain )
		return;
		
	Update_RefreshDynamicBuffers( DeltaTime );

	//Drawing:
	{
		EGFunctionLock Lock( &m_AssetCreationLock ); //No assets may be created while drawing.

		m_LastLayoutType = CT_Clear; // Reset vertex layout type.

		if( m_BufferMgr )
		{
			m_BufferMgr->PreFrameUpdate();
		}

		ProcessDeleteQue( false , nullptr != m_DlDraw ? m_DlDraw->List.GetAssetState() : 0 ); 

		m_MtrlMgr.Update_RenderThread();

		//Process Loaded items:
		MainLoader->ProcessCompletedLoads( EGLoader::LOAD_THREAD_RENDER );

		eg_bool IgnoreDl = false;

		if( !m_DlDraw )
		{
			EGLogf( eg_log_t::RendererActivity , "No frame to draw, frame dropped." );
			IgnoreDl = true;
		}

		if( !IgnoreDl && m_DlDraw->List.IsOutOfMem() )
		{
			EGLogf( eg_log_t::RendererActivity , "Display list out of memory, frame dropped." );
			IgnoreDl = true;
		}

		eg_bool ShouldDrawSplash = false;
		{
			EGFunctionLock Lock( &m_DataLock );
			ShouldDrawSplash = m_DrawSplash || bNeedsResetSplash;
		}

		if( IgnoreDl && !ShouldDrawSplash )return;

		if( m_ResetRequested && m_bAppIsActive )
		{
			Reset_Internal();
			m_ResetRequested = false;
		}
	
		m_bIsDrawing = true;
		m_bHasDepth = false;

		ResetScreenViewport();
		SetRT( FSRT_SCREEN );
		zero( &m_GlobalSamplers );
		m_bGlobalSamplersDirty = true;

		const eg_bool bUseBlackClear = false;
		if( ShouldDrawSplash || bUseBlackClear )
		{
			FLOAT Color[] = { 0.f , 0.f, 0.f, 1.0f };
			m_Context->ClearRenderTargetView( m_RTs[m_RtCur]->GetRenderTargetView() , Color );
			m_Context->ClearDepthStencilView( m_DsCur->GetDepthStencilView() , D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL , 1.0f , 0 );
		}
		else
		{
			FLOAT Color[] = { 0.25f , 0.25f, 1.0f, 1.0f };
			m_Context->ClearRenderTargetView( m_RTs[m_RtCur]->GetRenderTargetView() , Color );
			m_Context->ClearDepthStencilView( m_DsCur->GetDepthStencilView() , D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL , 1.0f , 0 );
		}

		// Set input layout and vertex buffers: (Can change for special draws, but should always be restored to this.)
		{
			SetVertexLayout( eg_crc("egv_vert_mesh") );
		}

		assert( m_CurRasterizerState.IsEmpty() );
		assert( m_CurSamplerState.IsEmpty() );
		assert( m_CurBlendState.IsEmpty() );
		assert( m_CurDepthStencilState.IsEmpty() );
		assert( m_CurDefaultShader.IsEmpty() );

		PushDefaultRenderStates();

		if( ShouldDrawSplash ) //If we aren't ready to actually draw yet, draw the splash screen.
		{
			if( !bNeedsResetSplash )
			{
				DrawSplash();
			}
		}
		else if( !IgnoreDl )
		{
			ProcessDisplayList( &m_DlDraw->List );	
		}

		{
			ID3D11RenderTargetView* RtView[] = { m_BackBuffer->GetRenderTargetView() };
			m_Context->OMSetRenderTargets( countof(RtView) , RtView , nullptr );
		
			m_CurSamplerState.Push( eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT );
			m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );

			SetViewTF( eg_mat::I );
			SetWorldTF( eg_mat::I );
			SetProjTF( eg_mat::I );
			SetMaterial( m_DefShaders[ISHADER_COPY] );
			SetGlobalSamplers( FSRT_SCREEN , FSRT_COUNT , FSRT_COUNT );
			ApplyScreenViewport();
			DrawBasicQuad();

			m_CurDepthStencilState.Pop();
			m_CurSamplerState.Pop();
		}

		PopDefaultRenderStates();

		m_bIsDrawing = false;
	}

	HRESULT Res = m_SwapChain ? m_SwapChain->Present( m_ip.VSync ? 1 : 0 ,0) : E_FAIL;
	assert( SUCCEEDED(Res) );
	m_Context->ClearState();
}

void EGD11R_Renderer::OnToolsWindowResized( eg_uint Width, eg_uint Height )
{
	assert( !m_bIsDrawing );
	if( !m_bIsDrawing )
	{
		if( m_SwapChain )
		{
			//DXGI_MODE_DESC TargDesc;
			//zero( &TargDesc );
			//Init_Device_AdjustRes( &TargDesc );
			//TargDesc.Width = Width;
			//TargDesc.Height = Height;
			//TargDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
			//
			//m_SwapChain->ResizeTarget( &TargDesc );
			//
			//m_Context->OMSetRenderTargets( 0 , nullptr , nullptr );
			m_ip.ScreenWidth = Width;
			m_ip.ScreenHeight = Height;
			m_ip.BufferWidth = Width;
			m_ip.BufferHeight = Height;
			Init_RenderTargets( false );
			m_SwapChain->ResizeBuffers( 2 , Width, Height , D3D_BB_FMT_A , 0 );
			Init_RenderTargets( true );

		}
	}
}

void EGD11R_Renderer::ProcessDisplayList( class EGDisplayList* DisplayList )
{
	for( const EGDisplayList::egCmdListItem* Cmd : DisplayList->List )
	{
		const egDisplayListCmdData* Data = Cmd->Data;
		switch( Cmd->Cmd )
		{
			case DLFUNC_SetGlobalSamplers: SetGlobalSamplers( Data->SetGlobalSamplers.Sampler0 , Data->SetGlobalSamplers.Sampler1 , Data->SetGlobalSamplers.Sampler2 ); break;
			case DLFUNC_ResolveToRenderTarget: ResolveToRenderTarget( Data->ResolveToRenderTarget.RenderTarget ); break;
			case DLFUNC_DrawDebugZOverlay: DrawDebugZOverlay( Data->DrawDebugZOverlay.Near , Data->DrawDebugZOverlay.Far , Data->DrawDebugZOverlay.FudgeFactor ); break;
			case DLFUNC_SetScreenViewport: SetScreenViewport( Data->SetScreenViewport.Vp ); break;
			case DLFUNC_SetClipPlane: SetClipPlane( &Data->SetClipPlane.pplnClipW );break;
			case DLFUNC_ClearClipPlane: SetClipPlane( nullptr ); break;
			case DLFUNC_ClearRT:	ClearRT( Data->ClearRT.Color ); break;
			case DLFUNC_ClearDS: ClearDS( Data->ClearDS.Depth , Data->ClearDS.Stencil ); break;
			case DLFUNC_DrawBasicQuad: DrawBasicQuad(); break;
			case DLFUNC_DrawBasicQuadColored: DrawBasicQuad( Data->DrawBasicQuadColored.Color ); break;
			case DLFUNC_DrawAABB:
			{
				const eg_aabb& Bounds = Data->DrawAABB.Box;
				const eg_color& Color = Data->DrawAABB.Color;
				DrawAABB( &Bounds , eg_color32(Color) );
			} break;
			case DLFUNC_DrawLine:
			{
				const eg_vec4& Start = Data->DrawLine.Start;
				const eg_vec4& End = Data->DrawLine.End;
				const eg_color& Color = Data->DrawLine.Color;
				DrawLine( &Start , &End , eg_color32(Color) );
			} break;
			case DLFUNC_DrawRawTris: DrawRawTris( Data->DrawRawTris.pVerts , Data->DrawRawTris.nNumTris ); break;
			case DLFUNC_DrawTris: DrawTris( Data->DrawTris.VertexBuffer , Data->DrawTris.nFirstV , Data->DrawTris.nNumTris ); break;
			case DLFUNC_DrawIndexedTris: 
				DrawIndexedTris( 
					Data->DrawIndexedTris.VertexBuffer , 
					Data->DrawIndexedTris.IndexBuffer , 
					Data->DrawIndexedTris.nFirstI , 
					Data->DrawIndexedTris.nNumVerts, 
					Data->DrawIndexedTris.nNumTris ); 
				break;
			case DLFUNC_DrawShadowVolume: DrawShadowVolume( Data->DrawShadowVolume.Color ); break;
			case DLFUNC_ResetScreenViewport: ResetScreenViewport(); break;
			case DLFUNC_BeginMainScene: BeginMainScene( Data->BeginMainScene.Flags ); break;
			case DLFUNC_EndMainScene: EndMainScene( Data->EndMainScene.Flags ); break;
			case DLFUNC_DoSceneProc: DoSceneProc( Data->DoSceneProc.Flags ); break;
			case DLFUNC_RunFullScreenProc: RunFullScreenProc( Data->RunFullScreenProc.mtrl ); break;
			default: 
				HandleCommonDisplayListCmd( Cmd ); 
				break;
		}
	}
}


void EGD11R_Renderer::Init(HWND hwndParent)
{
	if( nullptr == hwndParent )
	{
		Window_Init();
		if( !m_hwnd )return;
	}
	else
	{
		m_bIsChildWnd = true;
		m_hwnd = hwndParent;
	}

	//If the IDirect3D interface has already been initialized, then
	//Init has already been called and should not be called a second time.
	if( m_Device || m_Context || m_SwapChain )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ "Error: The system was already initialized." );	
		assert(false);
		return;
	}

	// Motion blur skip distance (we square it for faster computations later on)
	m_MotionBlurSkipDistSq = EGD11R_Renderer_MotionBlurSkipDistance.GetValueThreadSafe();
	m_MotionBlurSkipDistSq *= m_MotionBlurSkipDistSq;
	
	Init_Device();

	if( m_Device && m_Context && m_SwapChain )
	{
		Init_Mgrs();
		
		Validate();

		//Create constant buffers.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth           = 0;
		cbDesc.Usage               = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags           = 0;
		cbDesc.StructureByteStride = 0;

		HRESULT Res;

		static struct
		{
			VSB_T     Type;
			eg_uint   Size;
		}
		VsBufferCrTable[] =
		{
			{ VSB_T::BASE  , egRVSConsts::SZ_BASE  },
			{ VSB_T::BONES , egRVSConsts::SZ_BONES },
			{ VSB_T::IBONE , sizeof(eg_mat) },
		};

		static_assert( countof(VsBufferCrTable) == static_cast<eg_size_t>(VSB_T::COUNT) , "Missing types in table above?" );

		for( eg_uint i=0; i<countof(VsBufferCrTable); i++ )
		{
			cbDesc.ByteWidth = VsBufferCrTable[i].Size;
			Res = m_Device->CreateBuffer( &cbDesc , NULL , &m_VsConstsBuffers[static_cast<eg_size_t>(VsBufferCrTable[i].Type)] );
			assert( SUCCEEDED( Res ) );
		}

		cbDesc.ByteWidth = sizeof( m_PSC );
		Res = m_Device->CreateBuffer( &cbDesc , NULL , &m_PsConstsBuffer );
		assert( SUCCEEDED( Res ) );

		Init_RS(true);
		Init_Shaders(true);
	}
	else
	{
		Window_Deinit();
	}

	m_DataLock.Lock();
	m_InitStatus = ( m_Device && m_Context && m_SwapChain ) ? INIT_SUCCEEDED : INIT_FAILED;
	m_DataLock.Unlock();
}

void EGD11R_Renderer::SetRT(RENDERER_FSRT_TYPE Target, eg_bool bNoDs/*=false */)
{
	assert(0<=Target && Target < FSRT_COUNT);
	m_RtCur = Target;
	m_DsCur = bNoDs ? nullptr : m_DepthBuffer;
	ID3D11RenderTargetView* RtView[] = { m_RTs[m_RtCur]->GetRenderTargetView() };
	m_Context->OMSetRenderTargets( countof(RtView) , RtView , m_DsCur ? m_DsCur->GetDepthStencilView() : nullptr );
	
	if( Target == EGBaseR_Core::FSRT_SCREEN )
	{
		ApplyScreenViewport();
	}
	else
	{
		SetViewportToRT();
	}
}
 
void EGD11R_Renderer::ResolveToRenderTarget( egv_rtarget RenderTarget )
{
	eg_mat SavedP = m_mP;
	eg_mat SavedV = m_mV;
	eg_mat SavedW = m_mW;

	SetProjTF( eg_mat::I );
	SetViewTF( eg_mat::I );
	SetWorldTF( eg_mat::I );

	EGD11R_RenderTarget* Rt = nullptr;
	eg_sampler_s SamplerState = eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT;
	eg_defaultshader_t ShaderType = eg_defaultshader_t::TEXTURE;
	
	switch( RenderTarget )
	{
		case egv_rtarget::DepthBuffer:
			Rt = m_RTs[FSRT_Z];
			SamplerState = eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT;
			ShaderType = eg_defaultshader_t::STORE_DEPTH;
			break;
		default:
			Rt = reinterpret_cast<EGD11R_RenderTarget*>(static_cast<eg_uintptr_t>( RenderTarget ) );
			break;
	}
	if( Rt == nullptr )
	{
		return;
	}

	assert( Rt->IsA( &EGD11R_RenderTarget::GetStaticClass() ) );

	m_CurBlendState.Push( eg_blend_s::BLEND_NONE_COLOR_ALL );
	m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	m_CurDefaultShader.Push( ShaderType );
	m_CurSamplerState.Push( SamplerState );

	ID3D11RenderTargetView* RtView[] = { Rt->GetRenderTargetView() };
	m_Context->OMSetRenderTargets( countof(RtView) , RtView , nullptr );

	D3D11_VIEWPORT Vp;
	Vp.TopLeftX = 0.f;
	Vp.Width    = static_cast<eg_real>(Rt->GetWidth());
	Vp.TopLeftY = 0.f;
	Vp.Height   = static_cast<eg_real>(Rt->GetHeight());
	Vp.MinDepth = 0.f;
	Vp.MaxDepth = 1.f;
	m_Context->RSSetViewports( 1 , &Vp );

	if( RenderTarget == egv_rtarget::DepthBuffer )
	{
		SetMaterial( m_DefShaders[ISHADER_STOREDEPTH] );
	}
	else
	{
		SetMaterial( m_DefShaders[ISHADER_COPY] );
		SetGlobalSamplers( m_RtCur , FSRT_COUNT , FSRT_COUNT );
	}

	DrawBasicQuad();

	m_CurDefaultShader.Pop();
	m_CurDepthStencilState.Pop();
	m_CurBlendState.Pop();
	m_CurSamplerState.Pop();

	SetProjTF( SavedP );
	SetViewTF( SavedV );
	SetWorldTF( SavedW );
	SetRT( m_RtCur );
}

void EGD11R_Renderer::SetGlobalSamplers( egv_rtarget Sampler0, egv_rtarget Sampler1, egv_rtarget Sampler2 )
{
	auto GetResView = [this]( egv_rtarget Sampler ) -> ID3D11ShaderResourceView*
	{
		if( Sampler == egv_rtarget::Null )
		{
			return nullptr;
		}
		else if( Sampler == egv_rtarget::DepthBuffer )
		{
			return m_RTs[FSRT_Z]->GetResourceView();
		}

		EGD11R_RenderTarget* Rt = reinterpret_cast<EGD11R_RenderTarget*>(static_cast<eg_uintptr_t>(Sampler));
		assert( Rt->IsA( &EGD11R_RenderTarget::GetStaticClass() ) );
		return Rt->GetResourceView();
	};
	
	m_GlobalSamplers[0] = GetResView ( Sampler0 );
	m_GlobalSamplers[1] = GetResView ( Sampler1 );
	m_GlobalSamplers[2] = GetResView ( Sampler2 );
	m_bGlobalSamplersDirty = true;
}

void EGD11R_Renderer::SetGlobalSamplers( RENDERER_FSRT_TYPE Sampler0, RENDERER_FSRT_TYPE Sampler1, RENDERER_FSRT_TYPE Sampler2 )
{
	auto GetResView = [this]( RENDERER_FSRT_TYPE Sampler ) -> ID3D11ShaderResourceView*
	{
		if( Sampler == FSRT_COUNT )
		{
			return nullptr;
		}

		return m_RTs[Sampler]->GetResourceView();
	};

	m_GlobalSamplers[0] = GetResView ( Sampler0 );
	m_GlobalSamplers[1] = GetResView ( Sampler1 );
	m_GlobalSamplers[2] = GetResView ( Sampler2 );
	m_bGlobalSamplersDirty = true;
}

void EGD11R_Renderer::BeginMainScene( eg_flags Flags )
{
	assert( !m_bInMainScene );
	m_bInMainScene = true;

	if( Flags.IsSet( DL_F_SCENE_DRAWTOZ ) )
	{
		m_bIsDrawingZScene = true;
		m_MainSceneRT = FSRT_Z;
		SetRT( m_MainSceneRT );
		ClearRT(eg_color(1,1,1,1)); // Effectively clears-z
		ClearDS( 1.0f , 0 );
		//Draw the scene using only z-values to the z-buffer target:
		SetMaterialOverride(m_DefShaders[ISHADER_DRAWTODEPTH]);
		m_bIsDrawingZScene = true;
	}
	else if( Flags.IsSet( DL_F_PROC_TOSCREEN ) )
	{
		m_MainSceneRT = FSRT_SCREEN;
		SetRT( m_MainSceneRT );
		ClearDS( 1.f , 0 );
	}
	else if( Flags.IsSet( DL_F_SCENE_REFLECT ) )
	{
		m_MainSceneRT = FSRT_REFLECT;
		SetRT( m_MainSceneRT );
		ClearDS( 1.f , 0 );

		m_CurRasterizerState.Push( eg_rasterizer_s::CULL_CW );
	}
	else if( Flags.IsSet( DL_F_SCENE_SHADOWS ) )
	{
		m_MainSceneRT = FSRT_SCREEN;
		SetRT( m_MainSceneRT );
		m_Context->ClearDepthStencilView( m_DsCur->GetDepthStencilView() , D3D11_CLEAR_STENCIL , 0.f , static_cast<UINT8>(0) );
	}
	else
	{
		m_MainSceneRT = FSRT_FS1;
		SetRT( m_MainSceneRT );
		ClearDS( 1.f , 0 );
	}
}

void EGD11R_Renderer::DoSceneProc( eg_flags Flags )
{
	assert( m_bInMainScene );
	assert( m_MainSceneRT == FSRT_FS1 || m_MainSceneRT == FSRT_FS2 ); // Can't do a proc unless rendering to a full-screen buffer.

	eg_mat mCurView = m_mV;
	eg_mat mCurProj = m_mP;
	
	if( Flags.IsSet( DL_F_PROC_STOREDEPTH ) )
	{
		assert( !m_bManualDrawZ );
		ResolveToRenderTarget( egv_rtarget::DepthBuffer );
		m_bHasDepth = true;
	}

	RENDERER_FSRT_TYPE LastRT = m_MainSceneRT;
	const eg_bool bToScreen = Flags.IsSet( DL_F_PROC_TOSCREEN );

	m_CurSamplerState.Push( eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT );

	auto MotionBlurPass = [this]( RENDERER_FSRT_TYPE From ) -> void
	{
		eg_mat mPrevVP = m_DepthBufferInfo.mPrevView * m_DepthBufferInfo.mPrevProj;
		eg_mat mInvVP = m_DepthBufferInfo.mCurView * m_DepthBufferInfo.mCurProj;
		mInvVP = eg_mat::BuildInverse( mInvVP , nullptr );

		m_CurBlendState.Push( eg_blend_s::BLEND_NONE_COLOR_ALL );
		m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
		m_CurDefaultShader.Push( eg_defaultshader_t::STORE_DEPTH );

		if( true )
		{
			SetMaterial( m_DefShaders[ISHADER_MOTIONBLUR1] );
			SetGlobalSamplers( From , FSRT_Z , FSRT_COUNT );
			SetVec4(eg_rv4_t::FREG_0, &eg_vec4(1.f/GetViewWidth(), 1.f/GetViewHeight(), 0.95f, 0.f));
			m_PSC.m_ps_PrevVP = eg_mat::BuildTranspose( mPrevVP );
			m_PSC.m_ps_InvVP = eg_mat::BuildTranspose( mInvVP );
			DrawBasicQuad();
		}
		else
		{
			// Draw the non-masked portion of the scene normally.
			SetMaterial( m_DefShaders[ISHADER_COPY] );
			SetGlobalSamplers( From , FSRT_COUNT , FSRT_COUNT );
			DrawBasicQuad();
		}

		m_CurDefaultShader.Pop();
		m_CurDepthStencilState.Pop();
		m_CurBlendState.Pop();
	};

	auto FxaaPass = [this]( RENDERER_FSRT_TYPE From ) -> void
	{
		SetMaterial( m_DefShaders[ISHADER_FXAA] );
		SetGlobalSamplers( From , FSRT_COUNT , FSRT_COUNT );
		SetVec4(eg_rv4_t::FREG_0, &eg_vec4(1.f/GetViewWidth(), 1.f/GetViewHeight(), 0.75f, 0.166f));
		DrawBasicQuad();
	};

	auto CopyPass = [this]( RENDERER_FSRT_TYPE From ) -> void
	{
		SetMaterial( m_DefShaders[ISHADER_COPY] );
		SetGlobalSamplers( From , FSRT_COUNT , FSRT_COUNT );
		m_CurSamplerState.Push( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );

		//Needs pixel size.
		SetVec4(eg_rv4_t::FREG_0, &eg_vec4(1.0f/GetViewWidth(), 1.0f/GetViewHeight(), 0, 0));
		DrawBasicQuad();

		m_CurSamplerState.Pop();
	};

	enum class eg_effects
	{
		COPYSCREEN,
		FXAA,
		MOTIONBLUR1,

		COUNT,
	};

	eg_effects Todo[static_cast<eg_uint>(eg_effects::COUNT)];
	eg_uint    TodoCount = 0;

	auto PushTodo = [&Todo,&TodoCount]( eg_effects What ) -> void
	{
		if( TodoCount < countof(Todo) )
		{
			Todo[TodoCount] = What;
			TodoCount++;
		}
		else
		{
			assert( false );
		}
	};

	if( Flags.IsSet( DL_F_PROC_MOTIONBLUR ) )
	{
		PushTodo( eg_effects::MOTIONBLUR1 );
		assert( m_bHasDepth );
		m_DepthBufferInfo.mCurView = mCurView;
		m_DepthBufferInfo.mCurProj = mCurProj;
	}

	if( Flags.IsSet( DL_F_PROC_FXAA ) )
	{
		PushTodo( eg_effects::FXAA );
	}

	if( 0 == TodoCount )
	{
		PushTodo( eg_effects::COPYSCREEN );
	}


	assert( TodoCount > 0 ); // Something went wrong?

	SetViewTF(eg_mat::I);
	SetProjTF(eg_mat::I);
	SetWorldTF(eg_mat::I);

	for( eg_uint i=0; i<TodoCount; i++ )
	{
		RENDERER_FSRT_TYPE Dest = ( LastRT == FSRT_FS1 ? FSRT_FS2 : FSRT_FS1 );

		if( i==(TodoCount-1) && bToScreen )
		{
			Dest = FSRT_SCREEN;
		}

		SetRT(Dest);

		m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );

		switch( Todo[i] )
		{
		case eg_effects::COPYSCREEN:
			CopyPass( m_MainSceneRT );
			break;
		case eg_effects::FXAA:
			FxaaPass( m_MainSceneRT );
			break;
		case eg_effects::MOTIONBLUR1:
		{
			// const eg_real xDist = EG_Abs(m_DepthBufferInfo.mPrevView.tx - m_DepthBufferInfo.mCurView.tx);
			// const eg_real yDist = EG_Abs(m_DepthBufferInfo.mPrevView.ty - m_DepthBufferInfo.mCurView.ty);
			// const eg_real zDist = EG_Abs(m_DepthBufferInfo.mPrevView.tz - m_DepthBufferInfo.mCurView.tz);

			const eg_real CameraMoveDistSq = (*reinterpret_cast<eg_vec3*>(&m_DepthBufferInfo.mPrevView.tx) - *reinterpret_cast<eg_vec3*>(&m_DepthBufferInfo.mCurView.tx)).LenSq();
			const eg_real MotionBlurDistThresholdSq = m_MotionBlurSkipDistSq;
			if( CameraMoveDistSq > MotionBlurDistThresholdSq )
			{
				// EGLogf( eg_log_t::General , "Skipped motion blur due to far camera  movement." );
				CopyPass( m_MainSceneRT );
			}
			else
			{
				MotionBlurPass( m_MainSceneRT );
			}
			m_DepthBufferInfo.mPrevView = m_DepthBufferInfo.mCurView;
			m_DepthBufferInfo.mPrevProj = m_DepthBufferInfo.mCurProj;
		} break;
		case eg_effects::COUNT:
			assert( false );
			break;
		}

		m_CurDepthStencilState.Pop();

		LastRT = Dest;
		m_MainSceneRT = Dest;
	}

	m_CurSamplerState.Pop();

	SetViewTF( mCurView );
	SetProjTF( mCurProj );

}

void EGD11R_Renderer::RunFullScreenProc( egv_material Material )
{
	assert( m_bInMainScene );
	assert( m_MainSceneRT == FSRT_FS1 || m_MainSceneRT == FSRT_FS2 ); // Can't do a proc unless rendering to a full-screen buffer.

	RENDERER_FSRT_TYPE LastRT = m_MainSceneRT;
	RENDERER_FSRT_TYPE Dest = ( LastRT == FSRT_FS1 ? FSRT_FS2 : FSRT_FS1 );

	SetRT( Dest );
	SetMaterial( Material );
	SetGlobalSamplers( m_MainSceneRT , FSRT_Z , FSRT_COUNT ); // For now the 2nd sampler is the z-buffer... Do we always want that?
	DrawBasicQuad();

	LastRT = Dest;
	m_MainSceneRT = Dest;
}

void EGD11R_Renderer::EndMainScene( eg_flags Flags )
{
	assert( m_bInMainScene );

	if( m_MainSceneRT == FSRT_Z )
	{
		assert( !Flags.IsSet( DL_F_PROC_TOSCREEN ) );
		assert( m_bIsDrawingZScene );
		m_bIsDrawingZScene = false;
		SetMaterialOverride(EGV_MATERIAL_NULL);
		m_MainSceneRT = FSRT_SCREEN;
		SetRT( m_MainSceneRT );
		m_bHasDepth = true;
	}
	else if( m_MainSceneRT == FSRT_REFLECT )
	{
		m_CurRasterizerState.Pop();
		SetClipPlane(nullptr); //Clear the clip plane
		m_MainSceneRT = FSRT_SCREEN;
		SetRT( m_MainSceneRT );
		// At this point we want to make the reflected scene available for drawing ....
		ID3D11ShaderResourceView* ResView[] = { m_RTs[FSRT_REFLECT]->GetResourceView() };
		m_Context->PSSetShaderResources( 7 , countof(ResView) , ResView );
	}
	else if( m_MainSceneRT != FSRT_SCREEN )
	{
		assert( Flags.IsSet( DL_F_PROC_TOSCREEN ) );
		DoSceneProc( Flags );
	}
	else
	{

	}

	m_bInMainScene = false;
}

void EGD11R_Renderer::ResetScreenViewport()
{
	m_ScreenViewport.Left = -m_fAspect;
	m_ScreenViewport.Right = m_fAspect;
	m_ScreenViewport.Bottom = -1.f;
	m_ScreenViewport.Top = 1.f;
	m_ScreenViewport.Near = 0.f;
	m_ScreenViewport.Far = 1.f;
}

void EGD11R_Renderer::Init_RenderTargets(eg_bool bInit)
{
	for(eg_uint i=0; i<countof(m_RTs); i++)
	{
		EG_SafeRelease( m_RTs[i] );
	}
	EG_SafeRelease( m_DepthBuffer );
	EG_SafeRelease( m_BackBuffer );

	if(bInit)
	{
		eg_uint RtWidth = m_ip.BufferWidth;
		eg_uint RtHeight = m_ip.BufferHeight;
		
		HRESULT Res = S_OK;
		//Get the back buffer render target and z buffer:
		m_BackBuffer = EGNewObject<EGD11R_RenderTarget>( eg_mem_pool::System );
		if( m_BackBuffer )
		{
			ID3D11Texture2D* ScreenBufferTexture = nullptr;
			Res = m_SwapChain ? m_SwapChain->GetBuffer( 0 , __uuidof(ScreenBufferTexture) , reinterpret_cast<void**>(&ScreenBufferTexture) ) : E_FAIL;
			if( SUCCEEDED(Res) && ScreenBufferTexture )
			{
				m_BackBuffer->InitRenderTargetWithTexture( m_Device , ScreenBufferTexture , m_ip.BufferWidth , m_ip.BufferHeight );
				ScreenBufferTexture->Release();
			}
			else
			{
				assert( false );
			}
		}

		m_DepthBuffer = EGNewObject<EGD11R_RenderTarget>( eg_mem_pool::System );
		if( m_DepthBuffer )
		{
			m_DepthBuffer->InitDepthStencil( m_Device , RtWidth , RtHeight );
		}

		//Create the additional full screen render targets:
		static const RENDERER_FSRT_TYPE CreateTbl[] =
		{
			FSRT_SCREEN,
			FSRT_FS1,
			FSRT_FS2,
			FSRT_Z,
			FSRT_REFLECT,
		};

		static_assert( countof(CreateTbl) == (FSRT_COUNT) , "Something missing from above?" );

		for(eg_uint i=0; i<countof(CreateTbl); i++)
		{
			RENDERER_FSRT_TYPE t = CreateTbl[i];

			eg_uint Width = RtWidth;
			eg_uint Height = RtHeight;
			DXGI_FORMAT Format = D3D_BB_FMT_A;

			if( FSRT_REFLECT == t )
			{
				Width  = EG_Min<eg_uint>(512,RtWidth);
				Height = EG_Min<eg_uint>(512,RtHeight);
			}

			if( FSRT_Z == t )
			{
				Format = D3D_BB_FMT_Z;
			}

			EGD11R_RenderTarget* Rt = EGNewObject<EGD11R_RenderTarget>( eg_mem_pool::System );
			if( Rt )
			{
				Rt->InitRenderTarget( m_Device , Format , Width , Height );
			}

			m_RTs[t] = Rt;
		}
	}
}

void EGD11R_Renderer::Init_Shaders(eg_bool bInit)
{
	for(eg_uint Type=0; Type < ISHADER_COUNT; Type++)
	{
		m_MtrlMgr.DestroyMaterial( m_DefShaders[Type] );
		m_DefShaders[Type] = EGV_MATERIAL_NULL;
	}

	if(bInit)
	{
		ResetBoneMats();

		static const eg_cpstr MeshShaderByQuality[SHADER_QUALITY_MAXENUM] =
		{
			("mesh_default_high"),
			("mesh_default_high"),
			("mesh_default_high"),
			("mesh_default_low"),
			("mesh_default_low"),
		};

		static const eg_cpstr MapShaderByQuality[SHADER_QUALITY_MAXENUM] =
		{
			("map_default_high"),
			("map_default_high"),
			("map_default_high"),
			("map_default_low"),
			("map_default_low"),
		};

		RENDERER_SHADER_QUALITY Quality = EGRenderer_ShaderQuality.GetValueThreadSafe();
		assert( 0 <= Quality && Quality < SHADER_QUALITY_MAXENUM );
		Quality = EG_Clamp<RENDERER_SHADER_QUALITY>(Quality, RENDERER_SHADER_QUALITY(0), RENDERER_SHADER_QUALITY(SHADER_QUALITY_MAXENUM-1));

		const struct
		{
			ISHADER_T Type;
			eg_cpstr  ShaderFile;
			eg_cpstr  Tex0File;
			eg_cpstr  Tex1File;
		} 
		ShaderInfo[] = 
		{
			{ ISHADER_DEFAULT           , "default_Texture"                  , nullptr , nullptr },
			{ ISHADER_MESH_UNLIT        , "mesh_default_unlit"               , "default_black" , "default_white"  },
			{ ISHADER_MESH_LIT          , MeshShaderByQuality[Quality]       , "default_black" , "default_normal" },
			{ ISHADER_MAP               , MapShaderByQuality[Quality]        , "default_black" , "default_white"  },
			{ ISHADER_FONT              , "default_font"                     , nullptr , nullptr },
			{ ISHADER_FONT_SHADOW       , "default_font_palette_color"       , nullptr , nullptr },
			{ ISHADER_FONT_GLYPH        , "default_font_glyph"               , nullptr , nullptr },
			{ ISHADER_FONT_GLYPH_SHADOW , "default_font_glyph_palette_color" , nullptr , nullptr },
			{ ISHADER_COLOR             , "default_Color"                    , nullptr , nullptr },
			{ ISHADER_SHADOW            , "default_Shadow"                   , nullptr , nullptr },
			{ ISHADER_TERRAIN           , "default_Terrain"                  , nullptr , nullptr },
			{ ISHADER_STOREDEPTH        , "storeds"                          , nullptr , nullptr },
			{ ISHADER_DRAWTODEPTH       , "drawtoz"                          , nullptr , nullptr },
			{ ISHADER_CLEARDEPTH        , "clearz"                           , nullptr , nullptr },
			{ ISHADER_FXAA              , "post/fxaa"                        , nullptr , nullptr },
			{ ISHADER_COPY              , "post/default_copy"                , nullptr , nullptr },
			{ ISHADER_MOTIONBLUR1       , "motionblur"                       , nullptr , nullptr },
			{ ISHADER_DRAWZBUFFER       , "drawds"                           , nullptr , nullptr },
			{ ISHADER_SPLASH            , "default_Texture"                  , SPLASH_TEX_FILE , nullptr },
		};

		for(eg_uint i=0; i<countof(ShaderInfo); i++)
		{
			ISHADER_T Type = ShaderInfo[i].Type;
			assert( m_DefShaders[Type] == 0);

			EGMaterialDef MatDef;
			EGString_Format(("/egdata/shaders/%s") , ShaderInfo[i].ShaderFile).CopyTo( MatDef.m_strPS , countof(MatDef.m_strPS) );
			EGString_Format(("/egdata/shaders/%s") , ShaderInfo[i].ShaderFile).CopyTo( MatDef.m_strVS , countof(MatDef.m_strVS) );
			if( nullptr != ShaderInfo[i].Tex0File ) EGString_Format(("/egdata/textures/%s") , ShaderInfo[i].Tex0File).CopyTo( MatDef.m_strTex[0] , countof(MatDef.m_strTex[0]) );
			if( nullptr != ShaderInfo[i].Tex1File ) EGString_Format(("/egdata/textures/%s") , ShaderInfo[i].Tex1File).CopyTo( MatDef.m_strTex[1] , countof(MatDef.m_strTex[1]) );
			
			if( ISHADER_SPLASH == Type )
			{
				EGString_Copy( MatDef.m_strTex[0] , ShaderInfo[i].Tex0File , countof(MatDef.m_strTex[0]) );
			}

			//Create the material:
			m_DefShaders[Type] = m_MtrlMgr.CreateMaterial( &MatDef , "" );
		}

		//Do a check to make sure all shaders got loaded:
		for(eg_uint i=0; i<ISHADER_COUNT; i++)
		{
			assert( EGV_MATERIAL_NULL != m_DefShaders[i] );
		}

	}
	else
	{
		//Already made sure everything was released at the start of the function.
	}
}

void EGD11R_Renderer::SetScreenViewport( const egScreenViewport& Vp )
{
	m_ScreenViewport = Vp;
}

void EGD11R_Renderer::ApplyScreenViewport()
{
	D3D11_VIEWPORT Vp;

	eg_uint Width  = m_BackBuffer->GetWidth();  
	eg_uint Height = m_BackBuffer->GetHeight();
	eg_real A = m_fAspect;

	Vp.TopLeftX = EGMath_GetMappedRangeValue( m_ScreenViewport.Left  , eg_vec2(-A,A) , eg_vec2(0,static_cast<eg_real>(Width)) );
	Vp.Width    = EGMath_GetMappedRangeValue( m_ScreenViewport.Right , eg_vec2(-A,A) , eg_vec2(0,static_cast<eg_real>(Width)) ) - Vp.TopLeftX;

	eg_real ScrBottom = EGMath_GetMappedRangeValue( m_ScreenViewport.Bottom , eg_vec2(-1.f,1.f) , eg_vec2(0,static_cast<eg_real>(Height)) );
	eg_real ScrTop = EGMath_GetMappedRangeValue( m_ScreenViewport.Top , eg_vec2(-1.f,1.f) , eg_vec2(0,static_cast<eg_real>(Height)) );

	Vp.TopLeftY = Height - ScrTop;
	Vp.Height   = Height - ScrBottom - Vp.TopLeftY;

	Vp.MinDepth = m_ScreenViewport.Near;
	Vp.MaxDepth = m_ScreenViewport.Far;

	m_Context->RSSetViewports( 1 , &Vp );
}

void EGD11R_Renderer::SetViewportToRT()
{
	D3D11_VIEWPORT Vp;

	Vp.TopLeftX = 0.f;
	Vp.TopLeftY = 0.f;
	Vp.Width = static_cast<eg_real>(m_RTs[m_RtCur]->GetWidth());
	Vp.Height = static_cast<eg_real>(m_RTs[m_RtCur]->GetHeight());
	Vp.MinDepth = 0.f;
	Vp.MaxDepth = 1.f;
	m_Context->RSSetViewports( 1 , &Vp );
}

void EGD11R_Renderer::Deinit()
{
	//If the IDirect3D interface is null then the system was never started
	//and it can be assumed that no other structures were created and so
	//the function should return immediately.
	if( !m_Device )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": The system was not initialized." );
			
		return;
	}

	if( !m_Context )
	{
		assert(false);
		return;
	}

	Init_Shaders(false);
	Init_RS(false);

	for( eg_uint i=0; i<countof(m_VsConstsBuffers); i++ )
	{
		EG_SafeRelease( m_VsConstsBuffers[i] );
	}
	EG_SafeRelease( m_PsConstsBuffer );

	// Delete all assets in the delete queue no matter what.
	ProcessDeleteQue( true , 0 );

	Shutdown_Mgrs();

	Invalidate();
	
	//For debugging the references are tracked to insure that everything
	//was released properly, and no resources were unmanaged.
	eg_uint nRef = 0;

	m_Context->IASetInputLayout( nullptr );
	EG_SafeDelete( m_BufferMgr );

	if( m_SwapChain )
	{
		nRef = m_SwapChain->Release();
		EGLogf( eg_log_t::Renderer11 , "Released swap chain: %u references remaining." , nRef);
		m_SwapChain=nullptr;
		assert(nRef == 0);
	}
	
	nRef = m_Context->Release();
	EGLogf( eg_log_t::Renderer11 , "Released context: %u references remaining." , nRef);
	m_Context=nullptr;
	assert(nRef == 0);

	nRef = m_Device->Release();
	EGLogf( eg_log_t::Renderer11 , "Released device: %u references remaining." , nRef);
	m_Device=nullptr;
	assert(nRef == 0);

	EGRenderer_ScreenRes.DeinitAvailableResolutions();
	
	Window_Deinit();
}

void EGD11R_Renderer::Validate_Buffers( eg_bool Validate )
{
	if( m_BufferMgr )
	{
		m_BufferMgr->Validate( Validate );
	}
}
void EGD11R_Renderer::Init_Mgrs()
{
	m_ShaderMgr.Init( m_Device );
}

void EGD11R_Renderer::Shutdown_Mgrs()
{
	//Reset the render target to make sure the surfaces get released properly.
	SetRT(FSRT_SCREEN);

	m_MtrlMgr.PurgeDestroyQueue();
	m_ShaderMgr.Deinit();

}

void EGD11R_Renderer::Validate()
{
	m_fAspect = (eg_real)GetViewWidth()/(eg_real)GetViewHeight();

	//Create the render targets:
	Init_RenderTargets(true);
	Validate_Buffers( true );

	if( m_ip.ScreenMode != eg_screen_mode::FULL_SCREEN )
	{
		DXGI_MODE_DESC Desc;
		Init_Device_AdjustRes( &Desc , false );
		m_SwapChain->ResizeTarget( &Desc );
	}

	if( false )// IS_DEBUG ) // In debug mode we put the window at the top of the monitor so we can see the console which will be at the bottom.
	{
		RECT rcWindow;
		GetWindowRect( m_hwnd , &rcWindow );
		MoveWindow( m_hwnd , rcWindow.left , 0 , rcWindow.right-rcWindow.left , rcWindow.bottom-rcWindow.top , FALSE );
	}

	if( m_SwapChain )
	{
		m_SwapChain->SetFullscreenState( m_ip.ScreenMode == eg_screen_mode::FULL_SCREEN , NULL );
		if( m_ip.ScreenMode == eg_screen_mode::FULL_SCREEN )
		{
			BOOL bSwapChainFS = FALSE;
			if( SUCCEEDED(m_SwapChain->GetFullscreenState( &bSwapChainFS , NULL )) && bSwapChainFS )
			{
				DXGI_MODE_DESC Desc;
				Init_Device_AdjustRes( &Desc , true );
				m_SwapChain->ResizeTarget( &Desc );
			}
		}
	}
}

void EGD11R_Renderer::Invalidate()
{
	if( m_SwapChain )
	{
		m_SwapChain->SetFullscreenState( FALSE , NULL );
	}
	Validate_Buffers( false );
	Init_RenderTargets(false);
}

eg_bool EGD11R_Renderer::Reset_Internal()
{
	SetRT(FSRT_SCREEN);
	SetMaterial(EGV_MATERIAL_NULL);


	HRESULT nRes = S_OK;
	EGLogf( eg_log_t::Renderer11 , "Resetting renderer." );

	Invalidate();
	if( true ) // m_ip.ScreenMode == eg_screen_mode::FULL_SCREEN )
	{
#if 1
		ULONG SwapChainCount = m_SwapChain->Release();
		m_SwapChain = nullptr;
		assert( 0 == SwapChainCount );

		SetPPs();

		DXGI_SWAP_CHAIN_DESC Sd;
		memset( &Sd , 0 , sizeof(Sd) );
		DXGI_MODE_DESC Mode;
		Init_Device_AdjustRes( &Mode , true );
		Sd.BufferDesc = Mode;
		Sd.SampleDesc.Count = 1;
		Sd.SampleDesc.Quality = 0;
		Sd.BufferUsage = DXGI_USAGE_BACK_BUFFER|DXGI_USAGE_RENDER_TARGET_OUTPUT;
		Sd.BufferCount = 1;
		Sd.OutputWindow = m_hwnd;
		Sd.Windowed = TRUE;//Always start windowed. //m_ip.Windowed;
		Sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		Sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGIDevice* GiDevice = nullptr;
		HRESULT Res = m_Device ? m_Device->QueryInterface( __uuidof(IDXGIDevice) , reinterpret_cast<void**>(&GiDevice) ) : E_FAIL;
		assert( SUCCEEDED(Res) && GiDevice );
		IDXGIAdapter* GiAdapter = nullptr;
		Res = GiDevice ? GiDevice->GetAdapter( &GiAdapter ) : E_FAIL;
		assert( SUCCEEDED(Res) && GiAdapter );
		IDXGIFactory* GiFactory = nullptr;
		Res = GiAdapter ? GiAdapter->GetParent( __uuidof(IDXGIFactory) , reinterpret_cast<void**>(&GiFactory) ) : E_FAIL;
		assert( SUCCEEDED(Res) && GiFactory );
		if( GiFactory )
		{
			GiFactory->CreateSwapChain( GiDevice , &Sd , &m_SwapChain );
		}

		EG_SafeRelease( GiFactory );
		EG_SafeRelease( GiAdapter );
		EG_SafeRelease( GiDevice );

		assert( nullptr != m_SwapChain );
		nRes = S_OK;

#else
		SetPPs();
		nRes = m_SwapChain ? m_SwapChain->ResizeBuffers( 1 , m_ip.BufferWidth , m_ip.BufferHeight , D3D_BB_FMT_X , 0 ) : E_FAIL;
#endif
	}
	else
	{
		SetPPs();
		nRes = m_SwapChain ? m_SwapChain->ResizeBuffers( 1 , m_ip.BufferWidth , m_ip.BufferHeight , D3D_BB_FMT_A , 0 ) : E_FAIL;
	}

	//Init_Shaders( false );
	//Init_Shaders( true );
	//Init_RS( false );
	//Init_RS( true );

	if(SUCCEEDED(nRes))
	{
		Validate();
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": Device reset failed. Trying to reset with basic PPs..." );
		SetBasicPPs();
		nRes = m_SwapChain->ResizeBuffers( 1 , m_ip.BufferWidth , m_ip.BufferHeight , D3D_BB_FMT_A , 0 );
		if(SUCCEEDED(nRes))
		{
			Validate();
		}
		else
		{
			//assert( false );
			EGLogf( eg_log_t::Error , __FUNCTION__ ": Reset failed.\nThe device could not be reset, the game will probably crash now." );
		}
	}

	return SUCCEEDED(nRes);
}

void EGD11R_Renderer::Reset()
{
	if( INIT_SUCCEEDED == m_InitStatus )
	{
		m_ResetRequested = true;
		m_ResetTimer = m_ResetCountDownTime;
	}
}

void EGD11R_Renderer::Init_Device_AdjustRes( DXGI_MODE_DESC* ModeOut , bool bWantsBufferRes )
{
	IDXGIFactory* Factory = NULL;
	HRESULT Res = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&Factory) );
	assert( SUCCEEDED(Res) );

	IDXGIAdapter* Adapter = NULL;
	Res = Factory->EnumAdapters( 0 , &Adapter );
	assert( SUCCEEDED(Res) );

	IDXGIOutput* Output = NULL;
	Res = Adapter->EnumOutputs( 0 , &Output );
	assert( SUCCEEDED(Res) );

	DXGI_MODE_DESC MatchMode;
	MatchMode.Width = bWantsBufferRes ? m_ip.BufferWidth : m_ip.ScreenWidth;
	MatchMode.Height = bWantsBufferRes ? m_ip.BufferHeight : m_ip.ScreenHeight;
	MatchMode.RefreshRate.Numerator = 60;
	MatchMode.RefreshRate.Denominator = 1;
	MatchMode.Format = D3D_BB_FMT_A;
	MatchMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	MatchMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	memset( ModeOut , 0 , sizeof(*ModeOut) );
	Output->FindClosestMatchingMode( &MatchMode , ModeOut , NULL );
	if( m_ip.ScreenMode == eg_screen_mode::WINDOWED )
	{
		ModeOut->Width = m_ip.ScreenWidth;
		ModeOut->Height = m_ip.ScreenHeight;
	}

	if( bWantsBufferRes )
	{
		m_ip.BufferWidth = ModeOut->Width;
		m_ip.BufferHeight = ModeOut->Height;
	}
	else
	{
		m_ip.ScreenWidth = ModeOut->Width;
		m_ip.ScreenHeight = ModeOut->Height;
	}

	//Compute all available resolutions:
	EGArray<eg_ivec2> AllRes;
	m_Resolutions.Clear();
	UINT NumModes = 0;
	Output->GetDisplayModeList( D3D_BB_FMT_A , 0 , &NumModes , nullptr );
	const eg_uint DescsSize = sizeof(DXGI_MODE_DESC)*NumModes;
	DXGI_MODE_DESC* Descs = EGMem2_NewArray<DXGI_MODE_DESC>( NumModes , eg_mem_pool::DefaultHi );
	if( Descs )
	{
		EGMem_Set( Descs , 0 , DescsSize );
		Output->GetDisplayModeList( D3D_BB_FMT_A , 0 , &NumModes , Descs );

		for(eg_uint i=0; i<NumModes; i++)
		{
			DXGI_MODE_DESC& dm = Descs[i];
			AddSupportedResolution( dm.Width , dm.Height );
			AllRes.AppendUnique( eg_ivec2(dm.Width,dm.Height) );
		}

		EGMem2_Free( Descs );
	}

	EG_SafeRelease( Output );
	EG_SafeRelease( Adapter );
	EG_SafeRelease( Factory );

	AllRes.Sort( []( const eg_ivec2& Left , const eg_ivec2& Right ) -> eg_bool
	{
		if( Left.x != Right.x )
		{
			return Left.x > Right.x;
		}

		return Left.y > Right.y;
	} );

	EGRenderer_ScreenRes.InitAvailableResolutions( AllRes );
}

void EGD11R_Renderer::Init_Device()
{
	HRESULT nRes=0;

	SetPPs();

	IDXGIAdapter* AdapterToUse = Init_Device_SelectAdapter();

	//Startup Direct3D:
	DXGI_MODE_DESC Mode;
	Init_Device_AdjustRes( &Mode , true );
	HRESULT Res;
	static const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };//, D3D_FEATURE_LEVEL_11_1 };
	DXGI_SWAP_CHAIN_DESC Sd;
	memset( &Sd , 0 , sizeof(Sd) );
	Sd.BufferDesc = Mode;
	Sd.SampleDesc.Count = 1;
	Sd.SampleDesc.Quality = 0;
	Sd.BufferUsage = DXGI_USAGE_BACK_BUFFER|DXGI_USAGE_RENDER_TARGET_OUTPUT;
	Sd.BufferCount = 1;
	Sd.OutputWindow = m_hwnd;
	Sd.Windowed = TRUE;//Always start windowed. //m_ip.Windowed;
	Sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	Sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT DeviceFlags = 0;//D3D11_CREATE_DEVICE_SINGLETHREADED;
	if( false && IS_DEBUG )
	{
		DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	Res = D3D11CreateDeviceAndSwapChain( AdapterToUse , AdapterToUse ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, NULL , DeviceFlags , FeatureLevels , countof(FeatureLevels), D3D11_SDK_VERSION , &Sd , &m_SwapChain , &m_Device , &m_FeatureLevel , &m_Context );
	
	EG_SafeRelease( AdapterToUse );

	if( FAILED( Res ) )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ " Error: Could not create D3D interfaces. Make sure the latest DirectX version is installed." );
		MessageBoxW( m_hwnd , L"A DirectX 11.0 compatible display device is required to run this game. You may need to install the latest graphics drivers and update DirectX." , *Engine_GetGameTitle() , MB_OK );
		return;
	}

	if( m_Device && m_Context && m_SwapChain )
	{
		IDXGIFactory* GIFactory = nullptr;
		m_SwapChain->GetParent( __uuidof(IDXGIFactory) , reinterpret_cast<void**>(&GIFactory) );
		if( GIFactory )
		{
			GIFactory->MakeWindowAssociation( m_hwnd , DXGI_MWA_NO_WINDOW_CHANGES|DXGI_MWA_NO_ALT_ENTER );
			GIFactory->Release();
		}

		IDXGIDevice* GIDevice = nullptr;
		m_Device->QueryInterface( __uuidof(IDXGIDevice) , reinterpret_cast<void**>(&GIDevice) );
		if( GIDevice )
		{
			IDXGIAdapter* GIAdapter = nullptr;
			GIDevice->GetAdapter( &GIAdapter );
			if( GIAdapter )
			{
				DXGI_ADAPTER_DESC Desc;
				zero( &Desc );
				GIAdapter->GetDesc( &Desc );
				EGLogf( eg_log_t::General , "Using D3D11 Device %s." , *eg_d_string8( Desc.Description ) );
				EG_SafeRelease( GIAdapter );
			}
			EG_SafeRelease( GIDevice );
		}

		eg_bool bIsDevOkay = Init_VerifyDevCaps();

		if(bIsDevOkay)
		{
			assert( nullptr == m_BufferMgr );
			m_BufferMgr = new ( eg_mem_pool::RenderResource ) EGD11R_BufferMgr( m_Device , m_Context );
		}

		if(!bIsDevOkay)
		{
			EG_SafeDelete( m_BufferMgr );
			EG_SafeRelease(m_Device);
			EG_SafeRelease(m_Context);
			EG_SafeRelease(m_SwapChain);
		}
	}
}

IDXGIAdapter* EGD11R_Renderer::Init_Device_SelectAdapter()
{
	IDXGIAdapter* AdapterToUse = nullptr;
	HRESULT nRes = S_OK;
	// Enumerate devices...
	if( nullptr == AdapterToUse && EGD11R_Renderer_DebugPickAdapter.GetValueThreadSafe() )
	{
		IDXGIFactory* DxGiFactory = nullptr;
		nRes = CreateDXGIFactory( __uuidof(IDXGIFactory) , reinterpret_cast<void**>(&DxGiFactory) );
		if( SUCCEEDED(nRes) && DxGiFactory )
		{
			IDXGIAdapter* Adapter = nullptr;
			for( UINT i = 0; DxGiFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; i++ )
			{
				DXGI_ADAPTER_DESC AdapterDesc;
				zero( &AdapterDesc );
				Adapter->GetDesc( &AdapterDesc );
				eg_d_string8 AdapterStr = EGSFormat8( "Adapter {0}: {1}" , i , AdapterDesc.Description );
				EGLogf( eg_log_t::General , "Adapter %d: %s" , i , *AdapterStr);
				if( IDYES == MessageBoxW( m_hwnd , *eg_d_string16(*AdapterStr) , *Engine_GetGameTitle() , MB_YESNO ) )
				{
					AdapterToUse = Adapter;
					AdapterToUse->AddRef();
				}

				EG_SafeRelease( Adapter );
			}
		}

		EG_SafeRelease( DxGiFactory );
	}

	const eg_s_string_sml8 AdapterName = EGD11R_Renderer_Adapter.GetValueThreadSafe();
	if( nullptr == AdapterToUse && AdapterName.Len() > 0 )
	{
		IDXGIFactory* DxGiFactory = nullptr;
		nRes = CreateDXGIFactory( __uuidof(IDXGIFactory) , reinterpret_cast<void**>(&DxGiFactory) );
		if( SUCCEEDED(nRes) && DxGiFactory )
		{
			IDXGIAdapter* Adapter = nullptr;
			for( UINT i = 0; DxGiFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; i++ )
			{
				DXGI_ADAPTER_DESC AdapterDesc;
				zero( &AdapterDesc );
				Adapter->GetDesc( &AdapterDesc );
				if( AdapterName.EqualsI( *eg_s_string_sml8(AdapterDesc.Description) ) )
				{
					EG_SafeRelease( AdapterToUse );
					AdapterToUse = Adapter;
					AdapterToUse->AddRef();
				}

				EG_SafeRelease( Adapter );
			}
		}

		EG_SafeRelease( DxGiFactory );
	}

	if( nullptr == AdapterToUse )
	{
		// If there is only one hardware adapter return nullptr
		// If there is more than one pick the one with the highest dedicated video memory

		SIZE_T HighestDedicatedMemory = 0;
		eg_int NumDevicesFound = 0;
		IDXGIFactory* DxGiFactory = nullptr;
		nRes = CreateDXGIFactory( __uuidof(IDXGIFactory) , reinterpret_cast<void**>(&DxGiFactory) );
		if( SUCCEEDED(nRes) && DxGiFactory )
		{
			IDXGIAdapter* Adapter = nullptr;
			for( UINT i = 0; DxGiFactory->EnumAdapters(i, &Adapter) != DXGI_ERROR_NOT_FOUND; i++ )
			{
				NumDevicesFound++;
				DXGI_ADAPTER_DESC AdapterDesc;
				zero( &AdapterDesc );
				Adapter->GetDesc( &AdapterDesc );
				if( AdapterDesc.DedicatedVideoMemory > HighestDedicatedMemory )
				{
					EG_SafeRelease( AdapterToUse );
					AdapterToUse = Adapter;
					AdapterToUse->AddRef();
					HighestDedicatedMemory = AdapterDesc.DedicatedVideoMemory;
				}

				EG_SafeRelease( Adapter );
			}
		}

		EG_SafeRelease( DxGiFactory );

		if( NumDevicesFound <= 2 )
		{
			EG_SafeRelease( AdapterToUse );
		}
	}

	return AdapterToUse;
}

void EGD11R_Renderer::SetPPs()
{
	RECT rcDT;
	zero( &rcDT );
	GetClientRect(GetDesktopWindow(), &rcDT);

	eg_ivec2 SettingsRes = EGRenderer_ScreenRes.GetValueThreadSafe();
	if( SettingsRes.x == 0 || SettingsRes.y == 0 )
	{
		SettingsRes.x = rcDT.right - rcDT.left;
		SettingsRes.y = rcDT.bottom - rcDT.top;
		if( IS_DEBUG )
		{
			SettingsRes.x = EG_Min<eg_int>( SettingsRes.x , 1280 );
			SettingsRes.y = EG_Min<eg_int>( SettingsRes.y , 720 );
		}
		EGRenderer_ScreenRes.SetValue( SettingsRes );
	}
	m_ip.ScreenWidth = SettingsRes.x;
	m_ip.ScreenHeight = SettingsRes.y;
	m_ip.BufferWidth = m_ip.ScreenWidth;
	m_ip.BufferHeight = m_ip.ScreenHeight;
	m_ip.ScreenMode = VideoConfig_IsWindowed.GetValueThreadSafe() ? eg_screen_mode::WINDOWED : eg_screen_mode::BORDERLESS;
	if( !VideoConfig_IsWindowed.GetValueThreadSafe() && VideoConfig_IsExclusiveFullScreen.GetValueThreadSafe() )
	{
		m_ip.ScreenMode = eg_screen_mode::FULL_SCREEN;
	}
	m_ip.VSync = VideoConfig_VSyncEnabled.GetValueThreadSafe();

	if( m_ip.ScreenMode == eg_screen_mode::BORDERLESS )
	{
		m_ip.ScreenWidth = rcDT.right-rcDT.left;
		m_ip.ScreenHeight = rcDT.bottom-rcDT.top;
	}

	//Since the Present parameters determine the window size, it will
	//be changed here.
	if( m_ip.ScreenMode == eg_screen_mode::WINDOWED || m_ip.ScreenMode == eg_screen_mode::BORDERLESS )
	{
		const eg_bool bWindowed = (m_ip.ScreenMode == eg_screen_mode::WINDOWED);

		RECT rc;
		eg_int nX, nY;

		DWORD OldStyle = GetWindowLongW( m_hwnd , GWL_STYLE );
		
		if( bWindowed )
		{
			SetRect(&rc, 0, 0, m_ip.ScreenWidth, m_ip.ScreenHeight);
			AdjustWindowRectEx(&rc, GetWindowLong(m_hwnd, GWL_STYLE), GetMenu(m_hwnd)!=NULL, GetWindowLong(m_hwnd, GWL_EXSTYLE));

			//Determine where the window should be moved to, the center is the
			//desired position, but if the window is bigger than the desktop
			//that probably won't happen.
			nX = (rcDT.right-rcDT.left)/2 - (rc.right-rc.left)/2;
			nY = (rcDT.bottom-rcDT.top)/2 - (rc.bottom-rc.top)/2;

			if( !m_bIsChildWnd )
			{
				DWORD NewStyle = (OldStyle&(~WINDOW_STYLE_BORDERLESS))|WINDOW_STYLE_WINDOWED;
				SetWindowLongW( m_hwnd , GWL_STYLE , NewStyle );
			}
		}
		else
		{
			rc = rcDT;
			nX = 0;
			nY = 0;

			if( !m_bIsChildWnd )
			{
				DWORD NewStyle = (OldStyle&(~WINDOW_STYLE_WINDOWED))|WINDOW_STYLE_BORDERLESS;
				SetWindowLongW( m_hwnd , GWL_STYLE , NewStyle );
			}
		}

		SetWindowPos(
			m_hwnd,
			NULL,
			nX,
			nY,
			(rc.right-rc.left),
			(rc.bottom-rc.top),
			/*SWP_NOMOVE|*/SWP_NOZORDER|SWP_NOREDRAW|SWP_NOACTIVATE);
	}
	else if( m_ip.ScreenMode == eg_screen_mode::FULL_SCREEN )
	{
		// SetWindowLongW( m_hwnd , GWL_STYLE , m_OriginalWindowsStyle );
		// SetWindowLongW( m_hwnd , GWL_EXSTYLE , m_OriginalWindowsStyleEx );
	}
}

eg_bool EGD11R_Renderer::Init_VerifyDevCaps()
{
	//Any device capabilities we use should be checked here:
	eg_bool bValid = true;

	static_assert( sizeof(m_VSC) <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*sizeof(eg_vec4) , "Device caps not met!" );
	static_assert( sizeof(m_PSC) <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*sizeof(eg_vec4) , "Device caps not met!" );

	static_assert( 2 <= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT , "Device caps not met!" );

	assert(bValid);
	return bValid;
}

void EGD11R_Renderer::SetBasicPPs()
{
	m_ip.ScreenWidth    = 800;
	m_ip.ScreenHeight   = 600;
	m_ip.BufferWidth = m_ip.ScreenWidth;
	m_ip.BufferHeight = m_ip.ScreenHeight;
	m_ip.ScreenMode = eg_screen_mode::BORDERLESS;
	m_ip.VSync    = false;
}

/******************************
*** Vertex Buffer Functions ***
******************************/
egv_vbuffer EGD11R_Renderer::CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const void* pVerts )
{
	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	return m_BufferMgr ? m_BufferMgr->CreateVB( VertexType , nNumVerts , pVerts ) : CT_Clear;
}
	
/*******************************
*** Index buffer functions. ***
*******************************/

egv_ibuffer EGD11R_Renderer::CreateIB(eg_uint nNumInds, const egv_index* pIndices)
{
	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	return m_BufferMgr ? m_BufferMgr->CreateIB( nNumInds , pIndices ) : EGV_IBUFFER_NULL;
}

egv_rtarget EGD11R_Renderer::CreateRenderTarget( eg_uint Width, eg_uint Height )
{
	static_assert( sizeof(egv_rtarget) == sizeof(EGD11R_RenderTarget*) , "Render Target must fit." );

	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	EGD11R_RenderTarget* NewRT = EGNewObject<EGD11R_RenderTarget>( eg_mem_pool::System );
	if( NewRT )
	{
		NewRT->InitRenderTarget( m_Device , D3D_BB_FMT_A , Width , Height );
	}

	return static_cast<egv_rtarget>(reinterpret_cast<eg_uintptr_t>(NewRT));
}

EGVDynamicBuffer* EGD11R_Renderer::CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize, eg_uint IndexBufferSize )
{
	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	EGVDynamicBuffer* Out = nullptr;

	EGVDynamicBuffer::LockDynamicBuffers();

	eg_size_t VertexSize = m_BufferMgr ? m_BufferMgr->GetVertexFormatForType( VertexType ).VertexSize : 0;
	egv_vbuffer NewVBuffer = VertexBufferSize > 0 ? CreateVB( VertexType , VertexBufferSize , nullptr ) : CT_Clear;
	egv_ibuffer NewIBuffer = IndexBufferSize > 0 ? CreateIB( IndexBufferSize , nullptr ) : EGV_IBUFFER_NULL;
	Out = new ( eg_mem_pool::RenderResource ) EGVDynamicBuffer( VertexSize , NewVBuffer , NewVBuffer.IsValid() ? VertexBufferSize : 0 , NewIBuffer , NewIBuffer != EGV_IBUFFER_NULL ? IndexBufferSize : 0 );

	EGVDynamicBuffer::UnlockDynamicBuffers();

	return Out;
}

void EGD11R_Renderer::DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	EGVDynamicBuffer::LockDynamicBuffers();
	if( DynamicBuffer )
	{
		if( m_DynBufferUpdateList.Contains( DynamicBuffer ) )
		{
			m_DynBufferUpdateList.DeleteByItem( DynamicBuffer );
		}

		if( DynamicBuffer->GetVertexBuffer().IsValid() )
		{
			ReleaseVB( DynamicBuffer->GetVertexBuffer() );
		}
		if( DynamicBuffer->GetIndexBuffer() != EGV_IBUFFER_NULL )
		{
			ReleaseIB( DynamicBuffer->GetIndexBuffer() );
		}
		delete DynamicBuffer;
	}

	EGVDynamicBuffer::UnlockDynamicBuffers();
}

void EGD11R_Renderer::UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer )
{
	EGVDynamicBuffer::LockDynamicBuffers();
	if( DynamicBuffer )
	{
		m_DynBufferUpdateList.AppendUnique( DynamicBuffer );
	}
	EGVDynamicBuffer::UnlockDynamicBuffers();
}

void EGD11R_Renderer::HandleDeleteAsset( const egDestroyItem& DestroyItem )
{
	switch( DestroyItem.Type )
	{
		case eg_asset_t::MATERIAL:
		{
			EGLogf( eg_log_t::RendererActivity , "Deleting material %u." , DestroyItem.Material );
			m_MtrlMgr.DestroyMaterial( DestroyItem.Material );
		} break;
		case eg_asset_t::VB:
		{
			EGLogf( eg_log_t::RendererActivity , "Deleting vertex buffer %u." , DestroyItem.VBuffer );
			if( m_BufferMgr )
			{
				m_BufferMgr->DestroyVB( DestroyItem.VBuffer );
			}
		} break;
		case eg_asset_t::IB:
		{
			EGLogf( eg_log_t::RendererActivity , "Deleting index buffer %u." , DestroyItem.IBuffer );
			if( m_BufferMgr )
			{
				m_BufferMgr->DestroyIB( DestroyItem.IBuffer );
			}
		} break;
		case eg_asset_t::RT:
		{
			EGD11R_RenderTarget* Rt = reinterpret_cast<EGD11R_RenderTarget*>(static_cast<eg_uintptr_t>( DestroyItem.RTarget ) );
			if( Rt )
			{
				assert( Rt->IsA( &EGD11R_RenderTarget::GetStaticClass() ) );
				EGDeleteObject( Rt );
			}

		} break;
	}
}

void EGD11R_Renderer::Consts_CopyToDevice()
{
	assert( m_VSC.m_mBoneM[0].IsIdentity() );
	BeginClip();

	ID3D11Buffer* Buffers[2] = { nullptr , nullptr };

	//Hardly matters if we copy the whole buffer or not since mapping and unmapping copies everything.
	//const eg_size_t ConstSize = m_VSC.REG_SZ_ALL*sizeof(eg_vec4)-(RENDERER_MAX_BONES-m_NumBones)*sizeof(eg_mat);

	D3D11_MAPPED_SUBRESOURCE Resource;
	HRESULT Res;
	//Always take the base buffer
	{
		eg_uint BufferIndex = static_cast<eg_uint>(VSB_T::BASE);
		Res = m_Context->Map( m_VsConstsBuffers[BufferIndex] , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &Resource );
		if( S_OK == Res )
		{
			const eg_size_t ConstSize = egRVSConsts::SZ_BASE;
			EGMem_Copy( Resource.pData , &m_VSC , ConstSize );
			m_Context->Unmap( m_VsConstsBuffers[BufferIndex] , 0 );
		}
		else
		{
			assert( false );
		}

		Buffers[0] = m_VsConstsBuffers[BufferIndex];
	}

	if( m_NumBones > 1 ) //If there is more than one bone, take the bone buffer
	{
		eg_uint BufferIndex = static_cast<eg_uint>(VSB_T::BONES);
		Res = m_Context->Map( m_VsConstsBuffers[BufferIndex] , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &Resource );
		if( S_OK == Res )
		{
			const eg_size_t ConstSize = egRVSConsts::SZ_BONES;
			EGMem_Copy( Resource.pData , &m_VSC.m_mBoneM , ConstSize );
			m_Context->Unmap( m_VsConstsBuffers[BufferIndex] , 0 );
		}
		else
		{
			assert( false );
		}

		Buffers[1] = m_VsConstsBuffers[BufferIndex];
	}
	else //Otherwise only take the first bone. The first bone should always be the identity matrix.
	{
		eg_uint BufferIndex = static_cast<eg_uint>(VSB_T::IBONE);
		Res = m_Context->Map( m_VsConstsBuffers[BufferIndex] , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &Resource );
		if( S_OK == Res )
		{
			const eg_size_t ConstSize = sizeof(eg_mat);
			EGMem_Copy( Resource.pData , &m_VSC.m_mBoneM , ConstSize );
			m_Context->Unmap( m_VsConstsBuffers[BufferIndex] , 0 );
		}
		else
		{
			assert( false );
		}

		Buffers[1] = m_VsConstsBuffers[BufferIndex];
	}

	Res = m_Context->Map( m_PsConstsBuffer , 0 , D3D11_MAP_WRITE_DISCARD , 0 , &Resource );
	if( S_OK == Res )
	{
		EGMem_Copy( Resource.pData , &m_PSC , m_PSC.REG_SZ_ALL*sizeof(eg_vec4) );
		m_Context->Unmap( m_PsConstsBuffer , 0 );
	}
	else
	{
		assert( false );
	}

	m_Context->VSSetConstantBuffers( 0 , countof(Buffers) , Buffers );
	m_Context->PSSetConstantBuffers( 0 , 1 , &m_PsConstsBuffer );

	EndClip();
}

void EGD11R_Renderer::Update_RefreshDynamicBuffers( eg_real DeltaTime )
{
	unused( DeltaTime );

	EGVDynamicBuffer::LockDynamicBuffers();

	if( m_DynBufferUpdateList.HasItems() )
	{
		while( m_DynBufferUpdateList.HasItems() )
		{
			EGVDynamicBuffer* DynBuffer = m_DynBufferUpdateList.Last();
			m_DynBufferUpdateList.DeleteByItem( DynBuffer );
			if( DynBuffer )
			{
				{
					egv_vbuffer VBuffer = DynBuffer->GetVertexBuffer();
					const eg_byte* SourceV = DynBuffer->GetVertexBufferData<eg_byte>();
					const eg_size_t SourceVSize = DynBuffer->GetVertexSize() * DynBuffer->GetVertexBufferDataCount();
					if( m_BufferMgr )
					{
						m_BufferMgr->SetBufferData( VBuffer , SourceV , SourceVSize );
					}
				}

				{
					egv_ibuffer IBuffer = DynBuffer->GetIndexBuffer();
					const egv_index* SourceI = DynBuffer->GetIndexBufferData();
					const eg_size_t SourceISize = sizeof(SourceI[0]) * DynBuffer->GetIndexBufferDataCount();
					if( m_BufferMgr )
					{
						m_BufferMgr->SetBufferData( IBuffer , SourceI , SourceISize );
					}
				}
			}
		}
	}

	EGVDynamicBuffer::UnlockDynamicBuffers();
}

void EGD11R_Renderer::SetVertexLayout( eg_string_crc NewLayoutType )
{
	if( m_LastLayoutType != NewLayoutType )
	{
		m_LastLayoutType = NewLayoutType;
		if( m_BufferMgr )
		{
			m_BufferMgr->SetVertexLayout( NewLayoutType );
		}
	}
}

void EGD11R_Renderer::DrawLine(const eg_vec4* pvStart, const eg_vec4* pvEnd, eg_color32 color)
{
	SetMaterial( EGV_MATERIAL_NULL );
	m_CurDefaultShader.Push( eg_defaultshader_t::COLOR );
	CommitRenderStates();

	assert(m_bIsDrawing);
	egv_vert_simple v[2];
	v[0].Pos = *pvStart;
	v[0].Color0 = eg_color(color);
	v[1].Pos = *pvEnd;
	v[1].Color0 = eg_color(color);

	assert_pointer(m_pFXCur);
	const EGD11R_Mtrl* pE = m_pFXCur;
	if(!pE)return;

	//Copy vertexes into vertex buffer:
	m_BufferMgr->UpdateSimpleVB( &v , sizeof(v) );
	
	Consts_CopyToDevice();

	SetVertexLayout( eg_crc("egv_vert_simple") );
	m_Context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

	//Render by shader:
	pE->Begin( m_Context );
	m_Context->Draw( 2 , 0 );
	pE->End( m_Context );

	m_CurDefaultShader.Pop();
}

void EGD11R_Renderer::DrawAABB(const eg_aabb* pBox, eg_color32 color)
{
	assert(m_bIsDrawing);
	egv_vert_simple vAABB[16];
	
	for(eg_uint i=0; i<16; i++)
	{
		vAABB[i].Color0=eg_color(color);
		vAABB[i].Pos.w = 1;
	}
	
	vAABB[0].Pos.x=(*pBox).Min.x;
	vAABB[0].Pos.y=(*pBox).Min.y;
	vAABB[0].Pos.z=(*pBox).Min.z;
	
	vAABB[1].Pos.x=(*pBox).Min.x;
	vAABB[1].Pos.y=(*pBox).Max.y;
	vAABB[1].Pos.z=(*pBox).Min.z;
	
	vAABB[2].Pos.x=(*pBox).Min.x;
	vAABB[2].Pos.y=(*pBox).Max.y;
	vAABB[2].Pos.z=(*pBox).Max.z;

	vAABB[3].Pos.x=(*pBox).Max.x;
	vAABB[3].Pos.y=(*pBox).Max.y;
	vAABB[3].Pos.z=(*pBox).Max.z;

	vAABB[4].Pos.x=(*pBox).Max.x;
	vAABB[4].Pos.y=(*pBox).Max.y;
	vAABB[4].Pos.z=(*pBox).Min.z;

	vAABB[5].Pos.x=(*pBox).Max.x;
	vAABB[5].Pos.y=(*pBox).Min.y;
	vAABB[5].Pos.z=(*pBox).Min.z;

	vAABB[6].Pos.x=(*pBox).Max.x;
	vAABB[6].Pos.y=(*pBox).Min.y;
	vAABB[6].Pos.z=(*pBox).Max.z;

	vAABB[7].Pos.x=(*pBox).Min.x;
	vAABB[7].Pos.y=(*pBox).Min.y;
	vAABB[7].Pos.z=(*pBox).Max.z;

	vAABB[8].Pos.x=(*pBox).Min.x;
	vAABB[8].Pos.y=(*pBox).Min.y;
	vAABB[8].Pos.z=(*pBox).Min.z;

	vAABB[9].Pos.x=(*pBox).Max.x;
	vAABB[9].Pos.y=(*pBox).Min.y;
	vAABB[9].Pos.z=(*pBox).Min.z;

	vAABB[10].Pos.x=(*pBox).Max.x;
	vAABB[10].Pos.y=(*pBox).Min.y;
	vAABB[10].Pos.z=(*pBox).Max.z;

	vAABB[11].Pos.x=(*pBox).Max.x;
	vAABB[11].Pos.y=(*pBox).Max.y;
	vAABB[11].Pos.z=(*pBox).Max.z;

	vAABB[12].Pos.x=(*pBox).Max.x;
	vAABB[12].Pos.y=(*pBox).Max.y;
	vAABB[12].Pos.z=(*pBox).Min.z;

	vAABB[13].Pos.x=(*pBox).Min.x;
	vAABB[13].Pos.y=(*pBox).Max.y;
	vAABB[13].Pos.z=(*pBox).Min.z;

	vAABB[14].Pos.x=(*pBox).Min.x;
	vAABB[14].Pos.y=(*pBox).Max.y;
	vAABB[14].Pos.z=(*pBox).Max.z;

	vAABB[15].Pos.x=(*pBox).Min.x;
	vAABB[15].Pos.y=(*pBox).Min.y;
	vAABB[15].Pos.z=(*pBox).Max.z;

	//Drawing AABBs is always for debug purposes, so we'll just draw a series 
	//of lines. Not as efficient as possible, but reduces the number of places 
	//where DrawPrimitive happens.
	for( eg_uint i=0; i<(countof(vAABB)-1); i++ )
	{
		DrawLine( &vAABB[i].Pos , &vAABB[i+1].Pos , eg_color32(vAABB[i].Color0) );
	}
}

void EGD11R_Renderer::DrawRawTris( const egv_vert_simple* AllVerts , eg_uint TotalTris )
{
	CommitRenderStates();
	assert(m_bIsDrawing);
	//All DrawTris called should be within the programmable pipeline.
	assert_pointer(m_pFXCur);
	const EGD11R_Mtrl* pE = m_pFXCur;
	if(!pE)return;
	//Copy vertexes into vertex buffer:

	const eg_uint NumPasses = ((TotalTris*3)/EGD11R_BufferMgr::MAX_RAWTRI_VERTS) + 1;
	const eg_uint TrisPerPass = EGD11R_BufferMgr::MAX_RAWTRI_VERTS/3;
	const eg_uint LastPassTris = TotalTris - TrisPerPass*(NumPasses-1);

	for( eg_uint i=0; i<NumPasses; i++ )
	{
		const eg_uint FirstVert = EGD11R_BufferMgr::MAX_RAWTRI_VERTS*i;
		const eg_uint NumTris = (i == (NumPasses-1)) ? LastPassTris : TrisPerPass;

		eg_size_t Size = sizeof(AllVerts[0])*3*NumTris;
		m_BufferMgr->UpdateSimpleVB( &AllVerts[FirstVert] , Size );

		Consts_CopyToDevice();
		SetVertexLayout( eg_crc("egv_vert_simple") );
		m_Context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		//Render by shader:
		pE->Begin( m_Context );
		m_Context->Draw( NumTris*3 , 0 );
		pE->End( m_Context );
	}
}

void EGD11R_Renderer::DrawTris(egv_vbuffer VertexBuffer , eg_uint nFirstV, eg_uint nNumTris)
{
	CommitRenderStates();
	//The buffer is the relative address of the mem chunk, so we can just
	//pass it in as the offset in bytes.
	assert( 0 == (VertexBuffer.IntV1%VertexBuffer.IntV2) );
	eg_uint CurVert = static_cast<eg_uint>(VertexBuffer.IntV1/VertexBuffer.IntV2);
	
	assert(m_bIsDrawing);
	//All DrawTris called should be within the programmable pipeline.
	assert_pointer(m_pFXCur);
	const EGD11R_Mtrl* pE = m_pFXCur;//m_pFXDefault;
	if(!pE)return;

	Consts_CopyToDevice();
	SetVertexLayout( eg_string_crc( VertexBuffer.IntV3 ) );
	m_Context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	pE->Begin( m_Context );
	m_Context->Draw( nNumTris*3 , CurVert+nFirstV );
	pE->End( m_Context );
}


void EGD11R_Renderer::DrawIndexedTris(egv_vbuffer VertexBuffer, egv_ibuffer IndexBuffer, eg_uint nFirstI, eg_uint nNumVerts, eg_uint nNumTris)
{
	unused( nNumVerts );

	CommitRenderStates();

	//The buffer is the relative address of the mem chunk, so we can just
	//pass it in as the offset in bytes.
	assert( 0 == (VertexBuffer.IntV1%VertexBuffer.IntV2) );
	eg_uint CurVert = static_cast<eg_uint>(VertexBuffer.IntV1/VertexBuffer.IntV2);

	//The buffer is the relative address of the mem chunk, so we can just
	//pass it in as the offset in bytes.
	assert( 0 == (IndexBuffer%sizeof(egv_index)) );
	eg_uint CurIndx = static_cast<eg_uint>(IndexBuffer/sizeof(egv_index));

	assert(m_bIsDrawing);
	//All DrawTris called should be within the programmable pipeline.
	assert_pointer(m_pFXCur);
	const EGD11R_Mtrl* pE = m_pFXCur;
	if(!pE)return;

	Consts_CopyToDevice();
	SetVertexLayout( eg_string_crc( VertexBuffer.IntV3 ) );
	m_Context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	pE->Begin( m_Context );
	m_Context->DrawIndexed( nNumTris*3 , CurIndx+nFirstI , CurVert );
	pE->End( m_Context );
}

void EGD11R_Renderer::DrawBasicQuad()
{
	SetQuadColor( eg_color(1,1,1,1) );

	DrawRawTris(m_Quad, 2);
}

void EGD11R_Renderer::DrawBasicQuad(const eg_color& Color)
{
	SetQuadColor( Color );

	DrawRawTris(m_Quad, 2);
}

void EGD11R_Renderer::DrawShadowVolume(const eg_color& clr)
{
	SetQuadColor( clr );

	m_CurBlendState.Push( eg_blend_s::BLEND_DEFAULT_COLOR_ALL );
	m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF_STEST_NONZERO );
	m_CurDefaultShader.Push( eg_defaultshader_t::COLOR );

	SetWorldTF(eg_mat::I);
	SetViewTF(eg_mat::I);
	SetProjTF(eg_mat::I);
	DrawRawTris(m_Quad, 2);

	m_Context->ClearDepthStencilView( m_DsCur->GetDepthStencilView() , D3D11_CLEAR_STENCIL , 0.f , static_cast<UINT8>(0) );

	m_CurBlendState.Pop();
	m_CurDefaultShader.Pop();
	m_CurDepthStencilState.Pop();
}

void EGD11R_Renderer::SetClipPlane(const eg_plane* pplnClipW)
{
	if(pplnClipW)
	{
		m_bClipPlaneOn = true;
		m_plnClip      = *pplnClipW;
		m_plnClip.Normalize();
		eg_mat mT1 = m_mV * m_mP;
		mT1 = eg_mat::BuildInverse( mT1 , nullptr );
		mT1 = eg_mat::BuildTranspose( mT1 );	
		m_plnClipVP = m_plnClip * mT1;
	}
	else
	{
		m_bClipPlaneOn = false;
	}
}

void EGD11R_Renderer::ClearRT( eg_color Color )
{
	FLOAT ColorF[] = { Color.r , Color.g, Color.b, Color.a };
	m_Context->ClearRenderTargetView( m_RTs[m_RtCur]->GetRenderTargetView() , ColorF );
}

void EGD11R_Renderer::ClearDS( eg_real Depth, eg_uint Stencil )
{
	assert( m_DsCur );
	if( m_DsCur )
	{
		m_Context->ClearDepthStencilView( m_DsCur->GetDepthStencilView() , D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL , Depth , static_cast<UINT8>(Stencil) );
	}
}

void EGD11R_Renderer::BeginClip()
{
	if( !m_bClipPlaneOn )return;

	m_PreClipProj = m_mP;
	m_mP = EGMath_ClipProjectionMatrix( m_mP , m_plnClipVP );
	SetProjTF( m_mP );
}

void EGD11R_Renderer::EndClip()
{
	if( !m_bClipPlaneOn )return;

	SetProjTF( m_PreClipProj );
}

void EGD11R_Renderer::CommitRenderStates()
{
	assert( m_CurRasterizerState.HasItems() );
	assert( m_CurSamplerState.HasItems() );
	assert( m_CurBlendState.HasItems() );
	assert( m_CurDepthStencilState.HasItems() );
	assert( m_CurDefaultShader.HasItems() );

	if( m_bGlobalSamplersDirty )
	{
		m_bGlobalSamplersDirty = false;

		// Global samplers go after material textures.
		m_Context->PSSetShaderResources( RENDERER_MAX_MULTI_TEXTURE , countof(m_GlobalSamplers) , m_GlobalSamplers );
	}

	if( m_CurRasterizerState.HasItems() && m_CurRasterizerState.Top() != m_LastRasterizerState )
	{
		m_LastRasterizerState = m_CurRasterizerState.Top();
		m_Context->RSSetState( m_RasterizerStates[static_cast<eg_uint>(m_LastRasterizerState)] );
	}

	if( m_CurSamplerState.HasItems() && m_CurSamplerState.Top() != m_LastSamplerState )
	{
		m_LastSamplerState = m_CurSamplerState.Top();
		m_Context->PSSetSamplers( 0 , 1 , &m_SamplerStates[static_cast<eg_uint>(m_LastSamplerState)] );
	}

	if( m_CurBlendState.HasItems() && m_CurBlendState.Top() != m_LastBlendState )
	{
		m_LastBlendState = m_CurBlendState.Top();
		m_Context->OMSetBlendState( m_BlendStates[static_cast<eg_uint>(m_LastBlendState)] , NULL , 0xFFFFFFFF );
	}

	if( m_CurDepthStencilState.HasItems() && m_CurDepthStencilState.Top() != m_LastDepthStencilState )
	{
		m_LastDepthStencilState = m_CurDepthStencilState.Top();
		m_Context->OMSetDepthStencilState( m_DepthStencilStates[static_cast<eg_uint>(m_LastDepthStencilState)] , m_DepthStencilRef[static_cast<eg_uint>(m_LastDepthStencilState)] );
	}

	eg_bool bDefShaderChanged = false;
	if( m_CurDefaultShader.HasItems() && m_CurDefaultShader.Top() != m_LastDefaultShaderType )
	{
		bDefShaderChanged = true;
		m_LastDefaultShaderType = m_CurDefaultShader.Top();

		switch( m_LastDefaultShaderType )
		{
			case eg_defaultshader_t::TEXTURE:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_DEFAULT] );
				break;
			case eg_defaultshader_t::MESH_LIT: 
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_MESH_LIT] );
				break;
			case eg_defaultshader_t::MESH_UNLIT: 
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_MESH_UNLIT] );
				break;
			case eg_defaultshader_t::MAP: 
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_MAP] );
				break;
			case eg_defaultshader_t::SHADOW: 
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_SHADOW] );
				break;
			case eg_defaultshader_t::FONT:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_FONT] ); 
				break;
			case eg_defaultshader_t::FONT_SHADOW:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_FONT_SHADOW] );
				break;
			case eg_defaultshader_t::FONT_GLYPH:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_FONT_GLYPH] );
				break;
			case eg_defaultshader_t::FONT_GLYPH_SHADOW:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_FONT_GLYPH_SHADOW] );
				break;
			case eg_defaultshader_t::COLOR:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_COLOR] ); 
				break;
			case eg_defaultshader_t::TERRAIN:
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_TERRAIN] ); 
				break;
			case eg_defaultshader_t::STORE_DEPTH: 
				m_pFXState = m_MtrlMgr.GetMaterial( m_DefShaders[ISHADER_STOREDEPTH] );
				break;
			case eg_defaultshader_t::DRAW_TO_DEPTH: break;
			case eg_defaultshader_t::CLEAR_DEAPTH: break;
			case eg_defaultshader_t::FXAA: break;
			case eg_defaultshader_t::COPY_BUFFER: break;
			case eg_defaultshader_t::MOTION_BLUR_1: break;
			case eg_defaultshader_t::COUNT: assert(false); break;
		}
	}

	if( bDefShaderChanged || m_CurMaterial != m_LastMaterial || m_CurMaterial == EGV_MATERIAL_NULL )
	{
		m_LastMaterial = m_CurMaterial;

		const EGD11R_Mtrl* pMtrl = m_MtrlMgr.GetMaterial( m_LastMaterial );
		ID3D11ShaderResourceView* Textures[4];

		if(EGV_MATERIAL_NULL != m_LastMaterial && pMtrl)
		{
			Textures[0] = nullptr != pMtrl->m_Tx[0]->View ? pMtrl->m_Tx[0]->View : m_pFXState->m_Tx[0]->View;
			Textures[1] = nullptr != pMtrl->m_Tx[1]->View ? pMtrl->m_Tx[1]->View : m_pFXState->m_Tx[1]->View;
			Textures[2] = nullptr != pMtrl->m_Tx[2]->View ? pMtrl->m_Tx[2]->View : m_pFXState->m_Tx[2]->View;
			Textures[3] = nullptr != pMtrl->m_Tx[3]->View ? pMtrl->m_Tx[3]->View : m_pFXState->m_Tx[3]->View;
			m_Context->PSSetShaderResources( 0 , countof(Textures) , Textures );

			m_pFXCur = pMtrl->HasShaders() ? pMtrl : m_pFXState;
			m_VSC.m_Material = pMtrl->m_D3DMtrl;
		}
		else
		{
			Textures[0] = nullptr != m_pFXState->m_Tx[0] ? m_pFXState->m_Tx[0]->View : nullptr;
			Textures[1] = nullptr != m_pFXState->m_Tx[1] ? m_pFXState->m_Tx[1]->View : nullptr;
			Textures[2] = nullptr != m_pFXState->m_Tx[2] ? m_pFXState->m_Tx[2]->View : nullptr;
			Textures[3] = nullptr != m_pFXState->m_Tx[3] ? m_pFXState->m_Tx[3]->View : nullptr;
			m_Context->PSSetShaderResources( 0 , countof(Textures) , Textures );
			m_pFXCur = m_pFXState;
			ResetMtrl();
		}

		if( m_CurMaterial == m_DefShaders[ISHADER_STOREDEPTH] )
		{
			ID3D11ShaderResourceView* ResView[] = { m_DepthBuffer->GetResourceView() };
			m_Context->PSSetShaderResources( 0 , countof(ResView) , ResView );
		}
	}
}

void EGD11R_Renderer::DrawDebugZOverlay( eg_real Near , eg_real Far , eg_real FudgeFactor )
{
	//Test draw z-buffer:
	//The register x value is the value at which the z-draw considers the
	//maximum distance away. Closer will get more contrast in color for
	//different z values. Further will show more of the visible screen.
	//No reason to make it any bigger than the far clip plane.
	SetVec4( eg_rv4_t::FREG_0, &eg_vec4( Near , Far , FudgeFactor , 0) );
	m_CurDefaultShader.Push( eg_defaultshader_t::TEXTURE );
	m_CurSamplerState.Push( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );
	SetMaterial( m_DefShaders[ISHADER_DRAWZBUFFER] );
	SetGlobalSamplers( egv_rtarget::DepthBuffer , egv_rtarget::Null , egv_rtarget::Null );
	DrawBasicQuad();
	m_CurDefaultShader.Pop();
	m_CurSamplerState.Pop();
}

egv_material EGD11R_Renderer::CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId )
{
	EGFunctionLock Lock( &m_AssetCreationLock );
	UpdateAssetState();

	egv_material mtrl = EGV_MATERIAL_NULL;

	mtrl = m_MtrlMgr.CreateMaterial( pMtrlDef , SharedId );

	return mtrl;
}

eg_uint EGD11R_Renderer::GetViewWidth()const
{
	return m_ip.ScreenWidth;
}

eg_uint EGD11R_Renderer::GetViewHeight()const
{
	return m_ip.ScreenHeight;
}

eg_real EGD11R_Renderer::GetAspectRatio()const
{
	return m_fAspect;	
}

eg_bool EGD11R_Renderer::IsWindowed()const
{
	return m_ip.ScreenMode == eg_screen_mode::WINDOWED;
}

#include "EGInputKbmDevice.h"
#include "EGEngine.h"

const eg_char16 EGD11R_Renderer::WINDOW_CLASS_NAME[] = L"EGD11R_Renderer";

void EGD11R_Renderer::Window_Init()
{
	HINSTANCE Inst = NULL;
	//We need a window class and a window, we use very simple constructs for
	//these as the 3D API will do most of the work.

	//The window class:
	WNDCLASSEXW wc;
	wc.cbSize=sizeof(wc);
	wc.style=CS_CLASSDC | CS_OWNDC;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.lpfnWndProc=Window_Proc;
	wc.hInstance=Inst;
	wc.hbrBackground=(HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.hIcon=LoadIconW(GetModuleHandleW(NULL), L"EGICON");
	wc.hIconSm=NULL;
	wc.hCursor=LoadCursorW(NULL, IDC_ARROW);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=WINDOW_CLASS_NAME;
	
	if(!RegisterClassExW(&wc))
	{
		MessageBoxW(	NULL , L"Failed to register window class." , EGString_ToWide(ENGINE_NAME) , MB_OK|MB_ICONERROR );
		return;
	}
	
	RECT rcDT;
	GetClientRect(GetDesktopWindow(), &rcDT);
	
	const DWORD nStartWidth=320;
	const DWORD nStartHeight=240;

	DWORD WindowStyleEx = WS_EX_APPWINDOW;
	DWORD WindowStyle = WINDOW_STYLE_WINDOWED;

	eg_d_string16 StrWindowTitle = Engine_GetGameTitle();
	if( IS_DEBUG )
	{
		StrWindowTitle.Append( L" [DX11]" );
	}

	//Create the Window:
	m_hwnd=CreateWindowExW(
		WindowStyleEx,
		WINDOW_CLASS_NAME,
		*StrWindowTitle,
		WindowStyle,
		(rcDT.right-rcDT.left)/2 - nStartWidth/2,
		(rcDT.bottom-rcDT.top)/2 - nStartHeight/2,
		nStartWidth,
		nStartHeight,
		GetDesktopWindow(),
		NULL,
		Inst,
		this);
	
	if(!m_hwnd)
	{
		MessageBoxW( nullptr , L"Failed to create application window." , EGStringConvert<eg_char8,eg_char16>(ENGINE_NAME) , MB_OK|MB_ICONERROR );

		return;
	}

	m_OriginalWindowsStyleEx = GetWindowLongW( m_hwnd , GWL_EXSTYLE );
	m_OriginalWindowsStyle = GetWindowLongW( m_hwnd , GWL_STYLE );

	ShowWindow( m_hwnd , SW_SHOWNORMAL );
	UpdateWindow( m_hwnd );
	SetActiveWindow( m_hwnd );

	InputKbmDevice_WindowThread_Init( m_hwnd );
}

void EGD11R_Renderer::Window_Deinit()
{
	if( m_bIsChildWnd )
	{
		m_hwnd = nullptr;
	}
	else
	{
		HINSTANCE Inst = NULL;

		InputKbmDevice_WindowThread_Deinit( m_hwnd );

		DestroyWindow( m_hwnd );
		BOOL Success = UnregisterClassW( WINDOW_CLASS_NAME , Inst );
		assert( Success );
		m_hwnd = nullptr;
	}
}

void EGD11R_Renderer::Window_Update( eg_real DeltaTime )
{
	if( !m_bIsChildWnd )
	{
		MSG msg;
		zero(&msg);	
		InputKbmDevice_WindowThread_Update( DeltaTime , m_hwnd );
		while(PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}

LRESULT CALLBACK EGD11R_Renderer::Window_Proc( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam )
{
	switch( uMsg )
	{
		case WM_CREATE:
		{
			CREATESTRUCT* Cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			EGD11R_Renderer* _this = reinterpret_cast<EGD11R_Renderer*>(Cs->lpCreateParams);
			SetWindowLongPtr( hwnd , GWLP_USERDATA , reinterpret_cast<LONG_PTR>(_this) );
		} break;
		case WM_MOVE:
		case WM_SIZE:
		{
			WindowThread_HandleWindowPositionOrSizeChanged();
		} break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if( wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN) != 0 )
			{
				bool bEnableScreenSwitch = true;

				if( bEnableScreenSwitch )
				{
					VideoConfig_IsWindowed.SetValue( !VideoConfig_IsWindowed.GetValueThreadSafe() );
				
					EGD11R_Renderer* _this = reinterpret_cast<EGD11R_Renderer*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
					if( _this )
					{
						_this->Reset();
					}
				}

				return DefWindowProcW( hwnd , uMsg , wParam , lParam );
			}

			if( wParam == VK_F4 && (HIWORD(lParam) & KF_ALTDOWN) != 0 )
			{
				PostMessageW( hwnd , WM_CLOSE , 0 , 0 );
				return 0;
			}
		} // fallthru
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_CHAR:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:
		case WM_INPUT:
		case WM_CANCELMODE:
		case WM_CAPTURECHANGED:
		{
			InputKbmDevice_WindowThread_HandleMsg( hwnd , uMsg , wParam , lParam );
		} break;

		case WM_CLOSE:
		{
			Engine_QueMsg( "engine.quit()" );
		} break;

		case WM_ACTIVATEAPP:
		{
			EGD11R_Renderer* _this = reinterpret_cast<EGD11R_Renderer*>(GetWindowLongPtr( hwnd , GWLP_USERDATA ));
			_this->m_bAppIsActive = wParam != 0;
			if( _this->m_ip.ScreenMode == eg_screen_mode::FULL_SCREEN )
			{
				if( _this->m_bAppIsActive )
				{
					SetForegroundWindow( _this->m_hwnd );
					ShowWindow( _this->m_hwnd , SW_RESTORE );
					if( _this->m_SwapChain )
					{
						// _this->Window_Update( 0.f ); // Pump messages.
						_this->m_SwapChain->SetFullscreenState( TRUE , nullptr );
					}
				}
				else
				{
					if( _this->m_SwapChain )
					{
						_this->m_SwapChain->SetFullscreenState( FALSE , nullptr );
						ShowWindow( _this->m_hwnd , SW_SHOWMINIMIZED );
					}
				}
			}
			else if( _this->m_ip.ScreenMode == eg_screen_mode::BORDERLESS )
			{
				if( _this->m_bAppIsActive )
				{
					SetForegroundWindow( _this->m_hwnd );
					ShowWindow( _this->m_hwnd , SW_RESTORE );
				}
				else
				{
					ShowWindow( _this->m_hwnd , SW_SHOWMINIMIZED );
				}
			}
			InputKbmDevice_WindowThread_HandleMsg( hwnd , uMsg , wParam , lParam );
		} break;
		case WM_ACTIVATE:
		{

			InputKbmDevice_WindowThread_HandleMsg( hwnd , uMsg , wParam , lParam );
		} break;

		case WM_SETCURSOR:
		{
			const UINT HitTest = LOWORD(lParam);
			const UINT Event = HIWORD(lParam);
			if( HitTest == HTCLIENT )
			{
				SetCursor( NULL );
				return TRUE;
			}
		} return DefWindowProcW( hwnd , uMsg , wParam , lParam );

		/*
		case WM_SYSCOMMAND:
		{
			if( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
			{
				return 0;
			}
			return DefWindowProcW( hwnd , uMsg , wParam , lParam );
		} break;
		*/

		default: return DefWindowProcW( hwnd , uMsg , wParam , lParam );
	}

	return 0;
}

