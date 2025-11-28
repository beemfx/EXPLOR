// (c) 2016 Beem Media

#include "EGSmEdNodeWnd.h"
#include "EGSmEd.h"
#include "EGSmEdResources.h"
#include "../EGEdResLib/resource.h"
#include "EGSmEdVarMgr.h"
#include "EGResourceLib.h"
#include "EGFileData.h"

EGSmEdNodeWnd::EGSmEdNodeWnd( ISmEditor* ConvEditor , ISmEdApp* App , eg_size_t StateIndex )
: EGWndChildBase( ConvEditor->GetOwnerWnd() )
, m_App( App )
, m_ConvEditor( ConvEditor )
, m_StateIndex( StateIndex )
{
	LOGBRUSH BgBrush;
	zero( &BgBrush );
	BgBrush.lbColor = BG_COLOR;
	BgBrush.lbStyle = BS_SOLID;
	BgBrush.lbHatch = HS_CROSS;
	m_BgBrush = CreateBrushIndirect( &BgBrush );

	BgBrush.lbColor = TERMINAL_BOX_COLOR;
	m_TerminalBrush = CreateBrushIndirect( &BgBrush );

	m_Font = EGWnd_GetFont( egw_font_t::SMALL );
	m_FontHeader = EGWnd_GetFont( egw_font_t::SMALL_BOLD );

	m_BorderPen = CreatePen( PS_SOLID, 1, RGB(0,0,0) );
	m_FocusPen = CreatePen( PS_DOT, 1, EGWnd_GetColor(egw_color_t::DEFAULT) );
	m_HeaderBorderPen = CreatePen( PS_SOLID, EGSmNodeConsts::GetHeaderStartBorderSize() , HEADER_START_BORDER_COLOR );

	LONG Height = static_cast<LONG>(EGSmNodeConsts::GetStateHeight() + EGSmNodeConsts::GetChoiceHeight()*(GetMyNode().Branches.Len()));

	POINT AdjustedPos;
	AdjustedPos.x = EGWnd_GetAppScaledSize( static_cast<int>(GetMyNode().EditorPos.x) );
	AdjustedPos.y = EGWnd_GetAppScaledSize( static_cast<int>(GetMyNode().EditorPos.y) );
	SetWindowPos( m_hwnd , NULL , AdjustedPos.x , AdjustedPos.y , EGSmNodeConsts::GetStateWidth() , Height , 0 );

	UpdateView();

	if( GetMyNode().Type == egsm_node_t::NATIVE_EVENT )
	{
		m_NativeEventDecl = EGSmEdVarMgr_GetFunctionInfo( eg_string_crc(GetMyNode().Parms[0]) );
	}
	else
	{
		m_NativeEventDecl.DeclType = egsm_var_decl_t::UNK;
		m_NativeEventDecl.ReturnType = egsm_var_t::UNK;
	}

	FullRedraw();
}

EGSmEdNodeWnd::~EGSmEdNodeWnd()
{
	DeleteObject( m_BorderPen );
	DeleteObject( m_FocusPen );
	DeleteObject( m_BgBrush );
	DeleteObject( m_TerminalBrush );
	DeleteObject( m_HeaderBorderPen );
}

const egsmNodeScr& EGSmEdNodeWnd::GetMyNode() const
{
	return EGSmEd_GetFile().GetNode( m_StateIndex );
}

egsmNodeScr& EGSmEdNodeWnd::GetMyNode()
{
	return EGSmEd_GetFile().GetNode( m_StateIndex );
}

eg_size_t EGSmEdNodeWnd::YHitToIndex( LONG y )
{
	eg_size_t Out = 0;
	if( y > EGSmNodeConsts::GetStateHeight() )
	{
		Out = ( y - EGSmNodeConsts::GetStateHeight() ) / EGSmNodeConsts::GetChoiceHeight() + 1;
	}
	return Out;
}

