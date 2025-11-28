// (c) 2016 Beem Media

#pragma once

#include "EGUiWidget.h"
#include "EGTextNode.h"

class EGUiTextWidget: public EGUiWidget
{
	EG_CLASS_BODY( EGUiTextWidget , EGUiWidget )

protected:

	EGTextNode        m_TextNode = CT_Default;
	eg_real           m_TextWidth;
	eg_real           m_TextHeight;
	eg_real           m_LineHeight;
	eg_color          m_TextColor;
	eg_loc_text       m_LocText; 
	eg_text_align     m_Alignment;
	eg_bool           m_bHasDropShadow = true;
	eg_color          m_DropShadowColor = eg_color(0.f,0.f,0.f,1.f);
	class EGFontBase* m_Font;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	virtual void Draw( eg_real AspectRatio ) override;
	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override;
	virtual eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj )const override;

	//
	// ISdkMenuObj Interface:
	//
	virtual void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText ) override;
	virtual void SetPalette( eg_uint PaletteIndex , const eg_vec4& Palette ) override;
	
	void SetWordWrapEnabled( eg_bool bNewValue );
	void SetColorAlpha( eg_real NewValue );

private:
	
	void UpdateLocTextForTool();
};