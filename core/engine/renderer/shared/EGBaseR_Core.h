// (c) 2016 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGRendererInterface.h"
#include "EGDisplayList.h"
#include "EGStringMap.h"
#include "EGEngineConfig.h"

class EGBaseR_Core: public EGRendererInterface
{
	EG_ABSTRACT_CLASS_BODY( EGBaseR_Core , EGRendererInterface )

protected:

	enum RENDERER_FSRT_TYPE
	{
		FSRT_SCREEN , //The actual back buffer.
		FSRT_FS1    , //A screen buffer the size of FSRT_SCREEN (shares FSRT_SCREEN depth-stencil).
		FSRT_FS2    , //"                                                                                         "
		FSRT_Z      , //A screen buffer the size of FSRT, but of type D3D_BB_FMT_Z (shares the FSRT_SCREEN depth stencil).
		FSRT_REFLECT,

		FSRT_COUNT  ,
	};

public:

	EGBaseR_Core()
	: m_DlAvailable(CT_Default)
	, m_Resolutions(CT_Clear,0)
	, m_DlMainThread(nullptr)
	, m_DlNext(nullptr)
	, m_DlDraw(nullptr)
	, m_LastRasterizerState(eg_rasterizer_s::COUNT)
	, m_LastSamplerState(eg_sampler_s::COUNT)
	, m_LastBlendState(eg_blend_s::COUNT)
	, m_LastDepthStencilState(eg_depthstencil_s::COUNT)
	, m_LastDefaultShaderType(eg_defaultshader_t::COUNT)
	, m_LastMaterial(EGV_MATERIAL_NULL)
	, m_CurMaterial(EGV_MATERIAL_NULL)
	, m_DrawSplash(false)
	, m_bUseMtrlOverride(false)
	, m_bIsDrawing(false)
	, m_bIsDrawingMainScene( false )
	, m_bIsDrawingZScene( false )
	, m_AssetState(0)
	, m_InitStatus( INIT_NOT_INITIALIZED )
	, m_mW(eg_mat::I)
	, m_mV(eg_mat::I)
	, m_mP(eg_mat::I)
	, m_mWVP(eg_mat::I)
	, m_mWV(eg_mat::I)
	, m_mVP(eg_mat::I)
	, m_NumBones(0)
	, m_VSC()
	, m_PSC()
	, m_DestroyQue( 1 ) // Id of 1 cuz the temp queue has id 2
	{
		zero(&m_Quad);
		m_Quad[0].Pos  = eg_vec4(-1, 1, 0, 1);
		m_Quad[0].Tex0 = eg_vec2(0, 0);
		m_Quad[1].Pos  = eg_vec4(1, 1, 0, 1);
		m_Quad[1].Tex0 = eg_vec2(1, 0);
		m_Quad[2].Pos  = eg_vec4(-1, -1, 0, 1);
		m_Quad[2].Tex0 = eg_vec2(0, 1);

		m_Quad[3].Pos  = eg_vec4(1, 1, 0, 1);
		m_Quad[3].Tex0 = eg_vec2(1, 0);
		m_Quad[4].Pos  = eg_vec4(1, -1, 0, 1);
		m_Quad[4].Tex0 = eg_vec2(1, 1);
		m_Quad[5].Pos  = eg_vec4(-1, -1, 0, 1);
		m_Quad[5].Tex0 = eg_vec2(0, 1);


		for( eg_uint i=0; i<countof(m_DisplayList); i++ )
		{
			m_DisplayList[i].Mem = EGMem2_Alloc( DISPLAY_LIST_BUFFER_SIZE , eg_mem_pool::System );
			m_DisplayList[i].MemSize = DISPLAY_LIST_BUFFER_SIZE;
			m_DlAvailable.Push( &m_DisplayList[i] );
		}
	}

	~EGBaseR_Core()
	{
		for( eg_uint i=0; i<countof(m_DisplayList); i++ )
		{
			EGMem2_Free( m_DisplayList[i].Mem );
			m_DisplayList[i].Mem = nullptr;
			m_DisplayList[i].MemSize = 0;
		}
	}

//
// Types and Consts
//
public:

	static const eg_char SPLASH_TEX_FILE[];

	enum INIT_S
	{
		INIT_NOT_INITIALIZED,
		INIT_IN_PROGRESS,
		INIT_SUCCEEDED,
		INIT_FAILED,
	}; 