void EGSmEdNodeWnd::OnWmMouseMove( const eg_ivec2& MousePos )
{
	Super::OnWmMouseMove( MousePos );

	if( IsCapturing() )
	{
		if( m_CaptureReason == eg_capture_reason::Moving )
		{
			POINT MoveAmount = { MousePos.x - m_LastPos.x , MousePos.y - m_LastPos.y };

			GetMyNode().EditorPos.x += EG_Clamp<eg_real>( static_cast<eg_real>(MoveAmount.x) , -100.f , 100.f );
			GetMyNode().EditorPos.y += EG_Clamp<eg_real>( static_cast<eg_real>(MoveAmount.y) , -100.f , 100.f );

			RECT rcPrev;
			GetClientRect( m_hwnd , &rcPrev );

			UpdateView();
			m_ConvEditor->GetOwnerWnd()->FullRedraw();
			if( MoveAmount.x != 0 || MoveAmount.y != 0 )
			{
				m_ConvEditor->SetDirty();
			}
		}
		else if( m_CaptureReason == eg_capture_reason::Connecting )
		{
			RECT rcClient = GetViewRect();
			POINT Pt = { MousePos.x , MousePos.y };
			if( !PtInRect( &rcClient , Pt ) )
			{
				m_bDragHasLeftBounds = true;
			}

			if( m_bDragHasLeftBounds )
			{
				POINT GlobalMousePos = { MousePos.x,MousePos.y};
				ClientToScreen( GetWnd() , &GlobalMousePos );
				eg_size_t HitIndex = m_ConvEditor->SetDragHitMousePos( eg_ivec2(GlobalMousePos.x,GlobalMousePos.y) );
			}
		}
	}
	else
	{
		eg_bool bCanHit = m_App->GetFocusedNode().StateIndex != m_StateIndex || MousePos.y < EGSmNodeConsts::GetStateHeight();
		if( bCanHit )
		{
			m_ConvEditor->SetDragHitNode( m_StateIndex );
		}
	}
}

void EGSmEdNodeWnd::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonDown( MousePos );

	ISmEdApp::egFocus Focus( CT_Clear );
	Focus.StateIndex = m_StateIndex;

	POINT HitPos = { MousePos.x , MousePos.y };
	eg_size_t HitIndex = YHitToIndex(HitPos.y);
	m_LastPos = HitPos;
	if( 0 == HitIndex )
	{
		m_CaptureReason = eg_capture_reason::Moving;
		BeginCapture( eg_ivec2(HitPos.x,HitPos.y) , EGSmEdResources_GetCursor( egsm_cursor_t::GRAB ) );
	}
	else
	{
		m_bDragHasLeftBounds = false;
		Focus.ChoiceIndex = static_cast<eg_uint>((HitIndex-1));
		m_CaptureReason = eg_capture_reason::Connecting;
		BeginCapture( eg_ivec2(HitPos.x,HitPos.y) );
		m_ConvEditor->StartDrag( m_StateIndex , EG_Clamp<eg_size_t>( HitIndex-1 , 0 , GetMyNode().Branches.Len() ) );
	}

	m_App->SetFocusedNode( Focus );

	SetWindowPos( m_hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOMOVE );
	FullRedraw();
}

void EGSmEdNodeWnd::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmLButtonUp( MousePos );

	if( IsCapturing() )
	{
		EndCapture( MousePos );

		if( m_CaptureReason == eg_capture_reason::Connecting )
		{
			POINT GlobalMousePos = { MousePos.x,MousePos.y};
			ClientToScreen( GetWnd() , &GlobalMousePos );
			eg_size_t HitIndex = m_ConvEditor->SetDragHitMousePos( eg_ivec2(GlobalMousePos.x,GlobalMousePos.y) );
			
			eg_bool bCanHit = HitIndex != EGSM_INVALID_INDEX;

			if( !bCanHit )
			{
				RECT rcClient = GetViewRect();
				POINT Pt = { MousePos.x , MousePos.y };
				if( !PtInRect( &rcClient , Pt ) )
				{
					bCanHit = true;
				}
			}

			m_ConvEditor->EndDrag( HitIndex , !bCanHit );
		}
	}
}

void EGSmEdNodeWnd::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonDown( MousePos );

	ISmEdApp::egFocus Focus( CT_Clear );
	Focus.StateIndex = m_StateIndex;

	POINT HitPos = { MousePos.x , MousePos.y };
	eg_size_t HitIndex = YHitToIndex(HitPos.y);
	m_LastPos = HitPos;
	if( 0 == HitIndex )
	{

	}
	else
	{
		Focus.ChoiceIndex = static_cast<eg_uint>((HitIndex-1));
	}

	m_App->SetFocusedNode( Focus );

	SetWindowPos( m_hwnd , HWND_TOP , 0 , 0 , 0 , 0 , SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOMOVE );
	FullRedraw();
}

