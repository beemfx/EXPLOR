// (c) 2017 Beem Media

#include "EGTextNodeComponent.h"
#include "EGFont.h"
#include "EGRenderer.h"
#include "EGTextNode.h"
#include "EGTimelineTypes.h"
#include "EGEngine.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGTextNodeComponent )

void EGTextNodeComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );
	m_TextNodeDef = EGCast<EGTextNodeComponent>( InitData.Def );
	RefreshFromDef();
}

void EGTextNodeComponent::Draw( const eg_transform& ParentPose ) const
{
	DrawInternal( ParentPose , false );
}

void EGTextNodeComponent::DrawForTool( const eg_transform& ParentPose ) const
{
	DrawInternal( ParentPose , true );
}

void EGTextNodeComponent::SetText( const class eg_loc_text& NewText )
{
	m_CreatedLocText = NewText;
}

void EGTextNodeComponent::ConvertToBWNoShadow()
{
	m_Color = eg_color32::Black;
	m_bHasDropShadow = false;
}

void EGTextNodeComponent::QueryTextNodes( EGArray<eg_d_string>& Out ) const
{
	Out.Append( EGCrcDb::CrcToString(GetId()).String() );
}

void EGTextNodeComponent::ScriptExec( const struct egTimelineAction& Action )
{
	Super::ScriptExec( Action );
}

void EGTextNodeComponent::Reset()
{
	Super::Reset();

	m_Palette.SetValue( eg_color(m_TextNodeDef->m_Color).ToVec4() );
	m_CreatedLocText = eg_loc_text( m_TextNodeDef->m_LocText );
}

void EGTextNodeComponent::RefreshFromDef()
{
	Super::RefreshFromDef();

	if( m_TextNodeDef )
	{
		m_CreatedFont = EGFontMgr::Get()->GetFont( EGCrcDb::StringToCrc(*m_TextNodeDef->m_Font.Path) );
		m_Width = m_TextNodeDef->m_Width;
		m_Height = m_TextNodeDef->m_Height;
		m_NumLines = m_TextNodeDef->m_NumLines;
		m_LineHeight = m_TextNodeDef->m_Height / EG_Max<eg_uint>( m_TextNodeDef->m_NumLines , 1 );
		m_Palette.SetValue( eg_color(m_TextNodeDef->m_Color).ToVec4() );
		m_CreatedLocText = eg_loc_text( m_TextNodeDef->m_LocText );
		m_Alignment = m_TextNodeDef->m_Alignment;
		m_Alignment1 = static_cast<eg_text_align>(m_TextNodeDef->m_Alignment);
		m_bHasDropShadow = m_TextNodeDef->m_bHasDropShadow;
		m_DropShadowColor = m_TextNodeDef->m_DropShadowColor;
		m_bDisableZTestAndZWrite = m_TextNodeDef->m_bDisableZTestAndZWrite;

		UpdateLocTextForTool();
	}
}

void EGTextNodeComponent::DrawInternal( const eg_transform& ParentPose , eg_bool bForTool ) const
{
	if( m_bIsHidden )
	{
		return;
	}

	if( m_bDisableZTestAndZWrite )
	{
		EGTextNode::SetUseDepthIgnore( true );
	}

	eg_color TextColor(m_Palette.GetCurrentValue());

	//Scale size of text node.
	eg_vec4 Scale = m_CachedScale.GetCurrentValue() * m_GlobalScale;
	m_TextNode.SetupNode( m_CreatedFont , m_Width*Scale.x , m_Height*Scale.y , m_LineHeight*Scale.y );
	m_TextNode.SetColor( TextColor );
	m_TextNode.SetAlignment( m_Alignment1 );
	m_TextNode.SetText( m_CreatedLocText );

	eg_transform Pose = m_Pose.GetCurrentValue();
	Pose.ScaleTranslationOfThisNonUniformly( Scale.ToVec3() );
	Pose *= ParentPose;

	auto DrawPass = [this,&Pose,&bForTool,&Scale,&TextColor]( const eg_vec2& Offset , eg_bool bShadowColor , eg_bool bDebugBorder = false ) -> void
	{
		eg_transform OffsetPose( CT_Default );
		OffsetPose.TranslateThis( Offset.x , Offset.y , 0.f );
		eg_transform FinalPose = OffsetPose * Pose;
		MainDisplayList->SetWorldTF( eg_mat(FinalPose) );

		if( bShadowColor )
		{
			eg_color FinalShadowColor = eg_color(m_DropShadowColor);
			FinalShadowColor.a *= TextColor.a;
			m_TextNode.DrawShadow( FinalShadowColor );
		}
		else
		{
			m_TextNode.Draw();
		}

		if( bDebugBorder && ( DebugConfig_DrawTextNodes.GetValue() || bForTool ) )
		{
			m_TextNode.DrawDebugBorder();
		}
	};

	if( m_bHasDropShadow )
	{
		DrawPass( m_TextNode.GetFontShadowOffset() , true );
	}
	DrawPass( eg_vec2(0.f,0.f) , false , true );

	if( m_bDisableZTestAndZWrite )
	{
		EGTextNode::SetUseDepthIgnore( false );
	}
}

void EGTextNodeComponent::UpdateLocTextForTool()
{
	if( Engine_IsTool() && m_TextNodeDef )
	{
		if( m_TextNodeDef->m_LocText_enus.Len() > 0 )
		{
			m_CreatedLocText = eg_loc_text(EGString_ToWide(*m_TextNodeDef->m_LocText_enus));
		}
		else
		{
			m_CreatedLocText = eg_loc_text( m_TextNodeDef->m_LocText );
			if( m_CreatedLocText.GetLen() > 0 && m_CreatedLocText.GetString()[0] == '!' )
			{
				eg_string_big AsString = EGCrcDb::CrcToString( m_TextNodeDef->m_LocText );
				m_CreatedLocText = eg_loc_text(EGString_ToWide(EGString_Format( "LOCKEY:%s" , AsString.String() )));
			}
		}
	}
}
