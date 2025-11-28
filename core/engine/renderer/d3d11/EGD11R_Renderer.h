// (c) 2017 Beem Media

#pragma once

#include "EGBaseR_Core.h"
#include "EGD11R_ShaderMgr.h"
#include "EGD11R_MtrlMgr2.h"
#include "EGStringMap.h"
#include "EGHeap2.h"
#include "EGMutex.h"
#include "EGD11R_Texture.h"
#include "EGD11R_RenderTarget.h"
#include "EGDirectXAPI.h"

class EGD11R_AllocBuffer;
class EGD11R_BufferMgr;

class EGD11R_Renderer: public EGBaseR_Core
{
	EG_CLASS_BODY( EGD11R_Renderer , EGBaseR_Core )

	EGD11R_Renderer();
	~EGD11R_Renderer();

private:

	virtual eg_bool InitRenderer( void* Context ) override;

	virtual void Reset() override;
	
	virtual egRendererSpecs GetSpecs()const override
	{ 
		egRendererSpecs Specs;
		Specs.bDrawZScene = m_bManualDrawZ;
		Specs.bDrawVolumeShadows = true; // Maybe should only be set on setting....
		return Specs; 
	}

	virtual void Update_RenderThread( eg_real DeltaTime ) override;

	virtual void OnToolsWindowResized( eg_uint Width , eg_uint Height ) override final;

	/////////////////
	// Asset Creation
	/////////////////
	virtual egv_material CreateMaterial( const EGMaterialDef* pMtrlDef , eg_cpstr SharedId ) override final;
	virtual egv_vbuffer  CreateVB( eg_string_crc VertexType , eg_uint nNumVerts , const void* pVerts ) override final;
	virtual egv_ibuffer  CreateIB(eg_uint nNumInds, const egv_index* pIndices) override final;
	virtual egv_rtarget  CreateRenderTarget( eg_uint Width , eg_uint Height ) override final;
	virtual EGVDynamicBuffer* CreateDynamicBuffer( eg_string_crc VertexType , eg_uint VertexBufferSize , eg_uint IndexBufferSize ) override final;
	virtual void DestroyDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) override final;
	virtual void UpdateDynamicBuffer( EGVDynamicBuffer* DynamicBuffer ) override final;
	virtual void HandleDeleteAsset( const egDestroyItem& DestroyItem ) override final;

	//The ViewWidth and height can be obtained, this is important for some
	//drawing functions, and projection matrices.
	virtual eg_uint GetViewWidth()const override;
	virtual eg_uint GetViewHeight()const override;
	virtual eg_real GetAspectRatio()const override; //Width as a percentage of height, height is 1.0f.
	virtual eg_bool IsWindowed()const override;

private:

	static const DWORD WINDOW_STYLE_WINDOWED = WS_OVERLAPPED|WS_SYSMENU|WS_CAPTION;
	static const DWORD WINDOW_STYLE_BORDERLESS = WS_POPUP;

	struct egInitParms
	{
		eg_uint ScreenWidth;
		eg_uint ScreenHeight;
		eg_uint BufferWidth;
		eg_uint BufferHeight;
		eg_screen_mode ScreenMode:4;
		eg_bool VSync:1;
	};

	struct egDepthInfo
	{
		eg_mat mPrevView;
		eg_mat mPrevProj;
		eg_mat mCurView;
		eg_mat mCurProj;
	};

	enum class VSB_T
	{
		BASE,
		BONES,
		IBONE,

		COUNT,
	};

	enum ISHADER_T
	{
		ISHADER_DEFAULT,
		ISHADER_MESH_LIT,
		ISHADER_MESH_UNLIT,
		ISHADER_MAP,
		ISHADER_SHADOW,
		ISHADER_FONT,
		ISHADER_FONT_SHADOW,
		ISHADER_FONT_GLYPH,
		ISHADER_FONT_GLYPH_SHADOW,
		ISHADER_COLOR,
		ISHADER_TERRAIN,
		ISHADER_STOREDEPTH,
		ISHADER_DRAWTODEPTH,
		ISHADER_CLEARDEPTH,
		ISHADER_FXAA,
		ISHADER_COPY,
		ISHADER_MOTIONBLUR1,
		ISHADER_SPLASH,
		ISHADER_DRAWZBUFFER,

		ISHADER_COUNT,
	};

