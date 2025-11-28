// (c) 2017 Beem Media

#include "EGWndPropControl_Color.h"
#include "EGWndHelpers.h"

EGWndPropControl_Color::EGWndPropControl_Color( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
, m_EditAlpha( this )
{
	m_bDrawDirtyControls = true;
	m_Height = FONT_SIZE*4;

	SetupEditControl( m_EditAlpha , 80 , FONT_SIZE*2 , 76 , FONT_SIZE+8 , false , m_Font );

	zero( &m_ColorChooseColors );
	m_ColorChooseColors[0]  = RGB(255,0,0);
	m_ColorChooseColors[1]  = RGB(0,255,0);
	m_ColorChooseColors[2]  = RGB(0,0,255);
	m_ColorChooseColors[3]  = RGB(255,255,0);
	m_ColorChooseColors[4]  = RGB(255,0,255);
	m_ColorChooseColors[5]  = RGB(0,255,255);
	m_ColorChooseColors[6]  = RGB(255,255,255);
	m_ColorChooseColors[7]  = RGB(0,0,0);
	m_ColorChooseColors[8]  = RGB(255,165,0);
	m_ColorChooseColors[9]  = RGB(75,0,130);
	m_ColorChooseColors[10] = RGB(139,69,19);
	m_ColorChooseColors[11] = RGB(112,128,144);
	m_ColorChooseColors[12] = RGB(105,105,105);
	m_ColorChooseColors[13] = RGB(128,128,128);
	//static const eg_color32 ExColor_HeaderYellow(252,252,84);
	//static const eg_color32 ExColor_HeaderMinor(252,196,180);
	//m_ColorChooseColors[14] = RGB(169,169,169);
	//m_ColorChooseColors[15] = RGB(192,192,192);
	m_ColorChooseColors[14] = RGB(252,252,84);
	m_ColorChooseColors[15] = RGB(252,196,180);

	RefreshFromSourceData();
}

EGWndPropControl_Color::~EGWndPropControl_Color()
{
	if( m_ColorBrush )
	{
		DeleteObject( m_ColorBrush );
		m_ColorBrush = nullptr;
	}
}

LRESULT EGWndPropControl_Color::WndProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_EG_COMMIT_ON_FOCUS_LOST:
	case WM_EG_COMMIT:
	{
		HWND CommitWnd = reinterpret_cast<HWND>(lParam);
		if( CommitWnd == m_EditAlpha.GetWnd() )
		{
			m_Color.a = EG_Clamp( m_EditAlpha.GetText().ToFloat() , 0.f , 1.f );
			CommitChangesToSourceData( CommitWnd );
			return 0;
		}
	} break;
	case WM_CANCELMODE:
	{
		m_MouseCapture.OnCancelMode();
	} break;
	case WM_LBUTTONDOWN:
	{
		POINT HitPoint;
		HitPoint.x = GET_X_LPARAM(lParam); 
		HitPoint.y = GET_Y_LPARAM(lParam);


		if( PtInRect( &m_rcColorButton,HitPoint) )
		{
			m_MouseCapture.OnBeginCapture( GetWnd() , HitPoint.x , HitPoint.y );
		}

	} return 0;
	case WM_LBUTTONUP:
	{
		POINT HitPoint;
		HitPoint.x = GET_X_LPARAM(lParam); 
		HitPoint.y = GET_Y_LPARAM(lParam);

		if( m_MouseCapture.IsCapturing() )
		{
			if( PtInRect( &m_rcColorButton , HitPoint ) )
			{
				eg_bool bGotColor = WGWndHelper_ChooseColor( GetWnd() , m_Color , m_ColorChooseColors );
				if( bGotColor )
				{
					CommitChangesToSourceData( GetWnd() );
				}
			}

			m_MouseCapture.OnEndCapture();
		}

	} return 0;
	}

	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Color::OnPaint( HDC hdc )
{
	Super::OnPaint( hdc );

	SetFont( hdc , EGWnd_GetFont( egw_font_t::SMALL_BOLD ) );

	RECT rcAlphaText = { 80 , 1 + 1*FONT_SIZE , 78+100 , FONT_SIZE*2 };
	DrawTextW( hdc , L"Alpha" , -1 , &rcAlphaText , DT_SINGLELINE );
	TextOutW( hdc , 1 , 1 + FONT_SIZE , L"Color" , 5 );

	SetBrush( hdc , m_ColorBrush );
	Rectangle( hdc , m_rcColorButton.left , m_rcColorButton.top , m_rcColorButton.right , m_rcColorButton.bottom );
	SetBrush( hdc , WHITE_BRUSH );
}

void EGWndPropControl_Color::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	if( m_ControlInfo.Editor )
	{
		eg_color32* AsColor32 = reinterpret_cast<eg_color32*>(m_ControlInfo.Editor->GetData());
		*AsColor32 = eg_color32(m_Color);
	}
	RefreshFromSourceData();
	m_bIsDirty = false;
}

void EGWndPropControl_Color::RefreshFromSourceData()
{
	if( m_ControlInfo.Editor )
	{
		const eg_color32* AsColor32 = reinterpret_cast<const eg_color32*>(m_ControlInfo.Editor->GetData());
		m_Color = eg_color(*AsColor32);
		m_EditAlpha.SetText( EGString_Format("%g",AsColor32->A/255.f) );
	}
	UpdateBrush( m_Color );
	FullRedraw();
}

void EGWndPropControl_Color::UpdateBrush( const eg_color& NewColor )
{
	if( m_ColorBrush )
	{
		DeleteObject( m_ColorBrush );
		m_ColorBrush = nullptr;
	}

	eg_color32 AsColor32( NewColor );
	m_ColorBrush = CreateSolidBrush( RGB(AsColor32.R,AsColor32.G,AsColor32.B) );
}