void EGSmEdNodeWnd::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	Super::OnWmRButtonUp( MousePos );

	POINT HitPos = { MousePos.x , MousePos.y };

	m_EditIndex = YHitToIndex(HitPos.y);
	GetCursorPos( &HitPos );

	int SubMenuIndex = 1;

	if( 0 == m_EditIndex )
	{
		switch( GetMyNode().Type )
		{
		case egsm_node_t::ENTRY_POINT:
			SubMenuIndex =  GetMyNode().Id == EGSmEd_GetFile().GetMachineProps().DefaultEntryPointNode ? 6 : 7;
			break;
		case egsm_node_t::CALL:
			// These types don't let you change the number of Branches.
			SubMenuIndex = 3;
			break;
		case egsm_node_t::NATIVE_EVENT:
		{
			if( m_NativeEventDecl.ReturnType == egsm_var_t::TERMINAL || m_NativeEventDecl.ReturnType == egsm_var_t::BOOL || m_NativeEventDecl.ReturnType == egsm_var_t::RETURN_VOID )
			{
				SubMenuIndex = 3;
			}
			else
			{
				SubMenuIndex = 1;
			}
		} break;
		default:
			SubMenuIndex = 1;
			break;
		}
	}
	else
	{
		switch( GetMyNode().Type )
		{
		case egsm_node_t::ENTRY_POINT:
		case egsm_node_t::CALL:
			// These types don't let you change the number of Branches.
			SubMenuIndex = 4;
			break;
		case egsm_node_t::NATIVE_EVENT:
		{
			if( m_NativeEventDecl.ReturnType == egsm_var_t::BOOL || m_NativeEventDecl.ReturnType == egsm_var_t::RETURN_VOID )
			{
				SubMenuIndex = 4;
			}
			else if( GetMyNode().Branches.Len() == 1 )
			{
				SubMenuIndex = 4;
			}
			else
			{
				SubMenuIndex = 2;
			}
		} break;
		default:
		{
			SubMenuIndex = GetMyNode().Branches.Len() == 1 ? 4 : 2;
		}	break;
		}
	}

	TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGCE_CONTEXTMENU" ) , SubMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , m_hwnd , nullptr ); 
}

void EGSmEdNodeWnd::UpdateView()
{
	POINT Pos = { m_ConvEditor->GetView().x + static_cast<LONG>( GetMyNode().EditorPos.x ) , m_ConvEditor->GetView().y + static_cast<LONG>( GetMyNode().EditorPos.y ) };
	Pos.x = EGWnd_GetAppScaledSize( Pos.x );
	Pos.y = EGWnd_GetAppScaledSize( Pos.y );
	SetWindowPos( m_hwnd, HWND_TOP, Pos.x, Pos.y, 0, 0, SWP_NOSIZE );
}

