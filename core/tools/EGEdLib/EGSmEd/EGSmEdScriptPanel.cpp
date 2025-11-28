// (c) 2017 Beem Media

#include "EGSmEdScriptPanel.h"
#include "EGSmEd.h"
#include "EGSmEdResources.h"
#include "EGFileData.h"

EGSmEdScriptPanel::EGSmEdScriptPanel( EGWndPanel* Parent )
: EGWndPanel( Parent , eg_panel_size_t::Auto , 0 )
, m_IsDraggingChoice(false)
{
	m_ConnPen = CreatePen( PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND, 2, CONN_COLOR );
	m_ConnErasePen = CreatePen( PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND, 2, BG_COLOR );

	m_BgBrush = CreateSolidBrush( BG_COLOR );

	EnableDoubleBuffering( true );
}

EGSmEdScriptPanel::~EGSmEdScriptPanel()
{
	EGSmEd_GetFile().Clear();
	ReinitializeStates();

	DeleteObject( m_ConnErasePen );
	DeleteObject( m_ConnPen );
	DeleteObject( m_BgBrush );
}

LRESULT EGSmEdScriptPanel::WndProc( UINT Msg , WPARAM wParam , LPARAM lParam )
{
	switch( Msg )
	{
	case WM_LBUTTONDOWN:
	{
		EGWnd_ClearFocus();
		// m_App->SetFocusedNode( ISmEdApp::egFocus( CT_Clear ) );
		POINT HitPos = EGSmEdResources_MakePoint( lParam );
		assert( !m_IsDraggingChoice );
		m_LastViewDragPos = HitPos;
		BeginCapture( eg_ivec2(HitPos.x,HitPos.y) , EGSmEdResources_GetCursor( egsm_cursor_t::GRAB ) );
	} return 0;
	case WM_LBUTTONUP:
	{
		POINT HitPos = EGSmEdResources_MakePoint( lParam );

		if( IsCapturing() )
		{
			EndCapture( eg_ivec2(HitPos.x,HitPos.y) );
		}
		EndDrag( EGSM_INVALID_INDEX , false );
	} return 0;
	case WM_RBUTTONDOWN:
	{
		m_App->SetFocusedNode( ISmEdApp::egFocus( CT_Clear ) );
	} return 0;
	case WM_RBUTTONUP:
	{
		POINT HitPos = EGSmEdResources_MakePoint( lParam );
		m_ContextCmdPos = eg_vec2( static_cast<eg_real>(HitPos.x/EGWnd_GetAppScaling()) , static_cast<eg_real>(HitPos.y/EGWnd_GetAppScaling()) );
		GetCursorPos( &HitPos );
		TrackPopupMenu( EGSmEdResources_GetMenu( egsm_menu_t::CONTEXT_NEW_NODE ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y  ,0 , m_hwnd , nullptr );
		//TrackPopupMenu( GetSubMenu( LoadMenu( EGEditorResLib_GetLibrary() , L"EGCE_CONTEXTMENU" ) , 0 ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y  ,0 , m_hwnd , nullptr ); 
	} return 0;
	}

	return Super::WndProc( Msg , wParam , lParam );
}

void EGSmEdScriptPanel::OnWmMouseMove( const eg_ivec2& MousePos )
{
	Super::OnWmMouseMove( MousePos );

	POINT HitPos = { MousePos.x , MousePos.y };

	if( IsCapturing() )
	{
		assert( !m_IsDraggingChoice );

		POINT MoveAmount = { HitPos.x - m_LastViewDragPos.x , HitPos.y - m_LastViewDragPos.y };
		m_LastViewDragPos = HitPos;

		EGSmEd_GetFile().GetMachineProps().ViewOffset.x += MoveAmount.x;
		EGSmEd_GetFile().GetMachineProps().ViewOffset.y += MoveAmount.y;

		UpdateChildrenView();

		ScrollContent( MoveAmount.x , MoveAmount.y );
	}
}

void EGSmEdScriptPanel::OnPaint( HDC hdc )
{
	for( eg_size_t i=0; i<m_Connectors.Len(); i++ )
	{
		egConnector& Conn = m_Connectors[i];

		if( m_IsDraggingChoice && Conn.FromStateIndex == m_DragConnector.FromStateIndex && Conn.FromChoiceIndex == m_DragConnector.FromChoiceIndex )
		{
			continue;
		}

		DrawConnector( hdc , Conn , false );
	}

	if( m_IsDraggingChoice )
	{
		DrawConnector( hdc , m_DragConnector , true );
	}
}

void EGSmEdScriptPanel::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , m_BgBrush );
}

