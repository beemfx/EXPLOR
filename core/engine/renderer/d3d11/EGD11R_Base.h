#pragma once
#include "EGDirectXAPI.h"

static const DXGI_FORMAT D3D_BB_FMT_A = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT D3D_BB_FMT_X = DXGI_FORMAT_R8G8B8A8_UNORM;
static const DXGI_FORMAT D3D_BB_FMT_Z = DXGI_FORMAT_R32_FLOAT;

static const D3D11_INPUT_ELEMENT_DESC D3D11R_VF_STANDARD[] = 
{
	{ "SV_POSITION"  , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 0  , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "NORMAL"       , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 16 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TANGENT"      , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 32 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TEXCOORD"     , 0 , DXGI_FORMAT_R32G32_FLOAT       , 0 , 48 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TEXCOORD"     , 1 , DXGI_FORMAT_R32G32_FLOAT       , 0 , 56 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "COLOR"        , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 64 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "BLENDINDICES" , 0 , DXGI_FORMAT_R32_UINT           , 0 , 80 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "BLENDWEIGHT"  , 0 , DXGI_FORMAT_R32_FLOAT          , 0 , 84 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "BLENDINDICES" , 1 , DXGI_FORMAT_R32_UINT           , 0 , 88 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "BLENDWEIGHT"  , 1 , DXGI_FORMAT_R32_FLOAT          , 0 , 92 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
};

static_assert( 96 == sizeof(egv_vert_mesh) , "egv_vert_mesh changed?" );

static const D3D11_INPUT_ELEMENT_DESC D3D11R_VF_SIMPLE[] = 
{
	{ "SV_POSITION"  , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 0  , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TEXCOORD"     , 0 , DXGI_FORMAT_R32G32_FLOAT       , 0 , 16 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "COLOR"        , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 24 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
};

static_assert( 40 == sizeof(egv_vert_simple) , "egv_vert_simple changed?" );

static const D3D11_INPUT_ELEMENT_DESC D3D11R_VF_TERRAIN[] = 
{
	{ "SV_POSITION"  , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 0  , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "NORMAL"       , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 16 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TANGENT"      , 0 , DXGI_FORMAT_R32G32B32A32_FLOAT , 0 , 32 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TEXCOORD"     , 0 , DXGI_FORMAT_R32G32_FLOAT       , 0 , 48 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
	{ "TEXCOORD"     , 1 , DXGI_FORMAT_R32G32_FLOAT       , 0 , 56 , D3D11_INPUT_PER_VERTEX_DATA , 0 },
};

static_assert( 64 == sizeof(egv_vert_terrain) , "egv_vert_terrain changed?" );

struct egD11R_VertexFormat
{
	const eg_string_crc             VertexType;
	const eg_size_t                 VertexSize;
	eg_cpstr                        TemplateShader;
	const D3D11_INPUT_ELEMENT_DESC* ElementDesc;
	const eg_uint32                 ElementCount;
};

static const egD11R_VertexFormat D11R_VertexFormats[] =
{
	{ eg_crc("egv_vert_mesh") , sizeof(egv_vert_mesh) , "/egdata/shaders/default_EGVert.evs5" , D3D11R_VF_STANDARD , countof(D3D11R_VF_STANDARD) },
	{ eg_crc("egv_vert_simple") , sizeof(egv_vert_simple) , "/egdata/shaders/default_EGVert_Simple.evs5" , D3D11R_VF_SIMPLE , countof(D3D11R_VF_SIMPLE) },
	{ eg_crc("egv_vert_terrain") , sizeof(egv_vert_terrain) , "/egdata/shaders/default_EGVert_Terrain.evs5" , D3D11R_VF_TERRAIN , countof(D3D11R_VF_TERRAIN) },
};