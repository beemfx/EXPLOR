// (c) 2017 Beem Media

#include "EGUiImageWidget.h"
#include "EGUiImageWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGCamera2.h"
#include "EGEngine.h"

EG_CLASS_DECL( EGUiImageWidget )

void EGUiImageWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	EGUiWidget::Init( InOwner , InInfo ); // Skip the EGUiMeshWidget::Init

	m_ScaleVector = eg_vec4(m_Info->ScaleVec,1.f);

	assert( InInfo->Type == egUiWidgetInfo::eg_obj_t::IMAGE );

	m_MaterialDef.m_Mtr = InInfo->ImageInfo.Properties;
	EGString_Copy( m_MaterialDef.m_strTex[0] , *InInfo->ImageInfo.Texture.FullPath , countof(m_MaterialDef.m_strTex[0]) );
	if( m_MaterialDef.m_strTex[0][0] == '\0' )
	{
		EGString_Copy( m_MaterialDef.m_strTex[0] , "/egdata/textures/default_white" , countof(m_MaterialDef.m_strTex[0]) );
	}
	EGString_Copy( m_MaterialDef.m_strTex[1] , "/egdata/textures/default_normal" , countof(m_MaterialDef.m_strTex[1]) );
	EGString_Copy( m_MaterialDef.m_strTex[2] , "/egdata/textures/default_white" , countof(m_MaterialDef.m_strTex[2]) );

	eg_d_string VSShaderPath = InInfo->ImageInfo.Shader.FullPath;
	eg_d_string PSShaderPath = VSShaderPath;
	EGStringEx_ReplaceAll( PSShaderPath , "_vs_" , "_ps_" );
	EGStringEx_ReplaceAll( PSShaderPath , "_VS_" , "_PS_" );
	EGString_Copy( m_MaterialDef.m_strVS , *VSShaderPath , countof(m_MaterialDef.m_strVS) );
	EGString_Copy( m_MaterialDef.m_strPS , *PSShaderPath , countof(m_MaterialDef.m_strPS) );
	
	auto GetMyIndex = [&InOwner,&InInfo]() -> eg_uint
	{
		const EGUiLayout* Layout = InOwner->GetLayout();
		for( eg_uint i=0; i<Layout->m_Objects.LenAs<eg_uint>(); i++ )
		{
			if( InInfo == Layout->m_Objects.GetByIndex( i ) )
			{
				return i;
			}
		}

		return -1;
	};

	// There is actually a pretty problematic bug here... Basically if we create a material with the same name as
	// previously existing material before it gets purged we won't get the new material. This really only happens
	// with the editor when we change a texture. Since the names come from the definition. So in-game this wouldn't
	// ever happen. So when we are in the tool we'll add some additional information to make the name different so
	// the old material can get purged properly. Really this should happen in the renderer, but for now this seems
	// to work.
	const eg_int UniqueNumber = Engine_IsTool() ? static_cast<eg_int>(reinterpret_cast<eg_uintptr_t>(this)) : 0;

	eg_string MaterialUniqueId = EGString_Format("ImgWidget(%s,%i,%d)" , *InOwner->GetLayout()->m_Id , GetMyIndex() , UniqueNumber );
	m_OverrideMaterial = EGRenderer::Get().CreateMaterial( &m_MaterialDef , MaterialUniqueId );
	
	m_EntObj.Init( eg_crc("EGUIObjectImage1x1") );
	m_EntObj.SetDrawInfo( GetFullPose(1.f) , m_ScaleVector , m_Info->IsLit );

	if( InInfo->bUsePointFiltering || InInfo->bUseTextureEdgeClamp )
	{
		if( InInfo->bUsePointFiltering )
		{
			SetOverrideSamplerState( InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT : eg_sampler_s::TEXTURE_WRAP_FILTER_POINT );
		}
		else
		{
			SetOverrideSamplerState( InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT : eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT );
		}
	}
}

EGUiImageWidget::~EGUiImageWidget()
{
	DestroyMaterial();
}

void EGUiImageWidget::Draw( eg_real AspectRatio )
{
	MainDisplayList->SetMaterialOverride( m_OverrideMaterial );
	Super::Draw( AspectRatio );
	MainDisplayList->SetMaterialOverride( EGV_MATERIAL_NULL );
}

void EGUiImageWidget::SetTexture( eg_string_crc NodeId, eg_string_crc GroupId, eg_cpstr TexturePath )
{
	unused( NodeId , GroupId );

	if( !EGString_EqualsI( TexturePath , m_MaterialDef.m_strTex[0] ) )
	{
		DestroyMaterial();
		EGString_Copy( m_MaterialDef.m_strTex[0] , TexturePath , countof(m_MaterialDef.m_strTex[0]) );
		m_OverrideMaterial = EGRenderer::Get().CreateMaterial( &m_MaterialDef , "" );
		m_bMaterialIsExternal = false;
	}
}

void EGUiImageWidget::SetMaterial( eg_string_crc NodeId, eg_string_crc GroupId, egv_material Material )
{
	unused( NodeId , GroupId );

	DestroyMaterial();
	EGString_Copy( m_MaterialDef.m_strTex[0] , "ExtMaterial" , countof(m_MaterialDef.m_strTex[0]) );
	m_OverrideMaterial = Material;
	m_bMaterialIsExternal = true;
}

void EGUiImageWidget::SetScaleVector( const eg_vec3& NewScale )
{
	m_ScaleVector = eg_vec4( NewScale , 1.f );
	m_EntObj.SetDrawInfo( GetFullPose(1.f) , m_ScaleVector , m_Info->IsLit );
}

void EGUiImageWidget::DestroyMaterial()
{
	if( !m_bMaterialIsExternal )
	{
		EGRenderer::Get().DestroyMaterial( m_OverrideMaterial );
	}
	m_OverrideMaterial = EGV_MATERIAL_NULL;
	m_bMaterialIsExternal = false;
}
