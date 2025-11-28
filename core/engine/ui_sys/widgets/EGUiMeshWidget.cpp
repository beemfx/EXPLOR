#include "EGUiMeshWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGCamera2.h"
#include "EGEngine.h"

EG_CLASS_DECL( EGUiMeshWidget )

void EGUiMeshWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	assert( InInfo->Type == egUiWidgetInfo::eg_obj_t::MESH );

	m_EntObj.Init( m_Info->EntDefCrc );

	for( const egTextOverrideInfo& Info : m_Info->TextOverrides )
	{
		if( Info.NodeId.Crc.IsNotNull() )
		{
			SetText( Info.NodeId , eg_loc_text(Info.Text) );
		}
	}

	for( const egBoneOverride& Info : m_Info->BoneOverrides )
	{
		if( Info.BoneId.IsNotNull() )
		{
			SetCBone( CT_Clear , Info.BoneId , Info.Transform );
		}
	}

	m_ScaleVector = eg_vec4( m_Info->ScaleVec , 1.f );
	m_EntObj.SetDrawInfo( GetFullPose(1.f) , m_ScaleVector , m_Info->IsLit );
	
	if( InInfo->bUsePointFiltering || InInfo->bUseTextureEdgeClamp )
	{
		if( InInfo->bUsePointFiltering )
		{
			SetOverrideSamplerState( InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT : eg_sampler_s::TEXTURE_WRAP_FILTER_POINT );
		}
		else
		{
			SetOverrideSamplerState(InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT : eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT );
		}
	}

	UpdateLocTextForTool();
}

EGUiMeshWidget::~EGUiMeshWidget()
{
	m_EntObj.Deinit();
}

void EGUiMeshWidget::Draw( eg_real AspectRatio )
{
	eg_transform Pose = GetFullPose( AspectRatio );
	m_EntObj.SetPose( Pose );

	if(m_Info->IsLit)
	{
		MainDisplayList->DisableAllLights();

		eg_vec4 CameraPos;
		eg_color AmbientColor;
		EGLight Lts[4];
		eg_uint NumLights = m_Owner->GetLights( Lts , countof(Lts), &CameraPos, &AmbientColor );

		MainDisplayList->SetVec4( eg_rv4_t::AMBIENT_LIGHT , AmbientColor.ToVec4() );
		MainDisplayList->SetVec4( eg_rv4_t::CAMERA_POS , CameraPos );

		for( eg_uint i=0; i<NumLights; i++ )
		{
			Lts[i].Dir = Pose.GetPosition() - Lts[i].Pos;
			Lts[i].Dir.NormalizeThisAsVec3();
			MainDisplayList->SetLight( i , Lts[i] );
			MainDisplayList->EnableLight( i , true );
		}
	}

	if( m_Info->bDrawAsMask )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONE_SWRITE_1 );
		MainDisplayList->PushBlendState( eg_blend_s::BLEND_NONE_COLOR_NONE );
	}

	if( m_Info->bDrawOnMaskOnly )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_ON_STEST_NONZERO );
	}

	if( m_OverrideSamplerState != eg_sampler_s::COUNT )
	{
		MainDisplayList->PushSamplerState( m_OverrideSamplerState );
	}

	m_EntObj.Draw();

	if( m_OverrideSamplerState != eg_sampler_s::COUNT )
	{
		MainDisplayList->PopSamplerState();
	}

	if( m_Info->bDrawOnMaskOnly )
	{
		MainDisplayList->PopDepthStencilState();
	}

	if( m_Info->bDrawAsMask )
	{
		MainDisplayList->PopDepthStencilState();
		MainDisplayList->PopBlendState();
	}
}

void EGUiMeshWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	unused( AspectRatio );
	m_EntObj.Update( DeltaTime );
}

void EGUiMeshWidget::SetMuteAudio( eg_bool bMute )
{
	m_EntObj.SetMuteAudio( bMute );
}