	struct egDisplayListData
	{
		EGDisplayList List;
		void*         Mem;
		eg_size_t     MemSize;

		egDisplayListData(): List() , Mem(nullptr) , MemSize(0){ }
	};

	static const eg_uint   DISPLAY_LIST_BUFFER_COUNT = 3;
	static const eg_size_t DISPLAY_LIST_BUFFER_SIZE  = EG_DISPLAY_LIST_MEGS*1024*1024;
	typedef EGFixedArray<egDisplayListData*,DISPLAY_LIST_BUFFER_COUNT> EGDlStack;

	static const eg_uint STATE_STACK_DEPTH=10;
	typedef EGFixedArray<eg_rasterizer_s   ,STATE_STACK_DEPTH>	EGRsStack;
	typedef EGFixedArray<eg_sampler_s      ,STATE_STACK_DEPTH>	EGSsStack;
	typedef EGFixedArray<eg_blend_s        ,STATE_STACK_DEPTH>	EGBsStack;
	typedef EGFixedArray<eg_depthstencil_s ,STATE_STACK_DEPTH>	EGDsStack;
	typedef EGFixedArray<eg_defaultshader_t,STATE_STACK_DEPTH>	EGShStack;

	typedef EGFixedItemMap<eg_uint,eg_uint32,32>                EGResList;

	enum class eg_asset_t
	{
		MATERIAL,
		VB,
		IB,
		RT,
	};

	struct egDestroyItem: IListable
	{
		eg_asset_t Type;
		eg_uint    LastAssetStateUsed;
		union
		{
			egv_material Material;
			egv_vbuffer  VBuffer;
			egv_ibuffer  IBuffer;
			egv_rtarget  RTarget;
		};
	};

	typedef EGList<egDestroyItem> EGDestroyQue;

	EG_ALIGN struct egRVSConsts
	{
	public:
		egRVSConsts(){zero(this);}
	public:
		#define VS_CONSTS
		#define IN_GAME_DECL
		#include "EGShaderConsts.inc"
		#undef IN_GAME_DECL
		#undef VS_CONSTS

		static const eg_size_t REG_SZ_BASE  = 56;
		static const eg_size_t REG_SZ_BONES = (RENDERER_MAX_BONES+1)*4;
		static const eg_size_t REG_SZ_TOTAL = REG_SZ_BASE + REG_SZ_BONES;
		static const eg_size_t SZ_BASE      = REG_SZ_BASE*sizeof(eg_vec4);
		static const eg_size_t SZ_BONES     = REG_SZ_BONES*sizeof(eg_vec4);
		static const eg_size_t SZ_TOTAL     = (REG_SZ_BASE + REG_SZ_BONES)*sizeof(eg_vec4);
	};

	EG_ALIGN struct egRPSConsts
	{
	public:
		egRPSConsts(){zero(this);}
	public:			 											 							  
		#define PS_CONSTS
		#define IN_GAME_DECL
		#include "EGShaderConsts.inc"
		#undef IN_GAME_DECL
		#undef PS_CONSTS
		static const eg_uint REG_SZ_ALL = 7+4+4;
		static const eg_uint DATA_SIZE  = REG_SZ_ALL*4*sizeof(eg_real);
	};

	static_assert(sizeof(egRVSConsts) == egRVSConsts::SZ_TOTAL , "Incorrect vertex shader const declaration." );
	static_assert(sizeof(egRPSConsts) == egRPSConsts::DATA_SIZE , "Incorrect pixel shader const declaration." );

//
// Methods:
//
public:

	virtual void SetDrawSplash( eg_bool DrawSplash ) override final { EGFunctionLock Lock( &m_DataLock ); m_DrawSplash = DrawSplash; }

	virtual EGDisplayList* BeginFrame_MainThread() override final;
	virtual void EndFrame_MainThread( EGDisplayList* DisplayList ) override final;
	virtual void GetResolutions( EGArray<eg_ivec2>& ResOut )const override final;

protected:

	void AddSupportedResolution( eg_uint Width , eg_uint Height )
	{
		eg_uint32 Size = (Height << 16) | Width; //This will cause the resolutions to be sorted by height first.
		m_Resolutions.Insert(Size, Size);
	}

	INIT_S GetInitStatus()const
	{
		if( nullptr == this )return INIT_NOT_INITIALIZED;

		EGFunctionLock Lock( &m_DataLock );
		INIT_S Status = m_InitStatus;
		return Status;
	}