void EGSmEdNodeWnd::OnPaint( HDC hdc )
{
	eg_bool bIsStartState = EGSmEd_GetFile().GetMachineProps().DefaultEntryPointNode == GetMyNode().Id; 

	HBRUSH HeaderBrush = EGSmEdResources_GetHeaderBrush( GetMyNode().Type , GetMyNode().Type == egsm_node_t::NATIVE_EVENT ? eg_string_crc(GetMyNode().Parms[0]) : CT_Clear );
	COLORREF HeaderColor = EGSmEdResources_GetHeaderColor( GetMyNode().Type , GetMyNode().Type == egsm_node_t::NATIVE_EVENT ? eg_string_crc(GetMyNode().Parms[0]) : CT_Clear );

	ISmEdApp::egFocus Focus = m_App->GetFocusedNode();

	SetBkMode( hdc , TRANSPARENT );

	auto GetStateRect = [this]() -> RECT
	{
		RECT Out = GetViewRect();
		Out.bottom = EGSmNodeConsts::GetStateHeight();
		return Out;
	};

	const RECT rcState = GetStateRect();

	SetBrush( hdc , HeaderBrush );
	if( bIsStartState ) // Start state should probably really be an outline or something.
	{
		SetPen( hdc , m_HeaderBorderPen );
		Rectangle( hdc , rcState.left+1 , rcState.top+1 , rcState.right , rcState.bottom );
	}
	else
	{
		Rectangle( hdc , rcState.left , rcState.top , rcState.right , rcState.bottom );
	}

	SetPen( hdc , m_BorderPen );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );

	RECT rcText = rcState;
	rcText.bottom = EGSmNodeConsts::GetChoiceHeight();
	rcText.top += 3;
	rcText.left += 4;
	rcText.right -= 6;
	SelectObject( hdc , reinterpret_cast<HGDIOBJ>(m_FontHeader) );
	if( GetMyNode().Type == egsm_node_t::NATIVE_EVENT )
	{
		const egsmEdNodeFormatInfo& NodeInfo = EGSmEdResources_GetNodeInfo( eg_string_crc(GetMyNode().Parms[0].String()) );
		DrawTextA( hdc , EGString_Format( "%s%s" , GetMyNode().Parms[0].String() , NodeInfo.bIsIntrinsic ? "" : " (Native)" ) , -1 , &rcText , DT_CENTER );
	}
	else
	{
		eg_string_small NodeName = EGSmFile2::NodeTypeToDisplayString( GetMyNode().Type );
		DrawTextA( hdc , NodeName , NodeName.Len() , &rcText , DT_CENTER );
	}

	rcText = rcState;
	rcText.top = EGSmNodeConsts::GetChoiceHeight();
	rcText.top += 1;
	rcText.bottom -= 2;
	rcText.left += 4;
	rcText.right -= 6;
	SetFont( hdc , m_Font );

	eg_string_big StateBody = FormatStateBody( GetMyNode() );
	DrawTextA( hdc ,StateBody , -1 , &rcText , DT_LEFT|DT_WORDBREAK );

	eg_bool bNodeFocused = Focus.StateIndex == m_StateIndex && Focus.ChoiceIndex == ISmEdApp::NO_FOCUS_CHOICE;

	if( bNodeFocused )
	{
		RECT rcFocus = rcState;
		rcFocus.right--;
		rcFocus.bottom--;
		InflateRect( &rcFocus , -2 , -2 );
		SetPen( hdc , m_FocusPen );
		DrawBorder( hdc , rcFocus );
	}
	
	SetPen( hdc , m_BorderPen );
	SetFont( hdc , m_Font );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
	SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );// m_BgBrush );
	for( eg_size_t ci = 0; ci<GetMyNode().Branches.Len(); ci++ )
	{
		RECT rcChoice = rcState;
		rcChoice.top = static_cast<LONG>(EGSmNodeConsts::GetStateHeight() + EGSmNodeConsts::GetChoiceHeight()*(ci));
		rcChoice.bottom = rcChoice.top + EGSmNodeConsts::GetChoiceHeight();

		DrawRect( hdc , rcChoice );
		rcText = rcChoice;
		rcText.top += 1;
		rcText.left += 4;
		rcText.right -= 6;
		eg_string_big TextToDraw = GetMyNode().Branches[ci].Id;
		if( GetMyNode().Branches[ci].EnRef.Len() > 0 )
		{
			TextToDraw = GetMyNode().Branches[ci].EnRef;
		}
		if( ci == 0 )
		{
			// TextToDraw.Append( " (DEFAULT)" );
		}
		DrawTextA( hdc , TextToDraw , -1 , &rcText , DT_LEFT );

		if( GetMyNode().Branches[ci].bTerminal )
		{
			RECT rcTerminalBox = rcChoice;
			rcTerminalBox.left = rcTerminalBox.right-10;
			FillRect( hdc , &rcTerminalBox , m_TerminalBrush );
		}

		eg_bool bChoiceFocused = Focus.StateIndex == m_StateIndex && Focus.ChoiceIndex == ci;

		if( bChoiceFocused )
		{
			RECT rcFocus = rcChoice;
			rcFocus.right--;
			rcFocus.bottom--;
			InflateRect( &rcFocus , -2 , -2 );
			SetPen( hdc , m_FocusPen );
			DrawBorder( hdc , rcFocus );
			SetPen( hdc , m_BorderPen );
		}
	}
}

