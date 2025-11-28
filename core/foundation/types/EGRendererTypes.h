#pragma once
//Some constants:
static const eg_uint RENDERER_MAX_LIGHTS        = 4;
static const eg_uint RENDERER_MAX_MULTI_TEXTURE = 4;
static const eg_uint RENDERER_MAX_GLOBAL_SAMPLER = 3;
static const eg_uint RENDERER_MAX_BONES         = 31;

enum class eg_screen_mode
{
	WINDOWED,
	FULL_SCREEN,
	BORDERLESS,
};

enum class eg_rasterizer_s: eg_uint8
{
	CULL_CCW  ,
	CULL_CW   ,
	CULL_NONE ,

	WIREFRAME ,

	COUNT,
};

enum class eg_sampler_s: eg_uint8
{
	TEXTURE_CLAMP_FILTER_POINT,
	TEXTURE_CLAMP_FILTER_DEFAULT,
	TEXTURE_WRAP_FILTER_POINT,
	TEXTURE_WRAP_FILTER_DEFAULT,

	COUNT,
};

enum class eg_blend_s: eg_uint8
{
	BLEND_DEFAULT_COLOR_ALL,
	BLEND_NONE_COLOR_ALL,
	BLEND_NONE_COLOR_NONE,

	COUNT,
};

enum class eg_depthstencil_s: eg_uint8
{
	ZTEST_DEFAULT_ZWRITE_ON,
	ZTEST_DEFAULT_ZWRITE_OFF,
	ZTEST_NONE_ZWRITE_ON,
	ZTEST_NONE_ZWRITE_OFF,
	ZTEST_DEFAULT_ZWRITE_OFF_STEST_SWRITE_MESH_SHADOW,
	ZTEST_NONE_ZWRITE_OFF_STEST_NONZERO,
	ZTEST_NONE_ZWRITE_OFF_STEST_ZERO,
	ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONE_SWRITE_1,
	ZTEST_DEFAULT_ZWRITE_ON_STEST_NONZERO,
	ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONZERO,

	COUNT,
};

enum class eg_defaultshader_t: eg_uint8
{
	TEXTURE,
	MESH_LIT,
	MESH_UNLIT,
	MAP,
	SHADOW,
	FONT,
	FONT_SHADOW,
	FONT_GLYPH,
	FONT_GLYPH_SHADOW,
	COLOR,
	TERRAIN,
	STORE_DEPTH,
	DRAW_TO_DEPTH,
	CLEAR_DEAPTH,
	FXAA,
	COPY_BUFFER,
	MOTION_BLUR_1,

	COUNT,
};

enum RENDERER_FLOAT_T
{
	F_TIME,
	F_ONEMINUTECYCLE,
};

enum class eg_rv4_t
{
	FREG_0,
	CAMERA_POS,
	ENT_PALETTE_0,
	ENT_PALETTE_1,
	GAME_PALETTE_0,
	GAME_PALETTE_1,
	GAME_PALETTE_2,
	SCALE,
	AMBIENT_LIGHT,
};

enum class eg_r_ivec4_t
{
	EntInts0,
};

enum RENDERER_TEX_FILTER_T
{
	FILTER_POINT,
	FILTER_BILINEAR,
	FILTER_TRILINEAR,
	FILTER_ANISOTROPIC,
};

enum RENDERER_SHADER_QUALITY
{
	SHADER_QUALITY_VERY_HIGH,
	SHADER_QUALITY_HIGH,
	SHADER_QUALITY_MEDIUM,
	SHADER_QUALITY_LOW,
	SHADER_QUALITY_VERY_LOW,
	SHADER_QUALITY_MAXENUM,
};

enum egv_material: eg_uint{ EGV_MATERIAL_NULL };
enum egv_ibuffer : eg_uintptr_t{ EGV_IBUFFER_NULL };
enum class egv_rtarget : eg_uintptr_t{ Null , DepthBuffer };

struct egv_vbuffer
{
	eg_uintptr_t IntV1;
	eg_uint32    IntV2;
	eg_uint32    IntV3;

	egv_vbuffer() = default;
	egv_vbuffer( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			IntV1 = 0;
			IntV2 = 0;
			IntV3 = 0;
		}
	}

	eg_bool IsValid() const { return IntV1 != 0; }
};

#define IN_GAME_DECL
#include "EGRenderTypes.inc"
#undef IN_GAME_DECL

struct egRendererSpecs
{
	eg_bool bDrawZScene:1;
	eg_bool bDrawVolumeShadows:1;
	eg_bool bDoSimpleOnePass:1;

	egRendererSpecs(): bDrawZScene(false), bDrawVolumeShadows(false), bDoSimpleOnePass(false){ }
};


struct EGMaterialDef
{
	static const eg_uint MAX_TEX  = 4;

	eg_path m_strTex[MAX_TEX];
	eg_path m_strVS;
	eg_path m_strPS;

	EGMaterial m_Mtr;

	//Helper functionality:
	EGMaterialDef();
	EGMaterialDef( eg_ctor_t Ct );
	void SetFromTag( const class EGXmlAttrGetter& AttGet );
	eg_string_big CreateXmlTag( eg_uint Id );
	void MakePathsRelativeTo(eg_cpstr strFile);
	void SetShader( eg_cpstr Shader );

	private: void Reset();
};