void EGSmEdScriptPanel::OnWmMenuCmd( eg_int CmdId, eg_bool bFromAccelerator )
{
	unused( bFromAccelerator );

	if( EGSM_GENERIC_NATIVE_EVENT_ID_OFFSET == CmdId )
	{
		egsmVarDeclScr BlankNe;
		BlankNe.DeclType = egsm_var_decl_t::UNK;
		BlankNe.ReturnType = egsm_var_t::UNK;
		CreateNativeEvent( BlankNe , m_ContextCmdPos );
		return;
	}

	if( EGSM_NATIVE_EVENT_ID_OFFSET <= CmdId && CmdId < EGSM_MAX_NATIVE_EVENT )
	{
		UINT NativeEventOffset = CmdId - EGSM_NATIVE_EVENT_ID_OFFSET;
		EGArray<egsmVarDeclScr> Functions;
		EGSmEdVarMgr_GetFunctions( Functions );
		if( Functions.IsValidIndex( NativeEventOffset ) )
		{
			CreateNativeEvent( Functions[NativeEventOffset] , m_ContextCmdPos );
		}
		return;
	}

	if( EGCE_SCRIPT_PASTE == CmdId )
	{
		if( OpenClipboard( GetWnd() ) )
		{
			HANDLE ClipboardHandle = GetClipboardData( EGWnd_GetEgsmNodeClipboardFormat() );
			if( ClipboardHandle )
			{
				const eg_byte* Buffer = static_cast<const eg_byte*>(GlobalLock( ClipboardHandle ));
				if( Buffer )
				{
					EGFileData ClipboardData( Buffer , 16*1024 );

					egsmNodeScr PasteNode;
					PasteNode.PasteFromClipboard( ClipboardData );

					egsmNodeScr& NewState = EGSmEd_GetFile().CreateNode( PasteNode.Type );
					NewState.EditorPos = m_ContextCmdPos;
					NewState.EditorPos.x -= EGSmEd_GetFile().GetMachineProps().ViewOffset.x;
					NewState.EditorPos.y -= EGSmEd_GetFile().GetMachineProps().ViewOffset.y;

					for( eg_int i=0; i<EGSM_MAX_PARMS; i++ )
					{
						NewState.Parms[i] = PasteNode.Parms[i];
					}
					NewState.Branches = std::move( PasteNode.Branches );
					NewState.EnRef = PasteNode.EnRef;
					NewState.EnParmIndex = PasteNode.EnParmIndex;

					SetDirty();
					OnStatesChanged( false );
				}

				GlobalUnlock( ClipboardHandle );
			}
			else
			{
				MessageBoxW( GetWnd() , L"Nothing to paste." , L"Warning" , MB_OK );
			}

			CloseClipboard();
		}
		return;
	}

	switch( CmdId )
	{
#define DECL_NODE( _name_ , _color_ ) case EGSM_NEW_NODE_ID_OFFSET+static_cast<UINT>( egsm_node_t::_name_):{ CreateState( egsm_node_t::_name_ , m_ContextCmdPos ); } break;
#include "EGSmNodes.items"
	default:
		assert( false ); // What was this?
		break;
	}
}

