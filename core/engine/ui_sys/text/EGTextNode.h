// (c) 2015 Beem Media

#pragma once
#include "EGFont.h"
#include "EGLocText.h"

class EGTextNode
{
private:

	static const eg_loc_char WORD_SEPARATOR = ' ';
	static const eg_uint COLOR_STACK_SIZE = 3;

	static const eg_char STYLE_START_CHAR = '|';
	static const eg_char STYLE_END_CHAR = '|';

public:

	static void SetUseStencilIgnore( eg_bool bUse );
	static void SetUseDepthIgnore( eg_bool bUse );

public:

	EGTextNode( eg_ctor_t Ct );
	EGTextNode( const EGTextNode& rhs ) = delete;
	~EGTextNode();

	void PushColor( const eg_color& Color );
	void PopColor();
	void SetColor( const eg_color& Color );
	void SetupNode( class EGFontBase* Font , eg_real Width , eg_real Height , eg_real LineHeight );
	void SetAlignment( eg_text_align Alignment ){ m_Align = Alignment; }
	void SetWordWrapEnabled( eg_bool bNewValue ) { m_bWordWrapEnabled = bNewValue; }
	void ClearText();
	void ClearAndFreeMem();
	void SetText( const eg_loc_text& LocText );

	void SetAllowFormatting( bool Allow ){ m_AllowFormatting = Allow; }
	void SetAllowStyles( bool Allow ){ m_AllowStyles = Allow; }

	void Draw();
	void DrawShadow( const eg_color& Color );
	void DrawDebugBorder();

	eg_vec2 GetFontShadowOffset() const;

	static void RegisterDefaultColors();
	static void RegisterColor( eg_string_crc Crc , eg_color32 Color );


private:

	void FormatText( const eg_loc_char Text[] , eg_size_t Length );
	void FormatText_AddWord( const eg_loc_char Text[] , eg_size_t Length , eg_bool NoSpace );
	void FormatText_AddWord_HandleJustify( eg_size_t LastChar , eg_size_t LastGlyph );
	eg_size_t FormatText_HandleStyle( const eg_loc_char Text[] , eg_size_t Length, eg_real& EndPos );
	void FormatText_SquishSingleLine();
	void FormatText_SquishHeight();

private:

	class EGFontBase* m_CharFont;
	class EGGlyphFont* m_GlpyhFont;
	EGArray<egv_vert_simple> m_VChars;
	EGArray<egv_vert_simple> m_VGlyphs;
	EGArray<egv_vert_simple> m_DrawnChars;
	EGArray<egv_vert_simple> m_HandleStyleDrawnChars;
	EGArray<eg_loc_char> m_Text;
	EGFont::egTextFormat m_TextFormat;
	eg_size_t m_LastLineStart;
	eg_size_t m_LastGlyphLineStart;
	eg_real m_LastX;
	eg_real m_LastY;
	eg_color32 m_ColorStack[COLOR_STACK_SIZE];
	eg_uint   m_ColorStackPos;
	eg_text_align m_Align;
	eg_bool   m_AllowFormatting:1;
	eg_bool   m_AllowStyles:1;
	eg_bool   m_bSingleLine:1;
	eg_bool   m_bWordWrapEnabled:1;

private:

	static const eg_size_t MAX_REGISTERED_COLORS = 32;
	static EGStringCrcMapFixedSize<eg_color32,MAX_REGISTERED_COLORS> s_ColorMap;
	static eg_int g_UseStencilIgnore;
	static eg_int g_UseDepthIgnore;
};