void EGSmEdNodeWnd::OnDrawBg( HDC hdc )
{
	unused( hdc );

	// Everything should get drawn over.
}

void EGSmEdNodeWnd::OnWmMenuCmd( eg_int CmdId, eg_bool bFromAccelerator )
{
	Super::OnWmMenuCmd( CmdId , bFromAccelerator );

	switch( CmdId )
	{
	case EGC_STATEMENU_ADDCHOICE:
	{
		EGSmEd_GetFile().InsertBranchInNode( GetMyNode() , nullptr , m_NativeEventDecl.ReturnType );
		m_EditIndex = GetMyNode().Branches.Len();

		ISmEdApp::egFocus NewFocus( CT_Clear );
		NewFocus.StateIndex = m_StateIndex;
		NewFocus.ChoiceIndex = static_cast<eg_uint>(m_EditIndex-1);

		m_App->SetFocusedNode( NewFocus );

		m_ConvEditor->SetDirty();
		m_ConvEditor->OnStatesChanged();

	} break;
	case EGC_STATEMENU_DELETE:
	{
		m_App->SetFocusedNode( ISmEdApp::egFocus( CT_Clear ) );
		EGSmEd_GetFile().DeleteNode( GetMyNode().Id );
		m_ConvEditor->SetDirty();
		m_ConvEditor->OnStatesChanged();
	} break;
	case EGC_STATEMENU_DELETECHOICE:
	{
		m_App->SetFocusedNode( ISmEdApp::egFocus( CT_Clear ) );
		if( GetMyNode().Branches.Len() > 1 )
		{
			GetMyNode().Branches.DeleteByIndex( m_EditIndex-1 );
			m_ConvEditor->SetDirty();
			m_ConvEditor->OnStatesChanged();
		}
	} break;
	case EGC_STATEMENU_TOGGLETERMINAL:
	{
		GetMyNode().Branches[ m_EditIndex-1 ].bTerminal = !GetMyNode().Branches[ m_EditIndex-1 ].bTerminal;
		m_ConvEditor->SetDirty();
		m_ConvEditor->OnStatesChanged( true );
		if( m_App )
		{
			m_App->RefreshProperties();
		}
	} break;
	case EGC_STATEMENU_CHOICEUP:
	{
		eg_size_t ChoiceIndex = m_EditIndex-1;
		if( 0 < ChoiceIndex && ChoiceIndex < GetMyNode().Branches.Len() )
		{
			egsmBranchScr TempPrev = GetMyNode().Branches[ChoiceIndex-1];
			GetMyNode().Branches[ChoiceIndex-1] = GetMyNode().Branches[ChoiceIndex];
			GetMyNode().Branches[ChoiceIndex] = TempPrev;
			m_ConvEditor->SetDirty();
			m_ConvEditor->OnStatesChanged();
		}
	} break;
	case EGC_STATEMENU_CHOICEDOWN:
	{
		eg_size_t ChoiceIndex = m_EditIndex-1;
		if( 0 <= ChoiceIndex && ChoiceIndex < (GetMyNode().Branches.Len()-1) )
		{
			egsmBranchScr TempPrev = GetMyNode().Branches[ChoiceIndex+1];
			GetMyNode().Branches[ChoiceIndex+1] = GetMyNode().Branches[ChoiceIndex];
			GetMyNode().Branches[ChoiceIndex] = TempPrev;
			m_ConvEditor->SetDirty();
			m_ConvEditor->OnStatesChanged();
		}
	} break;
	case EGC_STATEMENU_SETFIRST:
	{
		EGSmEd_GetFile().SetDefaultEntryPoint( GetMyNode() );
		m_ConvEditor->SetDirty();
		m_ConvEditor->OnStatesChanged();
	} break;
	case EGCE_STATEMENU_COPY:
	{
		if( OpenClipboard( GetWnd() ) )
		{
			EGFileData ClipboardData( eg_file_data_init_t::HasOwnMemory );

			egsmNodeScr NodeCopy = GetMyNode();
			NodeCopy.Id.Clear();
			for( egsmBranchScr& Branch : NodeCopy.Branches )
			{
				Branch.ToNode.Clear();
			}
			NodeCopy.CopyToClipboard( ClipboardData );

			HGLOBAL ClipboardMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT , ClipboardData.GetSize() );
			if( ClipboardMem )
			{
				void* Buffer = GlobalLock( ClipboardMem );
				if( Buffer )
				{
					EGMem_Copy( Buffer , ClipboardData.GetData() , ClipboardData.GetSize() );
					GlobalUnlock( ClipboardMem );
				}
				HANDLE ClipboardHandle = SetClipboardData( EGWnd_GetEgsmNodeClipboardFormat() , ClipboardMem );
				if( nullptr == ClipboardHandle )
				{
					MessageBoxW( GetWnd() , L"Failed to copy to clipboard." , L"Warning" , MB_OK );
				}
			}

			CloseClipboard();
		}
	} break;
	default:
		assert( false ); // What's this?
		break;
	}
}