void EGSmEdScriptPanel::ReinitializeStates()
{
	for( eg_size_t i = 0; i < m_StateWnds.Len(); i++ )
	{
		m_StateWnds[i].Deinit();
	}
	m_StateWnds.Clear();

	for( eg_size_t i = 0; i < EGSmEd_GetFile().GetNodeCount(); i++ )
	{
		if( EGSmEd_GetFile().GetNode(i).bUsed )
		{
			m_StateWnds.ExtendToAtLeast( i+1 );
			m_StateWnds[i].Init( this, m_App, i );
		}
	}

	m_Connectors.Clear();

	for( eg_size_t si = 0; si < EGSmEd_GetFile().GetNodeCount(); si++ )
	{
		egsmNodeScr& State = EGSmEd_GetFile().GetNode( si );
		if( State.bUsed )
		{
			for( eg_size_t ci = 0; ci < State.Branches.Len(); ci++ )
			{
				eg_size_t EditorLink = m_Connectors.Len();
				m_Connectors.ExtendToAtLeast( EditorLink+1 );
				m_Connectors[EditorLink].bActive = true;
				m_Connectors[EditorLink].FromStateIndex = si;
				m_Connectors[EditorLink].FromChoiceIndex = ci;
				m_Connectors[EditorLink].bHasBeenDrawn = false;
				m_Connectors[EditorLink].ToStateIndex = EGSmEd_GetFile().GetNodeIndexById( State.Branches[ci].ToNode );

				if( m_Connectors[EditorLink].ToStateIndex == EGSM_INVALID_INDEX )
				{
					m_Connectors[EditorLink].bActive = false;
				}
			}
		}
	}

	FullRedraw();
}

void EGSmEdScriptPanel::CreateState( egsm_node_t Type, const eg_vec2& Pos )
{
	egsmNodeScr& NewState = EGSmEd_GetFile().CreateNode( Type );
	NewState.EditorPos = Pos;

	NewState.EditorPos.x -= GetViewOffset().x;
	NewState.EditorPos.y -= GetViewOffset().y;

	ISmEdApp::egFocus NewFocus( CT_Clear );
	NewFocus.StateIndex = EGSmEd_GetFile().GetNodeIndexById( NewState.Id );

	SetDirty();
	OnStatesChanged( false );

	m_App->SetFocusedNode( NewFocus );
}

void EGSmEdScriptPanel::CreateNativeEvent( const egsmVarDeclScr& FnDecl, const eg_vec2& Pos )
{
	egsmNodeScr& NewState = EGSmEd_GetFile().CreateNode( egsm_node_t::NATIVE_EVENT );
	NewState.EditorPos = Pos;
	NewState.EditorPos.x -= EGSmEd_GetFile().GetMachineProps().ViewOffset.x;
	NewState.EditorPos.y -= EGSmEd_GetFile().GetMachineProps().ViewOffset.y;

	NewState.Parms[0] = FnDecl.Name;


	// If any of the parms are LOC_TEXT generate a key for them.
	for( eg_size_t i = 0; i < countof( FnDecl.ParmInfo ); i++ )
	{
		if( FnDecl.ParmInfo[i].DefaultValue.Len() > 0 )
		{
			NewState.Parms[i+1] = FnDecl.ParmInfo[i].DefaultValue;
		}
		else
		{
			if( FnDecl.ParmInfo[i].Type == egsm_var_t::LOC_TEXT && ( i + 1 ) < countof( NewState.Parms ) )
			{
				NewState.Parms[i + 1] = EGSmEd_GetFile().GetNextLocTextId();
			}
			if( FnDecl.ParmInfo[i].Type == egsm_var_t::BOOL && !FnDecl.ParmInfo[i].bIsRef )
			{
				NewState.Parms[i+1] = "true";
			}
		}
	}

	// Insert appropriate branches based on the return type. (LOC_TEXT will have keys generated).
	switch( FnDecl.ReturnType )
	{
	case egsm_var_t::UNK:
	case egsm_var_t::RETURN_VOID:
		EGSmEd_GetFile().InsertBranchInNode( NewState , "NEXT" );
		break;
	case egsm_var_t::BOOL:
		EGSmEd_GetFile().InsertBranchInNode( NewState , "TRUE" );
		EGSmEd_GetFile().InsertBranchInNode( NewState , "FALSE" );
		break;
	case egsm_var_t::TERMINAL:
		// No branches...
		break;
	default:
		EGSmEd_GetFile().InsertBranchInNode( NewState , nullptr , FnDecl.ReturnType );
		break;
	}

	ISmEdApp::egFocus NewFocus( CT_Clear );
	NewFocus.StateIndex = EGSmEd_GetFile().GetNodeIndexById( NewState.Id );

	SetDirty();
	OnStatesChanged( false );

	m_App->SetFocusedNode( NewFocus );
}