private:

	//////////
	// Drawing
	//////////
	//Render target functions:
	void SetRT(RENDERER_FSRT_TYPE Target, eg_bool bNoDs=false);

	void SetScreenViewport( const egScreenViewport& Vp );
	void ApplyScreenViewport();
	void SetViewportToRT();
	/*__declspec(deprecated)*/ void SetClipPlane(const eg_plane* pplnClipW);//Set's a temporary clip plane (for mirrors) (null to disable).
	void ClearRT( eg_color Color );
	void ClearDS( eg_real Depth , eg_uint Stencil );

	//Basic drawing functions:
	void DrawBasicQuad();
	void DrawBasicQuad(const eg_color& Color);
	void DrawAABB(const eg_aabb* pBox, eg_color32 color);
	void DrawLine(const eg_vec4* pvStart, const eg_vec4* pvEnd, eg_color32 color);
	void DrawRawTris(const egv_vert_simple* pVerts, eg_uint nNumTris);
	void DrawTris(egv_vbuffer VertexBuffer, eg_uint nFirstV, eg_uint nNumTris);
	void DrawIndexedTris(egv_vbuffer VertexBuffer, egv_ibuffer IndexBuffer, eg_uint nFirstI, eg_uint nNumVerts, eg_uint nNumTris);

	void DrawShadowVolume(const eg_color& color);
	
	void ResolveToRenderTarget( egv_rtarget RenderTarget );
	void SetGlobalSamplers( egv_rtarget Sampler0 , egv_rtarget Sampler1 , egv_rtarget Sampler2 );
	void SetGlobalSamplers( RENDERER_FSRT_TYPE Sampler0 , RENDERER_FSRT_TYPE Sampler1 , RENDERER_FSRT_TYPE Sampler2 );

	void BeginMainScene( eg_flags Flags );
	void DoSceneProc( eg_flags Flags );
	void RunFullScreenProc( egv_material Material );
	void EndMainScene( eg_flags Flags );

	eg_bool Reset_Internal();

	void DrawSplash();
	void ResetScreenViewport();
	void CommitRenderStates();

	void DrawDebugZOverlay( eg_real Near , eg_real Far , eg_real FudgeFactor );

