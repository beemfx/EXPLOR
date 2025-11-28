// (c) 2016 Beem Media

#include "EGUiTextWidget.h"
#include "EGUiLayout.h"
#include "EGRenderer.h"
#include "EGTextNode.h"
#include "EGFont.h"
#include "EGCamera2.h"
#include "EGTextFormat.h"
#include "EGMenu.h"
#include "EGClient.h"
#include "EGGame.h"
#include "EGGlobalConfig.h"
#include "EGEngine.h"

EG_CLASS_DECL( EGUiTextWidget )

void EGUiTextWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	assert( InInfo->Type == egUiWidgetInfo::eg_obj_t::TEXT_NODE );

	m_TextHeight = m_Info->TextNodeInfo.Height;
	m_LineHeight = m_Info->TextNodeInfo.Height/m_Info->TextNodeInfo.NumLines;
	m_TextWidth  = m_Info->TextNodeInfo.Width;
	m_TextColor  = eg_color(m_Info->TextNodeInfo.Color);
	m_Alignment  = static_cast<eg_text_align>(m_Info->TextNodeInfo.Alignment);
	m_DropShadowColor = eg_color(m_Info->TextNodeInfo.DropShadowColor);
	m_bHasDropShadow = m_Info->TextNodeInfo.bHasDropShadow;
	m_LocText = eg_loc_text( m_Info->TextNodeInfo.LocText );
	m_Font = EGFontMgr::Get()->GetFont( eg_string_crc(*m_Info->TextNodeInfo.Font.Path) );

	UpdateLocTextForTool();
}

void EGUiTextWidget::Draw( eg_real AspectRatio )
{
	m_TextNode.SetupNode( m_Font , m_TextWidth , m_TextHeight , m_LineHeight );
	m_TextNode.SetAlignment( m_Alignment );
	m_TextNode.SetColor( m_TextColor );
	m_TextNode.SetText( m_LocText );

	eg_transform Pose = GetFullPose( AspectRatio);

	auto DrawPass = [this,&Pose]( const eg_vec2& Offset , eg_bool bShadowColor , eg_bool bDebugBorder = false ) -> void
	{
		eg_transform OffsetPose( CT_Default );
		OffsetPose.TranslateThis( Offset.x , Offset.y , 0.f );
		eg_transform FinalPose = OffsetPose * Pose;
		MainDisplayList->SetWorldTF( eg_mat(FinalPose) );

		if( bShadowColor )
		{
			eg_color FinalShadowColor = m_DropShadowColor;
			FinalShadowColor.a *= m_TextColor.a;
			m_TextNode.DrawShadow( FinalShadowColor );
		}
		else
		{
			m_TextNode.Draw();
		}

		if( bDebugBorder && DebugConfig_DrawTextNodes.GetValue() )
		{
			m_TextNode.DrawDebugBorder();
		}
	};

	if( m_bHasDropShadow )
	{
		DrawPass( m_TextNode.GetFontShadowOffset() , true );
	}
	DrawPass( eg_vec2(0.f,0.f) , false , true );
}

void EGUiTextWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	unused( DeltaTime , AspectRatio );

	if( nullptr == m_Owner )
	{
		return;
	}

	switch( m_Info->TextNodeInfo.TextContext )
	{
		case eg_text_context::Game:
		{
			EGClient* Client = m_Owner->GetClientOwner();
			const EGGame* Game = Client ? Client->SDK_GetGame() : nullptr;
			m_LocText = EGFormat( m_Info->TextNodeInfo.LocText , Game );
			UpdateLocTextForTool();
		} break;
		case eg_text_context::ThisClass:
		{
			m_LocText = EGFormat( m_Info->TextNodeInfo.LocText , m_Owner );
			UpdateLocTextForTool();
		} break;
		case eg_text_context::None:
		{
		} break;
	}
}

eg_aabb EGUiTextWidget::GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj ) const
{
	const eg_transform Pose = GetFullPose(AspectRatio);

	eg_aabb ButtonDims;
	ButtonDims.MakeZeroBox();
	ButtonDims.Min.x += -m_TextWidth/2.f;
	ButtonDims.Min.y += -m_TextHeight/2.f;
	ButtonDims.Max.x += m_TextWidth/2.f;
	ButtonDims.Max.y += m_TextHeight/2.f;

	ButtonDims.Min.z = -EG_SMALL_EPS;
	ButtonDims.Max.z = EG_SMALL_EPS;

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

void EGUiTextWidget::SetText( eg_string_crc TextNodeCrcId, const eg_loc_text& NewText )
{
	unused( TextNodeCrcId );
	m_LocText = NewText;
}

void EGUiTextWidget::SetPalette( eg_uint PaletteIndex, const eg_vec4& Palette )
{
	unused( PaletteIndex );

	eg_color Color = eg_color( Palette );
	
	m_TextColor = Color;// * m_Info->TextNodeInfo.Color;
}

void EGUiTextWidget::SetWordWrapEnabled( eg_bool bNewValue )
{
	m_TextNode.SetWordWrapEnabled( bNewValue );
}

void EGUiTextWidget::SetColorAlpha( eg_real NewValue )
{
	m_TextColor.a = NewValue;
}

void EGUiTextWidget::UpdateLocTextForTool()
{
	if( Engine_IsTool() )
	{
		if( m_Info->TextNodeInfo.LocText_enus.Len() > 0 )
		{
			m_LocText = eg_loc_text(EGString_ToWide(*m_Info->TextNodeInfo.LocText_enus));
		}
		else
		{
			m_LocText = eg_loc_text( m_Info->TextNodeInfo.LocText );
			if( m_LocText.GetLen() > 0 && m_LocText.GetString()[0] == '!' )
			{
				eg_string_big AsString = EGCrcDb::CrcToString( m_Info->TextNodeInfo.LocText );
				m_LocText = eg_loc_text(EGString_ToWide(EGString_Format( "LOCKEY:%s" , AsString.String() )));
			}
		}
	}
}