	eg_bool IsInitialized()const{ return GetInitStatus() == INIT_SUCCEEDED; }

	void SetMaterial(const egv_material mtrl)
	{
		if(m_bUseMtrlOverride)return;

		m_CurMaterial = mtrl;
	}

	void SetMaterialOverride(const egv_material mtrl)
	{
		m_bUseMtrlOverride = false;
		SetMaterial(mtrl);
		if(EGV_MATERIAL_NULL != mtrl)
		{
			m_bUseMtrlOverride = true;
		}
	}

	void SetLight(eg_uint nLight, const EGLight* pLight)
	{
		//Most of the properties in m_Light are already set, we just
		//need to set these ones.
		//The w coordinate is used for the on off value. So we save the current
		//state of that.
		assert(nLight < RENDERER_MAX_LIGHTS);
		eg_bool bEnabled = m_VSC.m_Lights[nLight].IsEnabled();
		m_VSC.m_Lights[nLight] = *pLight;
		m_VSC.m_Lights[nLight].SetEnabled( bEnabled );
		//Note the direction of the light should already be normalized.
	}

	void EnableLight(eg_uint nLight, eg_bool bEnable)
	{
		assert(nLight < RENDERER_MAX_LIGHTS);
		m_VSC.m_Lights[nLight].SetEnabled( bEnable );
	}

	void DisableAllLights()
	{
		for(eg_uint i=0; i<RENDERER_MAX_LIGHTS; i++)
		{
			EnableLight(i, false);
		}
	}

	void HandleCommonDisplayListCmd( const EGDisplayList::egCmdListItem* Cmd );

	eg_uint GetAssetState()const
	{
		//Every time an asset is created or destroyed, the state changes, that way 
		//display lists that were created with a different asset state can't be 
		//drawn (since they'd potentially reference assets that no longer exist).

		EGFunctionLock Lock( &m_AssetStateLock );

		eg_uint AssetState = m_AssetState;

		return AssetState;
	}

	void UpdateAssetState()
	{
		EGFunctionLock Lock( &m_AssetStateLock );
		m_AssetState++;
	}

	void ResetMtrl()
	{
		m_VSC.m_Material.Diffuse.r = 1.0f;
		m_VSC.m_Material.Diffuse.g = 1.0f;
		m_VSC.m_Material.Diffuse.b = 1.0f;
		m_VSC.m_Material.Diffuse.a = 1.0f;

		m_VSC.m_Material.Ambient.r = 1.0f;
		m_VSC.m_Material.Ambient.g = 1.0f;
		m_VSC.m_Material.Ambient.b = 1.0f;
		m_VSC.m_Material.Ambient.a = 1.0f;

		m_VSC.m_Material.Specular.r = 0;
		m_VSC.m_Material.Specular.g = 0;
		m_VSC.m_Material.Specular.b = 0;
		m_VSC.m_Material.Specular.a = 0;

		m_VSC.m_Material.Emissive.r = 0;
		m_VSC.m_Material.Emissive.g = 0;
		m_VSC.m_Material.Emissive.b = 0;
		m_VSC.m_Material.Emissive.a = 0;

		m_VSC.m_Material.Power = 1.0f;
	}

	void SetWorldTF(const eg_mat& pMat)
	{
		//When the world matrix is set we need to update the
		//W, WV, and WVP matrices.
		m_mW   = pMat;
		m_mWV  = m_mW*m_mV;
		m_mWVP = m_mWV*m_mP;

		m_VSC.m_mW = eg_mat::BuildTranspose(m_mW);
		m_VSC.m_mWV = eg_mat::BuildTranspose(m_mWV);
		m_VSC.m_mWVP = eg_mat::BuildTranspose(m_mWVP);
	}

	void SetViewTF(const eg_mat& pMat)
	{
		assert( !m_bIsDrawingMainScene && !m_bIsDrawingZScene ); // The view and projection matrices should not change while drawing the main scene to the z-buffer.
																					//When the view matrix is updated we need to update the
																					//W, WV, VP, and WVP matrices.
		m_mV   = pMat;
		m_mWV  = m_mW*m_mV;
		m_mVP  = m_mV*m_mP;
		m_mWVP = m_mWV*m_mP;

		m_VSC.m_mV = eg_mat::BuildTranspose(m_mV);
		m_VSC.m_mWVP = eg_mat::BuildTranspose(m_mWVP);
		m_VSC.m_mVP = eg_mat::BuildTranspose(m_mVP);
		m_VSC.m_mWV = eg_mat::BuildTranspose(m_mWV);
	}

