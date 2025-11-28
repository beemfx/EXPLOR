/******************************************************************************
File: Video_SetRenderStates.cpp
Class: CD3DRenderer
Purpose: Contains functions for changing specific render states.

(c) 2011 Beem Software
******************************************************************************/

#include "EGD11R_Base.h"
#include "EGD11R_Renderer.h"
#include "EGGlobalConfig.h"

void EGD11R_Renderer::Init_RS( eg_bool bInit )
{
	HRESULT Res;
	if( bInit )
	{
		// Rasterizer State:
		{
			/*
			D3D11_RASTERIZER_DESC:

			D3D11_FILL_MODE FillMode;
			D3D11_CULL_MODE CullMode;
			BOOL FrontCounterClockwise;
			INT DepthBias;
			FLOAT DepthBiasClamp;
			FLOAT SlopeScaledDepthBias;
			BOOL DepthClipEnable;
			BOOL ScissorEnable;
			BOOL MultisampleEnable;
			BOOL AntialiasedLineEnable;
			*/

			static const D3D11_RASTERIZER_DESC RsBaseDesc = 
				{ D3D11_FILL_SOLID 
				, D3D11_CULL_BACK 
				, FALSE 
				, 0
				, 0.f 
				, 0.f 
				, TRUE 
				, FALSE 
				, FALSE 
				, FALSE };

			for( eg_uint i=0; i<enum_count(eg_rasterizer_s); i++ )
			{
				assert( nullptr == m_RasterizerStates[i] );
				eg_rasterizer_s Mode = static_cast<eg_rasterizer_s>(i);
				D3D11_RASTERIZER_DESC Desc = RsBaseDesc;
				switch( Mode )
				{
				case eg_rasterizer_s::CULL_CCW: 
					break;
				case eg_rasterizer_s::CULL_NONE:
					Desc.CullMode = D3D11_CULL_NONE;
					break;
				case eg_rasterizer_s::CULL_CW:
					//Desc.CullMode = D3D11_CULL_FRONT;
					Desc.FrontCounterClockwise = TRUE;
					break;
				case eg_rasterizer_s::WIREFRAME:
					Desc.FillMode = D3D11_FILL_WIREFRAME;
					// Desc.CullMode = D3D11_CULL_NONE;
					break;
				case eg_rasterizer_s::COUNT: assert(false); break;
				}

				Res = m_Device->CreateRasterizerState( &Desc , &m_RasterizerStates[i] );
				assert( SUCCEEDED(Res) );
			}
		}

		// Sampler State:
		{
			/*
			D3D11_SAMPLER_DESC:

			D3D11_FILTER Filter;
			D3D11_TEXTURE_ADDRESS_MODE AddressU;
			D3D11_TEXTURE_ADDRESS_MODE AddressV;
			D3D11_TEXTURE_ADDRESS_MODE AddressW;
			FLOAT MipLODBias;
			UINT MaxAnisotropy;
			D3D11_COMPARISON_FUNC ComparisonFunc;
			FLOAT BorderColor[ 4 ];
			FLOAT MinLOD;
			FLOAT MaxLOD;
			*/

			static const D3D11_SAMPLER_DESC SmBaseDesc =
				{ D3D11_FILTER_MIN_MAG_MIP_POINT 
				, D3D11_TEXTURE_ADDRESS_WRAP 
				, D3D11_TEXTURE_ADDRESS_WRAP 
				, D3D11_TEXTURE_ADDRESS_CLAMP
				, 0 
				, 1 
				, D3D11_COMPARISON_NEVER 
				, {1.0,1.0,1.0,1.0} 
				, -D3D11_FLOAT32_MAX 
				, D3D11_FLOAT32_MAX };



			for( eg_uint i=0; i<countof(m_SamplerStates); i++ )
			{
				assert( nullptr == m_SamplerStates[i] );
				eg_sampler_s Mode = static_cast<eg_sampler_s>(i);
				D3D11_SAMPLER_DESC Desc = SmBaseDesc;

				switch( Mode )
				{
				case eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT: 
					Desc.AddressU = Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					Init_RS_GetTxFilter( FILTER_POINT , &Desc );
					break;
				case eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT: 
					Desc.AddressU = Desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
					Init_RS_GetTxFilter( m_TextureFilter , &Desc );
					break;
				case eg_sampler_s::TEXTURE_WRAP_FILTER_POINT: 
					Desc.AddressU = Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					Init_RS_GetTxFilter( FILTER_POINT , &Desc );
					break;
				case eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT: 
					Desc.AddressU = Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
					Init_RS_GetTxFilter( m_TextureFilter , &Desc );
					break;
				case eg_sampler_s::COUNT: assert(false); break;
				}

				Res = m_Device->CreateSamplerState( &Desc , &m_SamplerStates[i] );
				assert( SUCCEEDED(Res) );
			}
		}

		{
			D3D11_BLEND_DESC BlBaseDesc;
			zero( &BlBaseDesc );
			BlBaseDesc.AlphaToCoverageEnable = FALSE;
			BlBaseDesc.IndependentBlendEnable = FALSE;
			BlBaseDesc.RenderTarget[0].BlendEnable = TRUE;
			BlBaseDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			BlBaseDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			BlBaseDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			BlBaseDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			BlBaseDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			BlBaseDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			BlBaseDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			for( eg_uint i=0; i<countof(m_BlendStates); i++ )
			{
				assert( nullptr == m_BlendStates[i] );
				eg_blend_s Mode = static_cast<eg_blend_s>(i);
				D3D11_BLEND_DESC Desc = BlBaseDesc;
				switch( Mode )
				{
					case eg_blend_s::BLEND_DEFAULT_COLOR_ALL:
					{
						// Default is already this...
					} break;
					case eg_blend_s::BLEND_NONE_COLOR_NONE:
					{
						Desc.RenderTarget[0].BlendEnable = FALSE;
						Desc.RenderTarget[0].RenderTargetWriteMask = true ? 0 : D3D11_COLOR_WRITE_ENABLE_ALL;
					} break;
					case eg_blend_s::BLEND_NONE_COLOR_ALL:
					{
						Desc.RenderTarget[0].BlendEnable = FALSE;
					} break;
					case eg_blend_s::COUNT: assert(false); break;
				}

				Res = m_Device->CreateBlendState( &Desc , &m_BlendStates[i] );
				assert( SUCCEEDED(Res) );
			}
		}

		// Depth Stencil States:
		{
			/*
			D3D11_DEPTH_STENCIL_DESC:

			BOOL DepthEnable;
			D3D11_DEPTH_WRITE_MASK DepthWriteMask;
			D3D11_COMPARISON_FUNC DepthFunc;
			BOOL StencilEnable;
			UINT8 StencilReadMask;
			UINT8 StencilWriteMask;
			D3D11_DEPTH_STENCILOP_DESC FrontFace;
			D3D11_DEPTH_STENCILOP_DESC BackFace;

			D3D11_DEPTH_STENCILOP_DESC:

			D3D11_STENCIL_OP      StencilFailOp;
			D3D11_STENCIL_OP      StencilDepthFailOp;
			D3D11_STENCIL_OP      StencilPassOp;
			D3D11_COMPARISON_FUNC StencilFunc;
			*/

			static const D3D11_DEPTH_STENCIL_DESC DsBaseDesc = 
			{ TRUE 
			, D3D11_DEPTH_WRITE_MASK_ALL 
			, D3D11_COMPARISON_LESS_EQUAL
			, FALSE
			, 0
			, 0
			, { D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_COMPARISON_ALWAYS }
			, { D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_STENCIL_OP_KEEP , D3D11_COMPARISON_ALWAYS } };


			for( eg_uint i=0; i<countof(m_DepthStencilStates); i++ )
			{
				assert( nullptr == m_DepthStencilStates[i] );
				eg_depthstencil_s Mode = static_cast<eg_depthstencil_s>(i);
				D3D11_DEPTH_STENCIL_DESC Desc = DsBaseDesc;
				m_DepthStencilRef[i] = 0;
				switch( Mode )
				{
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_ON:
					// This is the default.
					break;
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF:
					// Meant for skybox drawing...
					//With the current implementation and the way the projection matrix is
					//modified, we don't actually need to change anything from regular mesh
					//render states, but for performance we can disable z writes. (All
					//skybox z values will be 1.0 anyway.
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					break;
				case eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF:
					Desc.DepthEnable = false;
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					break;
				case eg_depthstencil_s::ZTEST_NONE_ZWRITE_ON:
					Desc.DepthEnable = false;
					break;
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_SWRITE_MESH_SHADOW:
					Desc.DepthEnable = true;
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					Desc.StencilEnable = true;
					Desc.FrontFace.StencilFunc        = Desc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
					Desc.FrontFace.StencilPassOp      = Desc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
					Desc.FrontFace.StencilFailOp      = Desc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
					Desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
					Desc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_INCR;
					Desc.StencilReadMask  = 0x00;
					Desc.StencilWriteMask = 0xFF;
					break;
				case eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF_STEST_NONZERO:
				case eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF_STEST_ZERO:
					Desc.DepthEnable = false;
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					Desc.StencilEnable = true;
					if( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF_STEST_NONZERO == Mode )
					{
						Desc.FrontFace.StencilFunc = Desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
					}
					else
					{
						Desc.FrontFace.StencilFunc = Desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
					}
					Desc.FrontFace.StencilPassOp      = Desc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_ZERO;
					Desc.FrontFace.StencilDepthFailOp = Desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
					Desc.FrontFace.StencilFailOp      = Desc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_ZERO;
					Desc.StencilReadMask  = 0xFF;
					Desc.StencilWriteMask = 0x00;
					break;
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONE_SWRITE_1:
					// This will effectively mask everything that draws exactly
					// on top of itself.
					Desc.DepthEnable = true;
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
					Desc.StencilEnable = true;
					Desc.StencilWriteMask = 0xFF;
					Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
					m_DepthStencilRef[i] = 1;
					break;
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_ON_STEST_NONZERO:
					Desc.FrontFace.StencilFunc = Desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
					Desc.StencilEnable = true;
					Desc.StencilReadMask  = 0xFF;
					Desc.StencilWriteMask = 0x00;
					m_DepthStencilRef[i] = 0;
					break;
				case eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONZERO:
					Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
					Desc.FrontFace.StencilFunc = Desc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
					Desc.StencilEnable = true;
					Desc.StencilReadMask  = 0xFF;
					Desc.StencilWriteMask = 0x00;
					m_DepthStencilRef[i] = 0;
					break;
				case eg_depthstencil_s::COUNT: assert(false); break;
				}

				Res = m_Device->CreateDepthStencilState( &Desc , &m_DepthStencilStates[i] );
				assert( SUCCEEDED(Res) );
			}
		}
	}
	else
	{	
		//!Init	
		for( eg_uint i=0; i<countof(m_DepthStencilStates); i++ )
		{
			assert( nullptr != m_DepthStencilStates[i] );
			EG_SafeRelease( m_DepthStencilStates[i] );
		}

		for( eg_uint i=0; i<countof(m_BlendStates); i++ )
		{
			assert( nullptr != m_BlendStates[i] );
			EG_SafeRelease( m_BlendStates[i] );
		}

		for( eg_uint i=0; i<countof(m_SamplerStates); i++ )
		{
			assert( nullptr != m_SamplerStates[i] );
			EG_SafeRelease( m_SamplerStates[i] );
		}

		for( eg_uint i=0; i<countof(m_RasterizerStates); i++ )
		{
			assert( nullptr != m_RasterizerStates[i] );
			EG_SafeRelease( m_RasterizerStates[i] );
		}
	}
}

void EGD11R_Renderer::Init_RS_GetTxFilter( RENDERER_TEX_FILTER_T type , D3D11_SAMPLER_DESC* Out )
{
	//Sampler states, these states are shared for all samples
	//Should actually set these based of of GVARs or something.
	Out->Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	Out->MaxAnisotropy = 1;

	eg_uint SettingsAniso = VideoConfig_MaxAnisotropy.GetValue();

	switch(type)
	{
	default:
	case FILTER_POINT:
		break;
	case FILTER_BILINEAR:
		Out->Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case FILTER_TRILINEAR:
		Out->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case FILTER_ANISOTROPIC:
		Out->Filter = D3D11_FILTER_ANISOTROPIC;
		Out->MaxAnisotropy   = 0 == SettingsAniso ? 16 : EG_Clamp<eg_uint>(SettingsAniso, 1, 16);
		break;
	}
}
