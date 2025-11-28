// (c) 2019 Beem Media

#include "EGWndDraw.h"
#include "EGWnd.h"

EGWndDc::EGWndDc( HDC& DcIn )
: m_Dc( DcIn )
{

}

EGWndDc::~EGWndDc()
{
	if( NULL != m_OriginalFont )
	{
		SelectObject( m_Dc , static_cast<HGDIOBJ>(m_OriginalFont) );
		m_OriginalFont = NULL;
	}

	if( NULL != m_OriginalBrush )
	{
		SelectObject( m_Dc , static_cast<HGDIOBJ>(m_OriginalBrush) );
		m_OriginalBrush = NULL;
	}
}

void EGWndDc::DcSetBkMode( eg_wnd_dc_bk_mode NewMode )
{
	m_BkMode = NewMode;
	switch( m_BkMode )
	{
	case eg_wnd_dc_bk_mode::Unknown:
	case eg_wnd_dc_bk_mode::Opaque:
		SetBkMode( m_Dc , OPAQUE );
		break;
	case eg_wnd_dc_bk_mode::Transparent:
		SetBkMode( m_Dc , TRANSPARENT );
		break;
	}
}

void EGWndDc::DcSetFont( EGWndDcFont& NewFont )
{
	m_Font = &NewFont;
	const eg_bool bSaveOriginalFont = NULL == m_OriginalFont;
	HFONT OldFont = static_cast<HFONT>(SelectObject( m_Dc , static_cast<HGDIOBJ>(NewFont.GetWindowsFont()) ));
	if( bSaveOriginalFont )
	{
		m_OriginalFont = OldFont;
	}
}

void EGWndDc::DcSetFont( egw_font_t NewFont )
{
	DcSetFont( EGWnd_GetDcFont( NewFont ) );
}

void EGWndDc::DcSetTextColor( const eg_color32& NewColor )
{
	m_TextColor = NewColor;
	SetTextColor( m_Dc , RGB(m_TextColor.R,m_TextColor.G,m_TextColor.B) );
}

void EGWndDc::DcSetTextColor( egw_color_t NewColor )
{
	DcSetTextColor( EGWnd_GetDcColor( NewColor ) );
}

void EGWndDc::DcSetBrush( EGWndDcBrush& NewBrush )
{
	const eg_bool bSaveOriginal = NULL == m_OriginalBrush;
	HBRUSH OldObject = static_cast<HBRUSH>( SelectObject( m_Dc, static_cast<HGDIOBJ>( NewBrush.GetWindowsBrush() ) ) );
	if( bSaveOriginal )
	{
		m_OriginalBrush = OldObject;
	}
}

void EGWndDc::DcSetBrush( egw_color_t NewBrush )
{
	DcSetBrush( EGWnd_GetDcBrush( NewBrush ) );
}

void EGWndDc::DcDrawFilledRect( const eg_recti& Area, EGWndDcBrush& Brush )
{
	const RECT rc = { Area.left , Area.top , Area.right , Area.bottom };
	FillRect( m_Dc , &rc , Brush.GetWindowsBrush() );
}

void EGWndDc::DcDrawFilledRect( const eg_recti& Area, egw_color_t Color )
{
	DcDrawFilledRect( Area, EGWnd_GetDcBrush( Color ) );
}

void EGWndDc::DcDrawRectangle( const eg_recti& Area )
{
	const RECT rc = { Area.left , Area.top , Area.right , Area.bottom };
	Rectangle( m_Dc , Area.left , Area.top , Area.right , Area.bottom );
}

void EGWndDc::DcDrawRectangle( const eg_recti& Area, EGWndDcBrush& Brush )
{
	DcSetBrush( Brush );
	DcDrawRectangle( Area );
}

void EGWndDc::DcDrawRectangle( const eg_recti& Area, egw_color_t Color )
{
	DcDrawRectangle( Area , EGWnd_GetDcBrush( Color ) );
}

void EGWndDc::DcDrawText( eg_cpstr16 Text, const eg_recti& Area, eg_flags Flags )
{
	RECT rc = { Area.left , Area.top , Area.right , Area.bottom };
	UINT Format = 0;
	if( Flags.IsSet( EGWND_DC_F_CENTER ) )
	{
		Format |= DT_CENTER;
	}
	if( Flags.IsSet( EGWND_DC_F_SINGLELINE ) )
	{
		Format |= DT_SINGLELINE;
	}
	if( Flags.IsSet( EGWND_DC_F_VCENTER ) )
	{
		Format |= DT_VCENTER;
	}
	if( Flags.IsSet( EGWND_DC_F_LEFT ) )
	{
		Format |= DT_LEFT;
	}
	if( Flags.IsSet( EGWND_DC_F_RIGHT ) )
	{
		Format |= DT_RIGHT;
	}
	DrawTextW( m_Dc , Text , -1 , &rc , Format );
	// Area.top = rc.top;
	// Area.bottom = rc.bottom;
	// Area.left = rc.left;
	// Area.right = rc.right;
}

void EGWndDcBrush::InitSolidBrush( const eg_color32& InColor )
{
	Deinit();

	m_Brush = CreateSolidBrush( RGB(InColor.R,InColor.G,InColor.B) );
}

void EGWndDcBrush::Deinit()
{
	if( NULL != m_Brush )
	{
		DeleteObject( m_Brush );
		m_Brush = NULL;
	}
}

void EGWndDcFont::InitFont( eg_cpstr16 InFace, eg_int InSize, eg_bool bBold, eg_bool bItalic )
{
	Deinit();

	m_Face = InFace;
	m_Size = InSize;
	m_bIsBold = bBold;
	m_bIsItalic = bItalic;

	LOGFONTW lf;
	zero( &lf );
	lf.lfHeight = m_Size;
	lf.lfWeight = m_bIsBold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = m_bIsItalic ? TRUE : FALSE;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	EGString_Copy( lf.lfFaceName , *m_Face , countof(lf.lfFaceName) );
	m_Font = CreateFontIndirectW( &lf );
}

void EGWndDcFont::InitFont( HFONT InFont )
{
	Deinit();

	m_Font = InFont;
	m_bDoesNotOwnFont = true;
}

void EGWndDcFont::Deinit()
{
	if( NULL != m_Font )
	{
		if( !m_bDoesNotOwnFont )
		{
			DeleteObject( m_Font );
		}
		m_Font = NULL;
	}
	m_Face.Clear();
	m_Size = 0;
	m_bIsBold = false;
	m_bIsItalic = false;
	m_bDoesNotOwnFont = false;
}