void EGSmEdScriptPanel::UpdateChildrenView()
{
	for( eg_size_t i = 0; i < m_StateWnds.Len(); i++ )
	{
		if( m_StateWnds[i].Wnd )
		{
			m_StateWnds[i].Wnd->UpdateView();
		}
	}
}

POINT EGSmEdScriptPanel::ToPoint( const eg_vec2& v )
{
	POINT Out = { static_cast<LONG>( v.x ) , static_cast<LONG>( v.y ) };
	return Out;
}

POINT EGSmEdScriptPanel::GetViewOffset() const
{
	POINT Out = { static_cast<LONG>(EGSmEd_GetFile().GetMachineProps().ViewOffset.x) , static_cast<LONG>(EGSmEd_GetFile().GetMachineProps().ViewOffset.y) };
	return Out;
}

void EGSmEdScriptPanel::GetConnectorPos( const egConnector& Conn, POINT* StartOut, POINT* EndOut ) const
{
	const egsmNodeScr& FromState = EGSmEd_GetFile().GetNode( Conn.FromStateIndex );
	const egsmNodeScr& ToState = EGSmEd_GetFile().GetNode( Conn.ToStateIndex );

	StartOut->x = EGWnd_GetAppScaledSize(GetViewOffset().x) + EGWnd_GetAppScaledSize(static_cast<LONG>( FromState.EditorPos.x )) + EGSmNodeConsts::GetStateWidth();
	StartOut->y = EGWnd_GetAppScaledSize(GetViewOffset().y) + EGWnd_GetAppScaledSize(static_cast<LONG>( FromState.EditorPos.y )) + EGSmNodeConsts::GetStateHeight() + EGSmNodeConsts::GetChoiceHeight()*( static_cast<LONG>(Conn.FromChoiceIndex) ) + EGSmNodeConsts::GetChoiceHeight() / 2;

	EndOut->x = EGWnd_GetAppScaledSize(GetViewOffset().x) + EGWnd_GetAppScaledSize(static_cast<LONG>( ToState.EditorPos.x ));
	EndOut->y = EGWnd_GetAppScaledSize(GetViewOffset().y) + EGWnd_GetAppScaledSize(static_cast<LONG>( ToState.EditorPos.y )) + EGSmNodeConsts::GetStateHeight() / 2;
}

