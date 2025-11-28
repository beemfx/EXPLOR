// (c) 2017 Beem Media

#include "EGWndPropControl_Transform.h"

EGWndPropControl_Transform::EGWndPropControl_Transform( IWndPropControlOwner* Parent, const egPropControlInfo& Info )
: Super( Parent , Info )
, m_WndEditRotX( this )
, m_WndEditRotY( this )
, m_WndEditRotZ( this )
, m_WndEditPosX( this )
, m_WndEditPosY( this )
, m_WndEditPosZ( this )
{
	m_Height = FONT_SIZE;
	m_Transform = eg_transform::BuildIdentity();

	auto AddRotControl = [this]() -> void
	{
		static const eg_int EDIT_WIDTH=76;

		const eg_int TopOffset = m_Height;

		CreateStaticText( 2+EDIT_WIDTH*0 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About X" , m_Font );
		CreateStaticText( 2+EDIT_WIDTH*1 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About Y" , m_Font );
		CreateStaticText( 2+EDIT_WIDTH*2 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"About Z" , m_Font );

		SetupEditControl( m_WndEditRotX , 2+EDIT_WIDTH*0 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );
		SetupEditControl( m_WndEditRotY , 2+EDIT_WIDTH*1 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );
		SetupEditControl( m_WndEditRotZ , 2+EDIT_WIDTH*2 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH-25 , FONT_SIZE+8 , false , m_Font );
		m_WndEditRotX.SetCommitOnFocusLost( false );
		m_WndEditRotY.SetCommitOnFocusLost( false );
		m_WndEditRotZ.SetCommitOnFocusLost( false );
		m_WndEditRotX.SetUseDirtyState( false );
		m_WndEditRotY.SetUseDirtyState( false );
		m_WndEditRotZ.SetUseDirtyState( false );

		m_WndCommitRotX = CreateButtonControl( 2+EDIT_WIDTH*0 + (EDIT_WIDTH-25) , TopOffset + FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );
		m_WndCommitRotY = CreateButtonControl( 2+EDIT_WIDTH*1 + (EDIT_WIDTH-25) , TopOffset + FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );
		m_WndCommitRotZ = CreateButtonControl( 2+EDIT_WIDTH*2 + (EDIT_WIDTH-25) , TopOffset + FONT_SIZE*2 , 25 , FONT_SIZE+8 , L">" , m_Font );

		m_WndRotDisplay = CreateStaticText( 2 , TopOffset + FONT_SIZE*4 , EDIT_WIDTH*3 , FONT_SIZE , L"" , m_Font );

		m_WndCommitRotReset = CreateButtonControl( 2 , TopOffset + FONT_SIZE*5 , EDIT_WIDTH , FONT_SIZE+8 , L"Reset" , m_Font );
	
		m_Height += FONT_SIZE*6;
	};

	auto AddPosControl = [this]() -> void
	{
		const eg_int TopOffset = m_Height;

		static const eg_int EDIT_WIDTH=76;
		CreateStaticText( 2+EDIT_WIDTH*0 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"X" , m_Font );
		CreateStaticText( 2+EDIT_WIDTH*1 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"Y" , m_Font );
		CreateStaticText( 2+EDIT_WIDTH*2 , TopOffset + FONT_SIZE*1 , EDIT_WIDTH , FONT_SIZE , L"Z" , m_Font );

		SetupEditControl( m_WndEditPosX , 2+EDIT_WIDTH*0 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH , FONT_SIZE+8 , false , m_Font );
		SetupEditControl( m_WndEditPosY , 2+EDIT_WIDTH*1 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH , FONT_SIZE+8 , false , m_Font );
		SetupEditControl( m_WndEditPosZ , 2+EDIT_WIDTH*2 , TopOffset + FONT_SIZE*2 , EDIT_WIDTH , FONT_SIZE+8 , false , m_Font );

		m_Height += FONT_SIZE*4;
	};

	AddRotControl();
	AddPosControl();

	RefreshFromSourceData();
}

LRESULT EGWndPropControl_Transform::WndProc( UINT message , WPARAM wParam , LPARAM lParam )
{
	auto GetFloatValueFromWnd = []( EGWndTextNode& Wnd ) -> eg_real
	{
		eg_real Out = Wnd.GetText().ToFloat();
		// We also want to correct the text in case something invalid was entered:
		Wnd.SetText( EGString_Format( "%g" , Out ) ) ;
		return Out;
	};

	auto CommitChangesForWnd = [&GetFloatValueFromWnd,this]( HWND Wnd ) -> void
	{
		if( Wnd == m_WndEditRotX.GetWnd() )
		{
			eg_real Rot = GetFloatValueFromWnd( m_WndEditRotX );
			eg_transform RotTrans = eg_transform::BuildRotation( m_Transform.GetRotation() );
			RotTrans.RotateXThis( EG_Deg( Rot ) );
			m_Transform.SetRotation( RotTrans.GetRotation() );
			m_Transform.NormalizeRotationOfThis();
		}
		else if( Wnd == m_WndEditRotY.GetWnd() )
		{
			eg_real Rot = GetFloatValueFromWnd( m_WndEditRotY );
			eg_transform RotTrans = eg_transform::BuildRotation( m_Transform.GetRotation() );
			RotTrans.RotateYThis( EG_Deg( -Rot ) );
			m_Transform.SetRotation( RotTrans.GetRotation() );
			m_Transform.NormalizeRotationOfThis();
		}
		else if( Wnd == m_WndEditRotZ.GetWnd() )
		{
			eg_real Rot = GetFloatValueFromWnd( m_WndEditRotZ );
			eg_transform RotTrans = eg_transform::BuildRotation( m_Transform.GetRotation() );
			RotTrans.RotateZThis( EG_Deg( -Rot ) );
			m_Transform.SetRotation( RotTrans.GetRotation() );
			m_Transform.NormalizeRotationOfThis();
		}
		else if( Wnd == m_WndEditPosX.GetWnd() )
		{
			eg_real Value = GetFloatValueFromWnd( m_WndEditPosX );
			eg_vec3 NewTrans = m_Transform.GetTranslation();
			NewTrans.x = Value;
			m_Transform.SetTranslation( NewTrans );
		}
		else if( Wnd == m_WndEditPosY.GetWnd() )
		{
			eg_real Value = GetFloatValueFromWnd( m_WndEditPosY );
			eg_vec3 NewTrans = m_Transform.GetTranslation();
			NewTrans.y = Value;
			m_Transform.SetTranslation( NewTrans );
		}
		else if( Wnd == m_WndEditPosZ.GetWnd() )
		{
			eg_real Value = GetFloatValueFromWnd( m_WndEditPosZ );
			eg_vec3 NewTrans = m_Transform.GetTranslation();
			NewTrans.z = Value;
			m_Transform.SetTranslation( NewTrans );
		}
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

		if( m_WndCommitRotX == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditRotX.GetWnd() );
			return true;
		}
		else if( m_WndCommitRotY == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditRotY.GetWnd() );
			return true;
		}
		else if( m_WndCommitRotZ == ButtonHit )
		{
			CommitChangesForWnd( m_WndEditRotZ.GetWnd() );
			return true;
		}
		else if( m_WndCommitRotReset == ButtonHit )
		{
			m_Transform.SetRotation( eg_quat::I );
			CommitChangesToSourceData( ButtonHit );
			return true;
		}
	} break;
	}
	return Super::WndProc( message , wParam , lParam );
}

void EGWndPropControl_Transform::RefreshFromSourceData()
{
	if( m_ControlInfo.Editor )
	{
		m_Transform = *reinterpret_cast<const eg_transform*>(m_ControlInfo.Editor->GetData() );
	}
	
	SetWindowTextA( m_WndRotDisplay , EGString_Format( "Q: %g %g %g %g" , m_Transform.GetRotation().x , m_Transform.GetRotation().y , m_Transform.GetRotation().z , m_Transform.GetRotation().w ) );

	m_WndEditPosX.SetText( EGString_Format( "%g" , m_Transform.GetTranslation().x ) );
	m_WndEditPosY.SetText( EGString_Format( "%g" , m_Transform.GetTranslation().y ) );
	m_WndEditPosZ.SetText( EGString_Format( "%g" , m_Transform.GetTranslation().z ) );
}

void EGWndPropControl_Transform::CommitChangesToSourceData( HWND CommitControl )
{
	unused( CommitControl );

	if( m_ControlInfo.Editor )
	{
		*reinterpret_cast<eg_transform*>(m_ControlInfo.Editor->GetData()) = m_Transform;
		m_Parent->OnPropChangedValue( m_ControlInfo.Editor );
	}
	RefreshFromSourceData();
}
