// (c) 2016 Beem Media. All Rights Reserved.

#include "ExHintBarWidget.h"
#include "EGTextFormat.h"
#include "EGInput.h"
#include "ExInput.h"

EG_CLASS_DECL( ExHintBarWidget )

void ExHintBarWidget::ClearHints()
{
	m_Hints.Clear();
	UpdateText();
}

void ExHintBarWidget::AddHint( eg_cmd_t Cmd , const eg_loc_text& Text )
{
	// First replace the hint for this command if it already exists.
	for( exHintInfo& Hint : m_Hints )
	{
		if( Hint.Cmd == Cmd )
		{
			Hint.Text = Text;
			UpdateText();
			return;
		}
	}
	
	// Otherwise add the new hint.
	exHintInfo NewHint;
	NewHint.Cmd = Cmd;
	NewHint.Text = Text;
	m_Hints.Append( NewHint );
	UpdateText();
}

void ExHintBarWidget::UpdateText()
{
	eg_loc_text Text( CT_Clear );

	for( const exHintInfo& Hint : m_Hints )
	{
		eg_loc_text HintText;

		if( Hint.Cmd == CMDA_EX_MENU_LEFTRIGHT )
		{
			HintText = EGFormat( L"|Glyph(MENU_LEFTRIGHT)| {0}",Hint.Text.GetString() );
		}
		else
		{
			eg_cpstr ButtonGlyph = EGInput::Get().InputCmdToName( Hint.Cmd );

			if( ButtonGlyph && ButtonGlyph[0] != '\0' )
			{
				HintText = EGFormat( L"|Glyph({0})| {1}", eg_loc_text(EGString_ToWide(ButtonGlyph)).GetString() , Hint.Text.GetString() );
			}
			else
			{
				HintText = Hint.Text;
			}
		}

		Text = EGFormat( L"{0}{1} " , Text.GetString() , HintText.GetString() );
	}

	SetText( eg_crc("") , Text );
}