void EGSmEdScriptPanel::DrawConnector( HDC hdc, egConnector& Conn, eg_bool bDrawToDrag )
{
	if( !Conn.bActive || Conn.ToStateIndex == EGSM_INVALID_INDEX )
	{
		return;
	}

	if( !Conn.bHasBeenDrawn )
	{
		Conn.bHasBeenDrawn = true;
		GetConnectorPos( Conn, &Conn.LastDrawnStart, &Conn.LastDrawnEnd );
	}

	auto DrawCurve = [hdc]( HPEN Pen, const POINT& Start, const POINT& End ) -> void
	{
		auto MapTo = []( LONG t , eg_real FromMin , eg_real FromMax , eg_real ToMin , eg_real ToMax ) -> LONG
		{
			return static_cast<LONG>( EG_Clamp<eg_real>( EGMath_GetMappedRangeValue( static_cast<eg_real>( t ), eg_vec2( FromMin , FromMax ), eg_vec2( ToMin , ToMax ) ), EG_Min( ToMin , ToMax ), EG_Max( ToMin , ToMax ) ) );
		};

		LONG CurveLeftAmount = 0;
		LONG CurveUpAmount = 0;

		if( Start.x < End.x )
		{
			CurveLeftAmount = MapTo( (End.x - Start.x)/2 , 0 , 50 , 0.f , 50.f );
			CurveUpAmount = 0;

			const POINT CurvePoints[] =
			{
				Start,
				{ Start.x + CurveLeftAmount/2 , Start.y },
				{ Start.x + CurveLeftAmount , Start.y + CurveUpAmount },
				{ (Start.x + End.x)/2 , (Start.y+End.y)/2 },
				{ End.x - CurveLeftAmount   , End.y - CurveUpAmount },
				{ End.x - CurveLeftAmount/2   , End.y },
				End,
			};

			SelectObject( hdc, reinterpret_cast<HGDIOBJ>( Pen ) );
			BOOL Succeeded = PolyBezier( hdc, CurvePoints, countof( CurvePoints ) );
		}
		else
		{			
			CurveLeftAmount = MapTo( (End.x - Start.x) , -50 , 0 , 75 , 0.f );
			CurveUpAmount = MapTo( End.y - Start.y , -100.f, 0.f , -50.f, 50.f );
			LONG CurveMuliplier = MapTo( End.x - Start.x , -200.f , 0.f , 100.f , 0.f );

			CurveLeftAmount = (CurveLeftAmount*CurveMuliplier)/100;
			CurveUpAmount = (CurveUpAmount*CurveMuliplier)/100;
		}

		const POINT CurvePoints[] =
		{
			Start,
			{ Start.x + CurveLeftAmount/2 , Start.y },
			{ Start.x + CurveLeftAmount , Start.y + CurveUpAmount },
			{ (Start.x + End.x)/2 , (Start.y+End.y)/2 },
			{ End.x - CurveLeftAmount   , End.y - CurveUpAmount },
			{ End.x - CurveLeftAmount/2   , End.y },
			End,
		};

		SelectObject( hdc, reinterpret_cast<HGDIOBJ>( Pen ) );
		BOOL Succeeded = PolyBezier( hdc, CurvePoints, countof( CurvePoints ) );

		// MoveToEx( hdc , Start.x , Start.y , nullptr );
		// LineTo( hdc, End.x , End.y );
	};

	POINT Start, End;
	GetConnectorPos( Conn, &Start, &End );

	if( bDrawToDrag )
	{
		End = m_DragConnectorDest;
	}

	if(
		Conn.LastDrawnEnd.x != End.x
		|| Conn.LastDrawnEnd.y != End.y
		|| Conn.LastDrawnStart.x != Start.x
		|| Conn.LastDrawnStart.y != Start.y
		)
	{
		DrawCurve( m_ConnErasePen, Conn.LastDrawnStart, Conn.LastDrawnEnd ); // Erase the previous connector.
	}
	DrawCurve( m_ConnPen, Start, End ); // Draw the connector in the current position..

	Conn.LastDrawnStart = Start;
	Conn.LastDrawnEnd = End;
}

void EGSmEdScriptPanel::OnStatesChanged( eg_bool bOnlyRepaint )
{
	if( bOnlyRepaint )
	{
		for( egConvState& State : m_StateWnds )
		{
			if( State.Wnd )
			{
				State.Wnd->FullRedraw();
			}
		}
	}
	else
	{
		ReinitializeStates();
	}
}

void EGSmEdScriptPanel::OnAppLostFocus()
{
	if( m_IsDraggingChoice )
	{
		EndDrag( EGSM_INVALID_INDEX , true );
	}
}

void EGSmEdScriptPanel::OnFocusChanged( const ISmEdApp::egFocus& NewFocus, const ISmEdApp::egFocus& OldFocus )
{
	if( m_StateWnds.IsValidIndex(NewFocus.StateIndex) )
	{
		m_StateWnds[NewFocus.StateIndex].Wnd->FullRedraw();
	}

	if( m_StateWnds.IsValidIndex(OldFocus.StateIndex) )
	{
		m_StateWnds[OldFocus.StateIndex].Wnd->FullRedraw();
	}
}

