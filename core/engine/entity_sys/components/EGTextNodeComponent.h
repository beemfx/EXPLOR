// (c) 2017 Beem Media

#pragma once

#include "EGVisualComponent.h"
#include "EGLocText.h"
#include "EGTextNode.h"
#include "EGAssetPath.h"
#include "EGFoundationTypes.h"
#include "EGTextNodeComponent.reflection.h"

class EGFontBase;

egreflect class EGTextNodeComponent : public egprop EGVisualComponent
{
	EG_CLASS_BODY( EGTextNodeComponent , EGVisualComponent )
	EG_FRIEND_RFL( EGTextNodeComponent )

friend class EGDefEdFile;

protected:

	egprop eg_real        m_Width               = 1.f;
	egprop eg_real        m_Height              = .1f;
	egprop eg_int         m_NumLines            = 1;
	egprop eg_color32     m_Color               = eg_color32::White;
	egprop eg_string_crc  m_LocText             = CT_Clear;
	egprop eg_d_string_ml m_LocText_enus        = "";
	egprop eg_text_align_ed m_Alignment         = eg_text_align_ed::LEFT;
	egprop eg_bool        m_bHasDropShadow      = true;
	egprop eg_color32     m_DropShadowColor     = eg_color32::Black;
	egprop eg_asset_path  m_Font                = eg_asset_path( eg_asset_path_special_t::Font , "Default" );
	egprop eg_bool        m_bDisableZTestAndZWrite = false;

protected:

	const EGTextNodeComponent* m_TextNodeDef;
	mutable EGTextNode         m_TextNode = CT_Default;
	EGFontBase*                m_CreatedFont;
	eg_loc_text                m_CreatedLocText;
	eg_real                    m_LineHeight;
	eg_text_align              m_Alignment1;

protected:

	// BEGIN EGEntComponent
	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	// virtual void OnDestruct() override;
	// virtual void Update( eg_real DeltaTime ) override;
	virtual void Draw( const eg_transform& ParentPose ) const override;
	virtual void DrawForTool( const eg_transform& ParentPose ) const override;

public:

	virtual void SetText( const class eg_loc_text& NewText ) override;
	virtual void SetPalette( const eg_vec4& NewPalette ) override { Super::SetPalette( NewPalette ); }

	void ConvertToBWNoShadow();

protected:

	virtual void QueryTextNodes( EGArray<eg_d_string>& Out ) const override;
	virtual void ScriptExec( const struct egTimelineAction& Action ) override;
	virtual void Reset() override;
	virtual void RefreshFromDef() override;
	// END EGEntComponent

	void DrawInternal( const eg_transform& ParentPose , eg_bool bForTool ) const;
	void UpdateLocTextForTool();};