eg_aabb EGUiMeshWidget::GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj ) const
{
	//Form the button bounding box.

	//1) Transform the location of the button to screen space.
	const eg_transform Pose = GetFullPose(AspectRatio);

	//3) Create a bound box with the specified bounds.

	eg_aabb ButtonDims;
	if( m_EntObj.IsValid() )
	{
		ButtonDims = m_EntObj.GetBaseBounds();
	}
	else
	{
		zero( &ButtonDims );
	}

	ButtonDims.Min.x *= m_ScaleVector.x;
	ButtonDims.Min.y *= m_ScaleVector.y;
	ButtonDims.Min.z *= m_ScaleVector.z;
	ButtonDims.Min.w *= m_ScaleVector.w;

	ButtonDims.Max.x *= m_ScaleVector.x;
	ButtonDims.Max.y *= m_ScaleVector.y;
	ButtonDims.Max.z *= m_ScaleVector.z;
	ButtonDims.Max.w *= m_ScaleVector.w;

	ButtonDims.Min.w = 1.f;
	ButtonDims.Max.w = 1.f;

	eg_vec4 Corners[8];
	ButtonDims.Get8Corners( Corners , countof(Corners) );

	for( eg_vec4& Corner : Corners )
	{
		Corner *= Pose;

		eg_vec2 CornerMouseSpace = EGCamera2::WorldSpaceToMouseSpace( Corner , AspectRatio , MatView , MatProj );
		Corner.x = CornerMouseSpace.x;
		Corner.y = CornerMouseSpace.y;
		Corner.z = 0.f;
		Corner.w = 1.f;
	}

	ButtonDims.CreateFromVec4s( Corners , countof(Corners) );

	return ButtonDims;
}

void EGUiMeshWidget::QueryEvents( EGArray<eg_d_string>& Out )
{
	const EGEntDef* Def = m_EntObj.GetDef();
	if( Def )
	{
		Def->QueryEventList( Out );
	}
}

void EGUiMeshWidget::QueryTextNodes( EGArray<eg_d_string>& Out )
{
	m_EntObj.QueryTextNodes( Out );
}

void EGUiMeshWidget::QueryBones( EGArray<eg_d_string>& Out )
{
	m_EntObj.QueryBones( Out );
}

void EGUiMeshWidget::RunEvent( eg_string_crc Event )
{
	m_EntObj.RunEvent( Event );
}

void EGUiMeshWidget::SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t )
{
	m_EntObj.SetAnimationNormalTime( NodeId, SkeletonId, AnimationId, t );
}

void EGUiMeshWidget::SetCBone( eg_string_crc NodeId, eg_string_crc BoneId, const eg_transform& Pose )
{
	m_EntObj.SetCustomBone( NodeId, BoneId, Pose );
}

void EGUiMeshWidget::SetTexture( eg_string_crc NodeId, eg_string_crc GroupId, eg_cpstr TexturePath )
{
	m_EntObj.SetTexture( NodeId, GroupId, TexturePath );
}

void EGUiMeshWidget::SetMaterial( eg_string_crc NodeId, eg_string_crc GroupId, egv_material Material )
{
	m_EntObj.SetMaterial( NodeId , GroupId , Material );
}

void EGUiMeshWidget::SetPalette( eg_uint PaletteIndex, const eg_vec4& Palette )
{
	if( 0 == PaletteIndex )
	{
		m_EntObj.SetPalette( Palette );
	}
	else
	{
		assert( false ); //PaletteIndex out of range.
	}
}

void EGUiMeshWidget::SetText( eg_string_crc TextNodeCrcId, const eg_loc_text& NewText )
{
	m_EntObj.SetText( TextNodeCrcId, NewText );
}

void EGUiMeshWidget::UpdateLocTextForTool()
{
	if( Engine_IsTool() )
	{
		for( const egTextOverrideInfo& Info : m_Info->TextOverrides )
		{
			eg_loc_text LocText = eg_loc_text(Info.Text);
			if( Info.Text_enus.Len() > 0 )
			{
				LocText = eg_loc_text(EGString_ToWide(*Info.Text_enus));
			}
			else if( LocText.GetLen() > 0 && LocText.GetString()[0] == '!' )
			{
				LocText = eg_loc_text(EGString_ToWide(EGString_Format( "LOCKEY:%s" , EGCrcDb::CrcToString(Info.Text).String() )));
			}

			SetText( Info.NodeId , LocText );
		}
	}
}