	void SetProjTF(const eg_mat& pMat)
	{
		assert( !m_bIsDrawingMainScene && !m_bIsDrawingZScene ); // The view and projection matrices should not change while drawing the main scene to the z-buffer.
																					//When the projection matrix is changed we need to update the
																					//P, VP, and WVP matrices.
		m_mP   = pMat;
		m_mVP  = m_mV*m_mP;
		m_mWVP = m_mW*m_mVP;

		m_VSC.m_mP = eg_mat::BuildTranspose(m_mP);
		m_VSC.m_mVP = eg_mat::BuildTranspose(m_mVP);
		m_VSC.m_mWVP = eg_mat::BuildTranspose(m_mWVP);
	}

	void SetBoneMats( const eg_transform* pTransforms , const eg_uint nCount )
	{
		assert( pTransforms[0].IsIdentity() );
		m_NumBones = ::EG_Clamp<eg_uint>(nCount, 0, RENDERER_MAX_BONES+1);
		for(eg_uint i=1; i<m_NumBones; i++) //Start at 1st bone since 0 should always be identity.
		{
			m_VSC.m_mBoneM[i] = eg_mat::BuildTranspose(eg_mat(pTransforms[i]));
		}
	}

	void SetFloat(RENDERER_FLOAT_T type, eg_real fValue)
	{
		switch(type)
		{
		case F_TIME           : m_VSC.m_fTime    = fValue; break;
		case F_ONEMINUTECYCLE : m_VSC.m_fOneMinuteCycleTime = fValue; break;
		}
	}

	void SetVec4(eg_rv4_t type, const eg_vec4* pv)
	{
		switch(type)
		{
		case eg_rv4_t::FREG_0        : m_VSC.m_vs_reg0 = *pv; m_PSC.m_ps_reg0 = *pv;   break; //Vertex and Pixel shader
		case eg_rv4_t::CAMERA_POS    : assert(pv->w==1); m_VSC.m_v4Camera = *pv; break; //Vertex Shader Only
		case eg_rv4_t::ENT_PALETTE_0 : m_VSC.m_vs_Palette0 = *pv; m_PSC.m_ps_Palette0 = *pv; break; //Vertex and pixel shader.
		case eg_rv4_t::ENT_PALETTE_1 : m_VSC.m_vs_Palette1 = *pv; m_PSC.m_ps_Palette1 = *pv; break; //Vertex and pixel shader.
		case eg_rv4_t::GAME_PALETTE_0: m_VSC.m_vs_GamePalette0 = *pv; m_PSC.m_ps_GamePalette0 = *pv; break; //Vertex and pixel shader.
		case eg_rv4_t::GAME_PALETTE_1: m_VSC.m_vs_GamePalette1 = *pv; m_PSC.m_ps_GamePalette1 = *pv; break; //Vertex and pixel shader.
		case eg_rv4_t::GAME_PALETTE_2: m_VSC.m_vs_GamePalette2 = *pv; m_PSC.m_ps_GamePalette2 = *pv; break;
		case eg_rv4_t::SCALE         : m_VSC.m_ScaleVec = *pv; break; //Vertex shader only
		case eg_rv4_t::AMBIENT_LIGHT : m_VSC.m_AmbientLight = *pv; break;
		}
	}

	void SetIVec4( eg_r_ivec4_t InType , const eg_ivec4& InVec )
	{
		switch( InType )
		{
		case eg_r_ivec4_t::EntInts0: m_VSC.m_vs_EntInts0 = InVec; m_PSC.m_ps_EntInts0 = InVec; break;
		}
	}

	void ResetBoneMats()
	{
		for(eg_uint i=0; i<countof(m_VSC.m_mBoneM); i++)
		{
			m_VSC.m_mBoneM[i] = eg_mat::I;
		}
	}

	void SetQuadColor( const eg_color& Color )
	{
		m_Quad[0].Color0 
			= m_Quad[1].Color0
			= m_Quad[2].Color0
			= m_Quad[3].Color0
			= m_Quad[4].Color0
			= m_Quad[5].Color0 = Color;
	}

