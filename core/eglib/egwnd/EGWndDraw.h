// (c) 2019 Beem Media

#pragma once

#include "EGWndTypes.h"
#include "EGWindowsAPI.h"

class EGWndDc;
class EGWndDcBrush;
class EGWndDcFont;

enum class eg_wnd_dc_bk_mode
{
	Unknown,
	Opaque,
	Transparent,
};

EG_DECLARE_FLAG( EGWND_DC_F_CENTER , 1 );
EG_DECLARE_FLAG( EGWND_DC_F_SINGLELINE , 2 );
EG_DECLARE_FLAG( EGWND_DC_F_VCENTER , 3 );
EG_DECLARE_FLAG( EGWND_DC_F_LEFT , 4 );
EG_DECLARE_FLAG( EGWND_DC_F_RIGHT , 5 );

class EGWndDc
{
private:
	
	HDC&              m_Dc;
	eg_wnd_dc_bk_mode m_BkMode = eg_wnd_dc_bk_mode::Unknown;
	EGWndDcFont*      m_Font = nullptr;
	eg_color32        m_TextColor = eg_color32(0,0,0);
	HFONT             m_OriginalFont = NULL;
	HBRUSH            m_OriginalBrush = NULL;

public:

	EGWndDc( HDC& DcIn );
	~EGWndDc();

	HDC& GetWindowsDC() { return m_Dc; }

	void DcSetBkMode( eg_wnd_dc_bk_mode NewMode );
	void DcSetFont( EGWndDcFont& NewFont );
	void DcSetFont( egw_font_t NewFont );
	void DcSetTextColor( const eg_color32& NewColor );
	void DcSetTextColor( egw_color_t NewColor );
	void DcSetBrush( EGWndDcBrush& NewBrush );
	void DcSetBrush( egw_color_t NewBrush );

	void DcDrawFilledRect( const eg_recti& Area , EGWndDcBrush& Brush );
	void DcDrawFilledRect( const eg_recti& Area , egw_color_t Color );
	void DcDrawRectangle( const eg_recti& Area );
	void DcDrawRectangle( const eg_recti& Area , EGWndDcBrush& Brush );
	void DcDrawRectangle( const eg_recti& Area , egw_color_t Color );
	void DcDrawText( eg_cpstr16 Text , const eg_recti& Area , eg_flags Flags );
	void DcDrawText_CenteredSingleLine( eg_cpstr16 Text , const eg_recti& Area ) { DcDrawText( Text , Area , EGWND_DC_F_CENTER|EGWND_DC_F_SINGLELINE|EGWND_DC_F_VCENTER ); }
	void DcDrawText_LeftAlignedSingleLine( eg_cpstr16 Text , const eg_recti& Area ) { DcDrawText( Text , Area , EGWND_DC_F_SINGLELINE|EGWND_DC_F_VCENTER ); }
	void DcDrawText_RightAlignedSingleLine( eg_cpstr16 Text , const eg_recti& Area ) { DcDrawText( Text , Area , EGWND_DC_F_RIGHT|EGWND_DC_F_SINGLELINE|EGWND_DC_F_VCENTER ); }
};

class EGWndDcBrush
{
private:

	HBRUSH m_Brush = NULL;

public:

	EGWndDcBrush() = default;
	EGWndDcBrush( const eg_color32& InColor ) { InitSolidBrush( InColor ); }
	~EGWndDcBrush() { Deinit(); }

	void InitSolidBrush( const eg_color32& InColor );
	void Deinit();

	HBRUSH& GetWindowsBrush() { return m_Brush; }
};

class EGWndDcFont
{
private:

	HFONT         m_Font = NULL;
	eg_d_string16 m_Face;
	eg_int        m_Size = 0;
	eg_bool       m_bIsBold = false;
	eg_bool       m_bIsItalic = false;
	eg_bool       m_bDoesNotOwnFont = false;

public:

	EGWndDcFont() = default;
	EGWndDcFont( eg_cpstr16 InFace , eg_int InSize , eg_bool bBold , eg_bool bItalic ) { InitFont( InFace , InSize , bBold , bItalic ); }
	EGWndDcFont( HFONT InFont ) { InitFont( InFont ); } // TODO: RemoveThis
	~EGWndDcFont() { Deinit(); }

	void InitFont( eg_cpstr16 InFace , eg_int InSize , eg_bool bBold , eg_bool bItalic );
	void InitFont( HFONT InFont ); // TODO: RemoveThis
	void Deinit();

	HFONT& GetWindowsFont() { return m_Font; }
};
