// (c) 2017 Beem Media

#include "EGWndPropControl_Rot.h"

EGWndPropControl_Rot::EGWndPropControl_Rot( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
, m_WndEditX( this )
, m_WndEditY( this )
, m_WndEditZ( this )
{
	m_Height = FONT_SIZE*7;
	m_Quat = eg_quat::I;

	static const eg_int EDIT_WIDTH=76;
	CreateStaticText( 2+EDIT_WIDTH*0 , FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About X" , m_Font );
	CreateStaticText( 2+EDIT_WIDTH*1 , FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About Y" , m_Font );
	CreateStaticText( 2+EDIT_WIDTH*2 , FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About Z" , m_Font );

	SetupEditControl( m_WndEditX , 2+EDIT_WIDTH*0 , FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );
	SetupEditControl( m_WndEditY , 2+EDIT_WIDTH*1 , FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );
	SetupEditControl( m_WndEditZ , 2+EDIT_WIDTH*2 , FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );

	m_WndCommitX = CreateButtonControl( 2+EDIT_WIDTH*0 + (EDIT_WIDTH-25) , FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );
	m_WndCommitY = CreateButtonControl( 2+EDIT_WIDTH*1 + (EDIT_WIDTH-25) , FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );
	m_WndCommitZ = CreateButtonControl( 2+EDIT_WIDTH*2 + (EDIT_WIDTH-25) , FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );

	m_WndQuatDisplay = CreateStaticText( 2 , FONT_SIZE*4 , EDIT_WIDTH*3 , FONT_SIZE , L"" , m_Font );

	m_WndCommitReset = CreateButtonControl( 2 , FONT_SIZE*5 , EDIT_WIDTH , FONT_SIZE+8 , L"Reset" , m_Font );

	RefreshFromSourceData();
}

LRESULT EGWndPropControl_Rot::WndProc( UINT message , WPARAM wParam , LPARAM lParam )
{
	auto GetRotationFromWnd = []( EGWndTextNode& Wnd ) -> eg_real
	{
		eg_real Out = Wnd.GetText().ToFloat();
		// We also want to correct the text in case something invalid was entered:
		Wnd.SetText( EGString_Format( "%g" , Out ) ) ;
		return Out;
	};

	auto CommitChangesForWnd = [&GetRotationFromWnd,this]( HWND Wnd ) -> void
	{
		eg_transform RotTrans = eg_transform::BuildRotation( m_Quat );
		if( Wnd == m_WndEditX.GetWnd() )
		{
			eg_real Rot = GetRotationFromWnd( m_WndEditX );
			RotTrans.RotateXThis( EG_Deg( Rot ) );
		}
		else if( Wnd == m_WndEditY.GetWnd() )
		{
			eg_real Rot = GetRotationFromWnd( m_WndEditY );
			RotTrans.RotateYThis( EG_Deg( -Rot ) );
		}
		else if( Wnd == m_WndEditZ.GetWnd() )
		{
			eg_real Rot = GetRotationFromWnd( m_WndEditZ );
			RotTrans.RotateZThis( EG_Deg( -Rot ) );
		}
		m_Quat = RotTrans.GetRotation();
		m_Quat.NormalizeThis();
		CommitChangesToSourceData( Wnd );
	};

	switch( message )
	{
	case WM_EG_COMMIT:
	{
		HWND WndCommit = reinterpret_cast<HWND>( lParam );
		CommitChangesForWnd( WndCommit );
		Edit_SetSel( WndCommit , 0 , -1 );
	} break;
	case WM_COMMAND:
	{
		HWND ButtonHit = reinterpret_cast<HWND>(lParam);

		if( m_WndCommitX == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditX.GetWnd() );
			return true;
		}
		else if( m_WndCommitY == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditY.GetWnd() );
			return true;
		}
		else if( m_WndCommitZ == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditZ.GetWnd() );
			return true;
		}
		else if( m_WndCommitReset == ButtonHit )
		{
			m_Quat = eg_quat::I;
			CommitChangesToSourceData( ButtonHit );
			return true;
		}
	} break;
	}
	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Rot::RefreshFromSourceData()
{
	m_Quat = m_ControlInfo.Editor ? *static_cast<eg_quat*>(m_ControlInfo.Editor->GetData()) : CT_Default;
	SetWindowTextA( m_WndQuatDisplay , EGString_Format( "Q: %g %g %g %g" , m_Quat.x , m_Quat.y , m_Quat.z , m_Quat.w ) );
}

void EGWndPropControl_Rot::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	if( m_ControlInfo.Editor )
	{
		*static_cast<eg_quat*>(m_ControlInfo.Editor->GetData()) = m_Quat;
		m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
		RefreshFromSourceData();
	}
}
