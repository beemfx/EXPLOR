// (c) 2017 Beem Media

#include "EGWndPropControl_Anchor.h"
#include "EGWindowsAPI.h"

class EGWndPropControl_Anchor::EGInternalControl: public EGWndChildBase
{
private: typedef EGWndChildBase Super;

private:

	EGWndPropControl_Anchor*const m_Owner;
	HPEN   m_Pen;
	HBRUSH m_BgSelected;
	HBRUSH m_BgNormal;

public:

	EGInternalControl( EGWndPropControl_Anchor* Parent )
	: Super( Parent )
	, m_Owner( Parent )
	{
		m_Pen = CreateBasicPen( 2 , RGB(50,50,50) );
		m_BgNormal = CreateSolidBrush( RGB(255,255,255) );
		m_BgSelected = CreateSolidBrush( RGB(255,40,40) );
	}

	void Refresh()
	{
		FullRedraw();
	}

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam ) override final
	{
		switch( Msg )
		{
		case WM_LBUTTONDOWN:
		{
			RECT rcWnd;
			GetClientRect( m_hwnd, &rcWnd );

			const eg_int Width = rcWnd.right / 3;
			const eg_int Height = rcWnd.bottom / 3;

			POINT HitPoint = { GET_X_LPARAM( lParam ) , GET_Y_LPARAM( lParam ) };
			HitPoint.x = EG_Clamp<eg_int>( HitPoint.x / Width, 0, 2 );
			HitPoint.y = EG_Clamp<eg_int>( HitPoint.y / Height, 0, 2 );
			m_Owner->SetAnchorFromIndexPoint( HitPoint );
			Refresh();
		} break;
		default:
		{
			return Super::WndProc( Msg, wParam, lParam );
		}
		}
		return 0;
	}

	virtual void OnDrawBg( HDC hdc ) override
	{
		unused( hdc );
	}

	virtual void OnPaint( HDC hdc ) override
	{
		RECT rcWnd;
		GetClientRect( m_hwnd, &rcWnd );

		const eg_int Width = rcWnd.right / 3;
		const eg_int Height = rcWnd.bottom / 3;


		SetPen( hdc, m_Pen );
		SetBrush( hdc, m_BgNormal );

		POINT AnchorIndex = m_Owner->GetAnchorAsIndexPoint();

		for( eg_int x = 0; x < 3; x++ )
		{
			for( eg_int y = 0; y < 3; y++ )
			{
				eg_bool bIsAnchor = AnchorIndex.x == x && AnchorIndex.y == y;
				SetBrush( hdc, bIsAnchor ? m_BgSelected : m_BgNormal );

				Rectangle( hdc, x*Width, y*Height, x*Width + Width, y*Height + Height );
			}
		}
	}
};

EGWndPropControl_Anchor::EGWndPropControl_Anchor( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
, m_AnchorControl( nullptr )
{
	m_AnchorPos.X = eg_anchor_t::CENTER;
	m_AnchorPos.Y = eg_anchor_t::CENTER;

	m_AnchorControl = new EGInternalControl( this );
	static const eg_int ANCHOR_SIZE = 100;
	m_Height = FONT_SIZE + ANCHOR_SIZE+8;
	RECT rcPos;
	zero( &rcPos );
	rcPos.right = ANCHOR_SIZE;
	rcPos.bottom = ANCHOR_SIZE;
	OffsetRect( &rcPos , 10 , FONT_SIZE+4 );
	m_AnchorControl->SetWndRect( rcPos );

	RefreshFromSourceData();
}

EGWndPropControl_Anchor::~EGWndPropControl_Anchor()
{
	EG_SafeDelete( m_AnchorControl );
}

void EGWndPropControl_Anchor::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	if( m_ControlInfo.Editor )
	{
		*static_cast<eg_anchor*>(m_ControlInfo.Editor->GetData()) = m_AnchorPos;
		m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		RefreshFromSourceData();
	}
}

void EGWndPropControl_Anchor::RefreshFromSourceData()
{
	if( m_ControlInfo.Editor )
	{
		m_AnchorPos = *static_cast<eg_anchor*>(m_ControlInfo.Editor->GetData());
		m_AnchorControl->Refresh();
	}
}

POINT EGWndPropControl_Anchor::GetAnchorAsIndexPoint()
{
	POINT Out = { 0 , 0};
	switch( m_AnchorPos.X )
	{
	case eg_anchor_t::LEFT:
		Out.x = 0;
		break;
	case eg_anchor_t::CENTER:
		Out.x = 1;
		break;
	case eg_anchor_t::RIGHT:
		Out.x = 2;
		break;
	default:
		assert( false ); // X anchors cannot be this...
		break;
	}

	switch( m_AnchorPos.Y )
	{
	case eg_anchor_t::TOP:
		Out.y = 0;
		break;
	case eg_anchor_t::CENTER:
		Out.y = 1;
		break;
	case eg_anchor_t::BOTTOM:
		Out.y = 2;
		break;
	default:
		assert( false ); // X anchors cannot be this...
		break;
	}

	return Out;
}

void EGWndPropControl_Anchor::SetAnchorFromIndexPoint( const POINT& Point )
{
	eg_anchor Old = m_AnchorPos;

	switch( Point.x )
	{
	case 0:
		m_AnchorPos.X = eg_anchor_t::LEFT;
		break;
	case 1:
		m_AnchorPos.X = eg_anchor_t::CENTER;
		break;
	case 2:
		m_AnchorPos.X = eg_anchor_t::RIGHT;
		break;
	}

	switch( Point.y )
	{
	case 0:
		m_AnchorPos.Y = eg_anchor_t::TOP;
		break;
	case 1:
		m_AnchorPos.Y = eg_anchor_t::CENTER;
		break;
	case 2:
		m_AnchorPos.Y = eg_anchor_t::BOTTOM;
		break;
	}

	if( Old != m_AnchorPos )
	{
		CommitChangesToSourceData( m_AnchorControl->GetWnd() );
	}
}