private:
	
	EGD11R_BufferMgr* m_BufferMgr = nullptr;

	eg_bool m_ResetRequested;
	const eg_real m_ResetCountDownTime = .25f;
	eg_real m_ResetTimer = 0.f;

	ID3D11Device*           m_Device;
	ID3D11DeviceContext*    m_Context;
	IDXGISwapChain*         m_SwapChain;
	EGD11R_RenderTarget*    m_BackBuffer;
	EGD11R_RenderTarget*    m_DepthBuffer;
	EGD11R_RenderTarget*    m_RTs[FSRT_COUNT];
	RENDERER_FSRT_TYPE      m_RtCur;
	EGD11R_RenderTarget*    m_DsCur;

	ID3D11RasterizerState*   m_RasterizerStates[enum_count(eg_rasterizer_s)];
	ID3D11SamplerState*      m_SamplerStates[enum_count(eg_sampler_s)];
	ID3D11BlendState*        m_BlendStates[enum_count(eg_blend_s)];
	ID3D11DepthStencilState* m_DepthStencilStates[enum_count(eg_depthstencil_s)];
	eg_uint8                 m_DepthStencilRef[enum_count(eg_depthstencil_s)];

	egInitParms             m_ip;
	D3D_FEATURE_LEVEL       m_FeatureLevel;
	HWND                    m_hwnd;
	DWORD                   m_OriginalWindowsStyleEx = 0;
	DWORD                   m_OriginalWindowsStyle = 0;
	eg_bool                 m_bIsChildWnd:1;

	//Clip plane
	eg_plane m_plnClip;    //For fixed-function pipeline.
	eg_plane m_plnClipVP;  //For programmable pipeline.
	eg_bool  m_bClipPlaneOn;
	eg_mat   m_PreClipProj;

	//Built in rasterization features:
	EGD11R_ShaderMgr           m_ShaderMgr;
	EGD11R_MtrlMgr2            m_MtrlMgr;
	EGArray<EGVDynamicBuffer*> m_DynBufferUpdateList = eg_mem_pool::RenderResource;

	//SHADER DATA
	//This data is used to determine the current shader:
	const EGD11R_Mtrl* m_pFXState; //The current state's default render effect.
	const EGD11R_Mtrl* m_pFXCur;     //The active render effect.

	//Built in shaders:
	egv_material m_DefShaders[ISHADER_COUNT];

	//Unique internal ID, this number is used when creating internal data that
	//the resource manager should load only once, this number is incremented
	//every time it is used to insure that each internal object has a unique id.
	eg_uint m_nInternalID;

	ID3D11Buffer*   m_VsConstsBuffers[static_cast<eg_size_t>(VSB_T::COUNT)];
	ID3D11Buffer*   m_PsConstsBuffer;

	ID3D11ShaderResourceView* m_GlobalSamplers[RENDERER_MAX_GLOBAL_SAMPLER];
	eg_bool                   m_bGlobalSamplersDirty = true;

	eg_real m_fAspect;

	eg_uint m_nVBs;
	eg_uint m_nIBs;

	egDepthInfo m_DepthBufferInfo;

	egScreenViewport           m_ScreenViewport;

	RENDERER_FSRT_TYPE         m_MainSceneRT:8;
	eg_bool                    m_bManualDrawZ:1; // When true the z-buffer is drawn by the game instead of being stored.
	RENDERER_TEX_FILTER_T      m_TextureFilter;
	eg_string_crc              m_LastLayoutType = CT_Clear;

	eg_bool m_bInMainScene = false;
	eg_bool m_bHasDepth = false;

	eg_real m_MotionBlurSkipDistSq = 5.f*5.f;
	eg_bool m_bAppIsActive = false;

	static const eg_char16 WINDOW_CLASS_NAME[];

private:

	void Init( HWND hwndParent );
	void Init_Device();
	IDXGIAdapter* Init_Device_SelectAdapter();
	void Init_Device_AdjustRes( DXGI_MODE_DESC* ModeOut , bool bWantsBufferRes );
	void Init_Mgrs();
	void Init_Shaders(eg_bool bInit);
	void Init_RenderTargets(eg_bool bInit);
	eg_bool Init_VerifyDevCaps();
	void Validate();
	void Invalidate();
	void Validate_Buffers( eg_bool Validate );
	void Deinit();
	void Shutdown_Mgrs();

	void ProcessDisplayList( class EGDisplayList* DisplayList );

	void SetPPs();
	void SetBasicPPs();

	void Init_RS( eg_bool bInit );
	void Init_RS_GetTxFilter( RENDERER_TEX_FILTER_T type , D3D11_SAMPLER_DESC* Out );
	void Consts_CopyToDevice();
	void Update_RefreshDynamicBuffers( eg_real DeltaTime );
	void SetVertexLayout( eg_string_crc NewLayoutType );

	void BeginClip();
	void EndClip();

	void Window_Init();
	void Window_Deinit();
	void Window_Update( eg_real DeltaTime );
	static LRESULT CALLBACK Window_Proc( HWND hwnd , UINT uMsg , WPARAM wParam , LPARAM lParam );
};