void EGSmEdScriptPanel::StartDrag( eg_size_t FromState, eg_size_t FromChoice )
{
	assert( !IsCapturing() );

	m_IsDraggingChoice = true;
	m_DraggingFromStateIndex = FromState;
	m_DraggingFromChoiceIndex = FromChoice;

	m_DragConnector.bActive = true;
	m_DragConnector.FromStateIndex = FromState;
	m_DragConnector.ToStateIndex = FromState;
	m_DragConnector.bHasBeenDrawn = false;
	m_DragConnector.FromChoiceIndex = FromChoice;
}

void EGSmEdScriptPanel::EndDrag( eg_size_t HitStateIndex, eg_bool bDontClear )
{
	if( m_IsDraggingChoice )
	{
		if( 0 <= m_DraggingFromStateIndex && m_DraggingFromStateIndex < EGSmEd_GetFile().GetNodeCount() )
		{
			egsmNodeScr& FromState = EGSmEd_GetFile().GetNode( m_DraggingFromStateIndex );

			eg_string OldTo = FromState.Branches[m_DraggingFromChoiceIndex].ToNode;

			if( 0 <= HitStateIndex && HitStateIndex < EGSmEd_GetFile().GetNodeCount() )
			{
				egsmNodeScr& ToState = EGSmEd_GetFile().GetNode( HitStateIndex );

				if( FromState.Branches.IsValidIndex( m_DraggingFromChoiceIndex ) )
				{
					FromState.Branches[m_DraggingFromChoiceIndex].ToNode = ToState.Id;
				}
			}
			else
			{
				if( !bDontClear )
				{
					FromState.Branches[m_DraggingFromChoiceIndex].ToNode = "";
				}
			}

			if( OldTo != FromState.Branches[m_DraggingFromChoiceIndex].ToNode )
			{
				SetDirty();
			}
		}
		m_IsDraggingChoice = false;
		m_DragConnector.bActive = false;
		OnStatesChanged( false );
		FullRedraw();
	}

	m_IsDraggingChoice = false;
	m_DragConnector.bActive = false;
}

void EGSmEdScriptPanel::SetDragHitNode( eg_size_t StateIndex )
{
	if( m_IsDraggingChoice )
	{
		const egsmNodeScr& State = EGSmEd_GetFile().GetNode( StateIndex );
		m_DragConnectorDest.x = EGWnd_GetAppScaledSize(static_cast<eg_int>(GetViewOffset().x)) + EGWnd_GetAppScaledSize(static_cast<LONG>( State.EditorPos.x) );
		m_DragConnectorDest.y = EGWnd_GetAppScaledSize(static_cast<eg_int>(GetViewOffset().y)) + EGWnd_GetAppScaledSize(static_cast<LONG>( State.EditorPos.y) + EGSmNodeConsts::GetStateHeight() / 2 );
		FullRedraw();
	}
}

eg_size_t EGSmEdScriptPanel::SetDragHitMousePos( const eg_ivec2& GlobalMousePose )
{
	eg_size_t Out = EGSM_INVALID_INDEX;

	assert( m_IsDraggingChoice );
	if( m_IsDraggingChoice )
	{
		POINT ClientPoint = { GlobalMousePose.x , GlobalMousePose.y };
		ScreenToClient( GetWnd() , &ClientPoint );

		m_DragConnectorDest = ClientPoint;
		
		EGSmEdNodeWnd* StateHitWnd = nullptr;
		for( const egConvState& State : m_StateWnds )
		{
			if( State.Wnd && State.Wnd->DoesMouseHit( GlobalMousePose ) )
			{
				StateHitWnd = State.Wnd;
			}
		}

		if( StateHitWnd )
		{
			Out = StateHitWnd->GetStateIndex();
			SetDragHitNode( StateHitWnd->GetStateIndex() );
		}

		FullRedraw();
	}

	return Out;
}

POINT EGSmEdScriptPanel::GetView() const
{
	return GetViewOffset();
}

void EGSmEdScriptPanel::SetDirty()
{
	if( m_App )
	{
		m_App->SetDirty();
	}
}