	void PushDefaultRenderStates()
	{
		m_LastRasterizerState = eg_rasterizer_s::COUNT;
		m_LastSamplerState = eg_sampler_s::COUNT;
		m_LastBlendState = eg_blend_s::COUNT;
		m_LastDepthStencilState = eg_depthstencil_s::COUNT;
		m_LastDefaultShaderType = eg_defaultshader_t::COUNT;
		m_LastMaterial = EGV_MATERIAL_NULL;

		m_CurRasterizerState.Push( eg_rasterizer_s::CULL_CCW );
		m_CurSamplerState.Push( eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT );
		m_CurBlendState.Push( eg_blend_s::BLEND_DEFAULT_COLOR_ALL );
		m_CurDepthStencilState.Push( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_ON );
		m_CurDefaultShader.Push( eg_defaultshader_t::TEXTURE );

		SetMaterial( EGV_MATERIAL_NULL );
	}

	void PopDefaultRenderStates()
	{
		m_CurRasterizerState.Pop();
		m_CurSamplerState.Pop();
		m_CurBlendState.Pop();
		m_CurDepthStencilState.Pop();
		m_CurDefaultShader.Pop();
		SetMaterial( EGV_MATERIAL_NULL );
	}

	///////////////////////////////
	// Asset Creation (Main Thread)
	///////////////////////////////
	virtual void ReleaseMaterial(egv_material mtrl) override final;
	virtual void ReleaseVB(egv_vbuffer buffer) override final;
	virtual void ReleaseIB(egv_ibuffer buffer) override final;
	virtual void DestroyRenderTarget( egv_rtarget RenderTarget ) override final;

	void ProcessDeleteQue( eg_bool bForce , eg_uint LastDlAssetState );

	virtual void HandleDeleteAsset( const egDestroyItem& DestroyItem )=0;

//
// Attributes:
//
protected:

	//Display lists, 3 exist. One for the main thread to draw to.
	//One to be queued for the render thread.
	//And one that the render thread is currently processing.
	//The render thread will repeatedly draw the last display list submitted
	//to it. If there is one in the queue it takes that one. A newly submitted
	//one goes in the queue. If there was one already there it will get dropped.
	//So long as the render thread is running faster than the game thread no
	//frames should get dropped. Though some frames will be repeatedly drawn.

	egDisplayListData  m_DisplayList[DISPLAY_LIST_BUFFER_COUNT];
	EGDlStack          m_DlAvailable; //These are the display lists that can be used.
	egDisplayListData* m_DlMainThread;//The display list that the main thread has acquired.
	egDisplayListData* m_DlNext;      //This is the last display list to be submitted.
	egDisplayListData* m_DlDraw;      //This is the display list that is currently being drawn.
	EGMutex            m_DlLock;      //Lock for moving around display lists.

	EGDestroyQue       m_DestroyQue;

	mutable EGMutex    m_DataLock;
	mutable EGMutex    m_AssetCreationLock;
	mutable EGMutex    m_AssetStateLock;

	eg_uint            m_AssetState;

	EGRsStack          m_CurRasterizerState;
	EGSsStack          m_CurSamplerState;
	EGBsStack          m_CurBlendState;
	EGDsStack          m_CurDepthStencilState;
	EGShStack          m_CurDefaultShader;
	egv_material       m_CurMaterial;
							 
	eg_rasterizer_s    m_LastRasterizerState;
	eg_sampler_s       m_LastSamplerState;
	eg_blend_s         m_LastBlendState;
	eg_depthstencil_s  m_LastDepthStencilState;
	eg_defaultshader_t m_LastDefaultShaderType;
	egv_material       m_LastMaterial;
							 
	// Shader constants:
	eg_mat             m_mW;
	eg_mat             m_mV;
	eg_mat             m_mP;
	eg_mat             m_mWVP;
	eg_mat             m_mWV;
	eg_mat             m_mVP;
	eg_uint            m_NumBones;
	egRVSConsts        m_VSC;
	egRPSConsts        m_PSC;

	egv_vert_simple    m_Quad[6]; //Quad, used to render shadows, and quads.

	EGResList          m_Resolutions;

	INIT_S             m_InitStatus:8;						 
	eg_bool            m_DrawSplash:1;
	eg_bool            m_bUseMtrlOverride:1;
	eg_bool            m_bIsDrawing:1;
	eg_bool            m_bIsDrawingMainScene:1;
	eg_bool            m_bIsDrawingZScene:1;
};