void EGSmEdNodeWnd::OnCaptureLost()
{
	Super::OnCaptureLost();

	if( m_ConvEditor )
	{
		m_ConvEditor->EndDrag( EGSM_INVALID_INDEX , true );
	}
}

eg_bool EGSmEdNodeWnd::DoesMouseHit( const eg_ivec2& GlobalMousePos ) const
{
	POINT GlobalHit = { GlobalMousePos.x , GlobalMousePos.y};
	RECT WindowRect;
	GetWindowRect( GetWnd() , &WindowRect );

	if( m_App->GetFocusedNode().StateIndex == m_StateIndex )
	{
		WindowRect.bottom = WindowRect.top + EGSmNodeConsts::GetStateHeight();
	}

	return PtInRect( &WindowRect , GlobalHit ) == TRUE;
}

eg_string_big EGSmEdNodeWnd::FormatStateBody( const egsmNodeScr& Node )
{
	eg_string_big Out;

	switch( Node.Type )
	{
	case egsm_node_t::ENTRY_POINT:
	{
		Out = EGString_Format( "void %s( void )" , Node.Parms[0].String() );
	} break;
	case egsm_node_t::CALL:
	{
		eg_string ModuleName = Node.Parms[0].String();
		eg_string FunctionName = Node.Parms[1].String();

		if( ModuleName.Len() == 0 )
		{
			ModuleName = "this";
		}

		if( FunctionName.Len() == 0 )
		{
			FunctionName = "DefaultEntryPoint";
		}

		Out = EGString_Format( "%s.%s()" , ModuleName.String() , FunctionName.String() );
	} break;
	case egsm_node_t::NATIVE_EVENT:
	{
		const egsmEdNodeFormatInfo& NodeInfo = EGSmEdResources_GetNodeInfo( eg_string_crc(m_NativeEventDecl.Name) );

		EGFixedArray<eg_string_big,countof(m_NativeEventDecl.ParmInfo)> FormattedParms;

		for( eg_size_t i=0; i<EG_Min( countof(m_NativeEventDecl.ParmInfo) , countof(Node.Parms)-1 ); i++ )
		{
			const egsmVarDeclParmScr& Parm = m_NativeEventDecl.ParmInfo[i];

			eg_string_big FormattedParm( CT_Clear );

			if( Parm.Type == egsm_var_t::UNK )
			{
				FormattedParm = "unused";
				continue;
			}

			if( Parm.Type == egsm_var_t::LOC_TEXT )
			{
				FormattedParm = EGString_Format( "\"%s\"" , Node.EnRef.Len() > 0 ? Node.EnRef.String() : Node.Parms[i+1].String() );
			}
			else if( Parm.Type == egsm_var_t::CRC )
			{
				if( Parm.bIsRef )
				{
					FormattedParm = EGString_Format( "%s" , Node.Parms[i+1].String() );
				}
				else
				{
					FormattedParm = EGString_Format( "crc(%s)" , Node.Parms[i+1].String() );
				}
			}
			else
			{
				FormattedParm = Node.Parms[i+1].String();
			}

			if( FormattedParm.Len() == 0 )
			{
				FormattedParm = "?";
			}

			FormattedParms.Append( FormattedParm );
		}

		FormattedParms.ExtendToAtLeast( 3 );
		Out = EGString_Format( NodeInfo.FormatString , FormattedParms[0].String() , FormattedParms[1].String() , FormattedParms[2].String() );
	} break;
	default:
	{
		Out = Node.Parms[0];
	} break;

	}

	return Out;
}
