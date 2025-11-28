// (c) 2017 Beem Media

#include "EGDefEdTimelineEditor.h"
#include "EGWndDragableLayoutEditor.h"
#include "EGEngineSerializeTypes.h"
#include "EGDefEdFile.h"
#include "EGCrcDb.h"
#include "EGWeakPtr.h"
#include "EGTimeline.h"
#include "EGWndTextNode.h"
#include "EGDefEd.h"
#include "../EGEdResLib/resource.h"
#include "EGResourceLib.h"

class EGDefEdTimelineSelector : public EGWndDragableLayoutEditor
{
	EG_DECL_SUPER( EGWndDragableLayoutEditor )

private:

	struct egItemInfo
	{
		eg_string_small DisplayName;
		eg_string_crc   TimelineId;
	};

private:

	EGArray<egItemInfo>          m_Events;
	EGDefEdTimelineEditor*const  m_OwnerEditor;

public:

	EGDefEdTimelineSelector( EGWndPanel* Parent , EGDefEdTimelineEditor* InOwner )
	: Super( Parent )
	, m_OwnerEditor( InOwner )
	{
		m_DragType = EGWndDragableLayoutEditor::eg_drag_t::None;
		Refresh();
	}

	virtual eg_size_t GetNumItems() const override final { return m_Events.Len(); }

	virtual void OnDrawItem( HDC hdc, eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const override
	{
		Super::OnDrawItem( hdc , ItemIndex , rc , bIsBeingDroppedOn , bIsHovered , bIsSelected );

		RECT TextRc = rc;
		InflateRect( &TextRc , -5 , -1 );
		DrawTextA( hdc , m_Events[ItemIndex].DisplayName , -1 , &TextRc , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX );
	}

	virtual void OnSelectedItemChanged( eg_int NewFocusedIndex ) override final
	{
		if( m_OwnerEditor )
		{
			m_OwnerEditor->SetSelectedTimeline( m_Events.IsValidIndex( NewFocusedIndex ) ? m_Events[NewFocusedIndex].TimelineId : CT_Clear );
		}
	}

	virtual void OnItemRightClicked( eg_int IndexClicked )
	{
		eg_int PopupMenuIndex = 0;
		
		if( m_Events.IsValidIndex( IndexClicked ) )
		{
			PopupMenuIndex = 0;
		}
		else
		{
			PopupMenuIndex = 1;
		}

		POINT HitPos;
		GetCursorPos( &HitPos );
		TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGDEFED_TIMLINE_EDITOR_POPUPS" ) , PopupMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr ); 
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator )
	{
		unused( bFromAccelerator );

		switch( CmdId )
		{
			case ID_TIMELINEEVENTS_NEWEVENT:
			{
				eg_int InsertIndex = GetSelectedIndex();
				SetSelectedIndex( -1 );
				if( !m_Events.IsValidIndex( InsertIndex ) )
				{
					InsertIndex = m_Events.LenAs<eg_int>();
				}
				EGDefEdFile::Get().CreateNewTimeline( InsertIndex );
				Refresh();
				SetSelectedIndex( EG_Min<eg_int>( InsertIndex , static_cast<eg_int>(GetNumItems())-1 ) );
			} break;
			case ID_TIMELINEEVENTS_DELETEEVENT:
			{
				if( m_Events.IsValidIndex( GetSelectedIndex() ) )
				{
					EGDefEdFile::Get().DeleteTimeline( GetSelectedIndex() );
					Refresh();
					SetSelectedIndex( -1 );
				}
			} break;
			case ID_TIMELINEEVENTS_MOVEUP:
			case ID_TIMELINEEVENTS_MOVEDOWN:
			{
				const eg_int NewIndex = EGDefEdFile::Get().MoveTimelineEvent( GetSelectedIndex(), CmdId == ID_TIMELINEEVENTS_MOVEUP ? -1 : 1 );
				Refresh();
				SetSelectedIndex( NewIndex );
			} break;
		}
	}

	void Refresh()
	{
		m_Events.Clear();
		EGArray<eg_d_string> AllEvents;
		EGDefEdFile::Get().QueryTimelines( AllEvents );
		for( const eg_d_string& Event : AllEvents )
		{
			egItemInfo NewItem;
			NewItem.TimelineId = EGCrcDb::StringToCrc(*Event);
			NewItem.DisplayName = *Event;
			m_Events.Append( NewItem );
		}

		UpdateScroll();
	}
};

///////////////////////////////////////////////////////////////////////////////

class EGDefEdTimelinePanel : public EGWndDragableLayoutEditor
{
	EG_DECL_SUPER( EGWndDragableLayoutEditor )

private:

	EGDefEdTimelineEditor*const  m_OwnerEditor;
	EGWeakPtr<EGTimeline>        m_Timeline = nullptr;
	eg_string_small              m_TimelineName;
	HBRUSH                       m_DotBrush = nullptr;
	HPEN                         m_DotPen = nullptr;
	HBRUSH                       m_Bg = nullptr;
	HBRUSH                       m_EventBg = nullptr;
	HBRUSH                       m_EventBgHovered = nullptr;
	HBRUSH                       m_EventBgSelected = nullptr;
	eg_real                      m_DefaultTimeSpacing = .5f;

public:

	EGDefEdTimelinePanel( EGWndPanel* Parent , EGDefEdTimelineEditor* InOwnerEditor )
	: Super( Parent )
	, m_OwnerEditor( InOwnerEditor )
	{ 
		m_DragType = EGWndDragableLayoutEditor::eg_drag_t::None;
		SetAlwaysShowScrollbar( true );
		SetScrollDir( EGWndScrollingPanel::eg_scroll_dir::Horizontal );
		SetFillSize( eg_panel_size_t::Fixed , 50 );
		SetItemSize( 50 );

		m_DotBrush = CreateSolidBrush( RGB(0,0,0) );
		m_DotPen = CreateBasicPen( 1 , RGB(0,0,0) );

		m_Bg = CreateSolidBrush( false ? EGWnd_GetColor( egw_color_t::BG_STATIC ) : RGB(55,55,58) );
		m_EventBg = CreateSolidBrush( RGB(255,255,255) );
		m_EventBgHovered = CreateSolidBrush( RGB(111,175,255) );
		m_EventBgSelected = CreateSolidBrush( RGB(0,79,174) );
	}

	virtual ~EGDefEdTimelinePanel() override
	{
		DeletePen( m_DotPen );
		DeleteBrush( m_DotBrush );
		DeleteBrush( m_Bg );
		DeleteBrush( m_EventBg );
		DeleteBrush( m_EventBgHovered );
		DeleteBrush( m_EventBgSelected );
	}

	virtual eg_size_t GetNumItems() const override 
	{
		return m_Timeline.IsValid() ? m_Timeline->GetNumKeyframes() : 0; 
	}

	eg_bool IsKeyframeIndex( eg_int Index ) const
	{
		return m_Timeline.IsValid() && EG_IsBetween<eg_int>( Index , 0 , m_Timeline->GetNumKeyframes()-1 );
	}

	virtual void OnPreDrawItems( HDC hdc )
	{
		FillRect( hdc , &GetViewRect() , m_Bg );
	}

	virtual void OnSelectedItemChanged( eg_int NewFocusedIndex ) override final
	{
		if( m_OwnerEditor )
		{
			m_OwnerEditor->SetSelectedKeyframe( NewFocusedIndex );
		}
	}

	virtual void OnDrawItem( HDC hdc, eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
	{
		unused( bIsBeingDroppedOn );

		// Super::OnDrawItem( hdc , ItemIndex , rc , bIsBeingDroppedOn , bIsHovered , bIsSelected );

		if( bIsSelected )
		{
			SetBrush( hdc , m_EventBgSelected );
		}
		else if( bIsHovered )
		{
			SetBrush( hdc , m_EventBgHovered );
		}
		else
		{
			SetBrush( hdc , m_EventBg );
		}

		SetPen( hdc , m_DotPen );
		Rectangle( hdc , rc.left , rc.top , rc.right , rc.bottom );

		SetBrush( hdc , m_DotBrush );
		SetPen( hdc , m_DotPen );
		RECT rcDot;
		SetRect( &rcDot , -4 , -4 , 4 , 4 );
		eg_int Dim = rc.right - rc.left;
		OffsetRect( &rcDot , rc.left + Dim/2 , rc.top + Dim/5 );
		if( IsKeyframeIndex( ItemIndex ) && m_Timeline->GetKeyframe( ItemIndex ).Script.Len() > 0 )
		{
			Ellipse( hdc , rcDot.left , rcDot.top , rcDot.right , rcDot.bottom );
		}

		SetBkMode( hdc , TRANSPARENT );

		RECT rcText = rc;
		InflateRect( &rcText , -2 , -Dim/4 );
		OffsetRect( &rcText , 0 , 4 );
		SetFont( hdc , EGWnd_GetFont( egw_font_t::SMALL_BOLD ) );
		// Rectangle( hdc , rcText.left , rcText.top , rcText.right , rcText.bottom );

		eg_real FrameTime = 0.f;
		if( IsKeyframeIndex( ItemIndex ) )
		{
			FrameTime = m_Timeline->GetKeyframe( ItemIndex ).Time;
		}
		eg_string_small LabelText = EGString_Format( "%gs" , FrameTime );
		SetTextColor( hdc , bIsSelected ? RGB(255,255,255) : RGB(0,0,0) );
		DrawTextA( hdc , LabelText  , -1 , &rcText , DT_SINGLELINE|DT_CENTER|DT_VCENTER );
	}

	virtual void OnItemRightClicked( eg_int IndexClicked )
	{
		if( m_Timeline.IsValid() )
		{
			eg_int PopupMenuIndex = 2;

			if( IsKeyframeIndex( IndexClicked ) )
			{
				PopupMenuIndex = 2;
			}
			else
			{
				PopupMenuIndex = 3;
			}

			POINT HitPos;
			GetCursorPos( &HitPos );
			TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGDEFED_TIMLINE_EDITOR_POPUPS" ) , PopupMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr ); 
		}
	}

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator )
	{
		unused( bFromAccelerator );

		switch( CmdId )
		{
			case ID_KEYFRAME_INSERTKEYFRAME:
			{
				if( m_Timeline.IsValid() )
				{
					EGDefEd_SetDirty();

					eg_int InsertIndex = GetSelectedIndex();
					SetSelectedIndex( -1 );

					egTimelineKeyframe NewKeyframe;

					if( !IsKeyframeIndex( InsertIndex ) )
					{
						InsertIndex = EG_Max<eg_int>( static_cast<eg_int>(GetNumItems()) , 0 );
					}
					m_Timeline->EditorInsertAtAndRespace( InsertIndex , NewKeyframe );
					m_Timeline->CompileScript();

					Refresh();
					SetSelectedIndex( EG_Min<eg_int>( InsertIndex , static_cast<eg_int>(GetNumItems())-1 ) );
				}
			} break;
			case ID_KEYFRAME_DELETEKEYFRAME:
			{
				if( IsKeyframeIndex( GetSelectedIndex() ) )
				{
					EGDefEd_SetDirty();

					EGDefEdFile::Get().PreviewTimeline( CT_Clear );
					m_Timeline->EditorDeleteKeyframeAndRespace( GetSelectedIndex() );
					m_Timeline->CompileScript();
					SetSelectedIndex( -1 );
					Refresh();
				}
			} break;
		}
	}

	void SetTimeline( EGTimeline* InTimeline )
	{
		SetSelectedIndex( -1 );
		m_Timeline = InTimeline;
		if( m_Timeline.IsValid() )
		{
			m_TimelineName = EGCrcDb::CrcToString( m_Timeline->GetId() );
			if( m_Timeline->GetNumKeyframes() > 0 )
			{
				SetSelectedIndex( 0 );
			}
		}
		else
		{
			m_TimelineName = "";
		}
		Refresh();
	}

	eg_string_crc GetCurrentTimelineId() const { return eg_string_crc(m_TimelineName); }

	void Refresh()
	{
		UpdateScroll();
		FullRedraw();
	}
};

class EGDefEdKeyframePanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	EGDefEdTimelineEditor*const m_OwnerEditor;
	EGWndTextNode               m_ScriptEditor;
	EGWndTextNode               m_EventIdHeader;
	EGWndTextNode               m_EventIdEditor;
	EGWndTextNode               m_FrameTimeHeader;
	EGWndTextNode               m_FrameTimeEditor;
	HWND                        m_PlayButton;
	HWND                        m_StopButton;
	HWND                        m_ResetButton;
	EGWeakPtr<EGTimeline>       m_Timeline;
	eg_int                      m_KeyframeIndex = -1;

public:

	EGDefEdKeyframePanel( EGWndPanel* Parent , EGDefEdTimelineEditor* InOwnerEditor )
	: Super( Parent , eg_panel_size_t::Auto , 0 )
	, m_ScriptEditor( this )
	, m_EventIdEditor( this )
	, m_EventIdHeader( this )
	, m_FrameTimeEditor( this )
	, m_FrameTimeHeader( this )
	, m_OwnerEditor( InOwnerEditor )
	{ 
		m_ScriptEditor.SetVisible( false );
		m_ScriptEditor.SetFont( EGWnd_GetFont( egw_font_t::CODE_EDIT ) );
		m_ScriptEditor.SetWordWrap( false );
		m_ScriptEditor.SetMultiline( true );
		m_ScriptEditor.SetCommitAction( EGWndTextNode::eg_commit_action::PressShiftEnter );

		const eg_int HeaderWidth = 100;

		m_EventIdHeader.SetVisible( true );
		m_EventIdHeader.SetFont( EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		m_EventIdHeader.SetStatic( true );
		m_EventIdHeader.SetMultiline( false );
		m_EventIdHeader.SetWndPos( 5 , 5 , HeaderWidth , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 6 );
		m_EventIdHeader.SetText( "Event ID" );

		m_EventIdEditor.SetVisible( false );
		m_EventIdEditor.SetFont( EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		m_EventIdEditor.SetMultiline( false );
		m_EventIdEditor.SetWndPos( 5 + HeaderWidth , 5 , 150 , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 8 );

		m_FrameTimeHeader.SetVisible( true );
		m_FrameTimeHeader.SetFont( EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		m_FrameTimeHeader.SetStatic( true );
		m_FrameTimeHeader.SetMultiline( false );
		m_FrameTimeHeader.SetWndPos( 5 , 35 , HeaderWidth , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 6 );
		m_FrameTimeHeader.SetText( "Frame Time" );

		m_ResetButton = CreateWindowW( L"BUTTON" , L"Reset" , WS_CHILD|WS_VISIBLE , 5 , 65 , 75 , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 8 , GetWnd() , NULL , NULL , NULL );
		m_StopButton = CreateWindowW( L"BUTTON" , L"Stop All" , WS_CHILD|WS_VISIBLE , 85 , 65 , 75 , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 8 , GetWnd() , NULL , NULL , NULL );
		m_PlayButton = CreateWindowW( L"BUTTON" , L"Play" , WS_CHILD|WS_VISIBLE , 165 , 65 , 75 , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 8 , GetWnd() , NULL , NULL , NULL );

		m_FrameTimeEditor.SetVisible( false );
		m_FrameTimeEditor.SetFont( EGWnd_GetFont( egw_font_t::DEFAULT_BOLD ) );
		m_FrameTimeEditor.SetMultiline( false );
		m_FrameTimeEditor.SetWndPos( 5 + HeaderWidth , 35 , 150 , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) + 8 );
	
		SetKeyframe( nullptr , -1 );
	}

	virtual ~EGDefEdKeyframePanel() override
	{
		DestroyWindow( m_StopButton );
		DestroyWindow( m_PlayButton );
		DestroyWindow( m_ResetButton );
	}

	void CommitScript()
	{
		eg_bool bWasDirty = m_ScriptEditor.IsDirty();
		m_ScriptEditor.SetDirty( false );

		if( m_Timeline.IsValid() && EG_IsBetween<eg_int>( m_KeyframeIndex , 0 , m_Timeline->GetNumKeyframes()-1 ) )
		{
			m_Timeline->GetKeyframe( m_KeyframeIndex ).Script.Set( m_ScriptEditor.GetText() );
			m_Timeline->CompileScript();
		}

		if( bWasDirty )
		{
			EGDefEd_SetDirty();
			m_OwnerEditor->NotifyScriptChanged();
		}
	}

	void CommitId()
	{
		EGDefEd_SetDirty();

		// TODO... Change Id
		eg_string_small NewName = m_EventIdEditor.GetText();
		eg_string_crc NameAsCrc = EGCrcDb::StringToCrc( NewName );
		if( m_Timeline.IsValid() && !EGDefEdFile::Get().IsTimelineEventNameTaken( NameAsCrc ) )
		{
			m_Timeline->SetId( NameAsCrc );
			m_OwnerEditor->NotifyTimelineNameChanged();
		}
		else
		{
			m_EventIdEditor.SetText( m_Timeline.IsValid() ? EGCrcDb::CrcToString( m_Timeline->GetId() ) : "" );
		}

		m_EventIdEditor.SetDirty( false );
	}

	void CommitFrameTime()
	{
		EGDefEd_SetDirty();

		eg_real NewFrameTime = m_FrameTimeEditor.GetText().ToFloat();
		if( m_Timeline.IsValid() )
		{
			m_Timeline->EditorSetSpacing( NewFrameTime );
			m_FrameTimeEditor.SetText( EGString_Format( "%g" , m_Timeline->EditorGetSpacing() ) );
			m_Timeline->CompileScript();
		}

		m_FrameTimeEditor.SetDirty( false );

		m_OwnerEditor->NotifyFrameTimeChanged();
	}

	virtual LRESULT WndProc( UINT Msg , WPARAM wParam , LPARAM lParam )
	{
		switch( Msg )
		{
			case WM_EG_COMMIT_ON_FOCUS_LOST:
			case WM_EG_COMMIT:
			{
				HWND SrcWnd = reinterpret_cast<HWND>(lParam);
				if( SrcWnd == m_ScriptEditor.GetWnd() )
				{
					CommitScript();
				}
				else if( SrcWnd == m_EventIdEditor.GetWnd() )
				{
					CommitId();
				}
				else if( SrcWnd == m_FrameTimeEditor.GetWnd() )
				{
					CommitFrameTime();
				}
			} return 0;
		}
		return Super::WndProc( Msg , wParam , lParam );
	}

	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override
	{
		unused( ControlId , NotifyId );

		if( m_PlayButton == WndControl )
		{
			if( m_Timeline.IsValid() )
			{
				m_OwnerEditor->PreviewTimeline( m_Timeline->GetId() );
			}
		}
		else if( m_ResetButton == WndControl )
		{
			m_OwnerEditor->ResetTimeline();
		}
		else if( m_StopButton == WndControl )
		{
			m_OwnerEditor->StopTimeline();
		}
	}

	virtual void OnDrawBg( HDC hdc ) override final
	{
		RECT rc = GetViewRect();
		FillRect( hdc , &rc , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
	}

	virtual void OnPaint( HDC hdc ) override final
	{
		RECT RcView = GetViewRect();

		RECT rcScriptEdit = GetScriptEditBox();

		eg_string TempText;

		if( m_Timeline.IsValid() )
		{
			if( EG_IsBetween<eg_int>( m_KeyframeIndex , 0 , m_Timeline->GetNumKeyframes() - 1 ) )
			{
				
			}
			else
			{
				TempText = "Select a key frame to edit.";
			}
		}
		else
		{
			TempText = "Select a time line to edit.";
		}

		if( TempText.Len() > 0 )
		{
			SetBrush( hdc , EGWnd_GetBrush( egw_color_t::BG_EDIT ) );
			DrawRect( hdc , rcScriptEdit );

			SetFont( hdc , EGWnd_GetFont( egw_font_t::DEFAULT ) );
			SetBkMode( hdc , TRANSPARENT );
			SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );
			DrawTextA( hdc , TempText , -1 , &rcScriptEdit , DT_SINGLELINE|DT_TOP|DT_CENTER|DT_VCENTER|DT_NOPREFIX );
		}
	}

	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override final
	{
		Super::OnWmSize( NewClientSize );

		RECT rcScriptEdit = GetScriptEditBox();

		m_ScriptEditor.SetWndRect( rcScriptEdit );
	}

	void SetKeyframe( EGTimeline* Timeline , eg_int KeyframeIndex )
	{
		m_Timeline = Timeline;
		m_KeyframeIndex = KeyframeIndex;

		if( m_Timeline.IsValid() && EG_IsBetween<eg_int>( m_KeyframeIndex , 0 , m_Timeline->GetNumKeyframes() - 1 ) )
		{
			const egTimelineKeyframe& Keyframe = m_Timeline->GetKeyframe( KeyframeIndex );
			m_ScriptEditor.SetVisible( true );
			m_ScriptEditor.SetText( *Keyframe.Script );
		}
		else
		{
			m_ScriptEditor.SetVisible( false );
		}

		m_EventIdEditor.SetVisible( m_Timeline.IsValid() );
		m_EventIdEditor.SetText( m_Timeline.IsValid() ? EGCrcDb::CrcToString( m_Timeline->GetId() ) : "" );

		m_FrameTimeEditor.SetVisible( m_Timeline.IsValid() );
		m_FrameTimeEditor.SetText( m_Timeline.IsValid() ? EGString_Format( "%g" , m_Timeline->EditorGetSpacing() ) : "" );

		ShowWindow( m_PlayButton , Timeline ? SW_SHOW : SW_HIDE );

		FullRedraw();
	}

	void Refresh()
	{
		FullRedraw();
	}

	RECT GetScriptEditBox() const
	{
		RECT ViewRect = GetViewRect();
		RECT rcScriptEdit;
		SetRect( &rcScriptEdit , 300 , 10 , ViewRect.right - 10 , ViewRect.bottom - 10 );
		return rcScriptEdit;
	}
};

///////////////////////////////////////////////////////////////////////////////

EGDefEdTimelineEditor::EGDefEdTimelineEditor( EGWndPanel* Parent )
: Super( Parent , eg_panel_fill::Vertical , eg_panel_size_t::Auto , 0 )
{
	EGWndPanelHeader* MainHeader = AddChild<EGWndPanelHeader>( "Timeline Editor" );
	// MainHeader->SetColors( EGWnd_GetColor( egw_color_t::FG_BTN ) , egw_color_t::BG_BTN );
	EGWndPanel* MainContainer = AddContainer( eg_panel_fill::Horizontal );
	EGWndPanel* LeftPanel = MainContainer->AddContainer( eg_panel_fill::Vertical );
	LeftPanel->SetFillSize( eg_panel_size_t::Fixed , EG_To<eg_int>(GetPane3SideWidth()*.75f) );
	LeftPanel->AddChild<EGWndPanelHeader>( "Events" );
	EGWndPanel* RightPanel = MainContainer->AddContainer( eg_panel_fill::Vertical );
	m_TimelineSelector = LeftPanel->AddChild<EGDefEdTimelineSelector>( this );
	m_KeyframePanel = RightPanel->AddChild<EGDefEdKeyframePanel>( this );
	m_TimelinePanel = RightPanel->AddChild<EGDefEdTimelinePanel>( this );
	
	m_TimelineSelector->Refresh();
}

void EGDefEdTimelineEditor::PreviewTimeline( eg_string_crc TimelineId )
{
	EGDefEdFile::Get().PreviewTimeline( TimelineId );
}

void EGDefEdTimelineEditor::ResetTimeline()
{
	EGDefEdFile::Get().ResetTimeline();
}

void EGDefEdTimelineEditor::StopTimeline()
{
	EGDefEdFile::Get().StopTimeline();
}

void EGDefEdTimelineEditor::SetSelectedTimeline( eg_string_crc TimelineId )
{
	PreviewTimeline( TimelineId );
	m_Timeline = EGDefEdFile::Get().GetTimelineById( TimelineId );
	SetSelectedKeyframe( -1 );
	if( m_TimelinePanel )
	{
		m_TimelinePanel->SetTimeline( m_Timeline.GetObject() ); // Will set the timeline to null for CT_Clear
	}
}

void EGDefEdTimelineEditor::SetSelectedKeyframe( eg_int KeyframeIndex )
{
	if( m_KeyframePanel )
	{
		m_KeyframePanel->SetKeyframe( m_Timeline.GetObject() , KeyframeIndex );
	}
}

void EGDefEdTimelineEditor::NotifyTimelineNameChanged()
{
	if( m_TimelineSelector )
	{
		m_TimelineSelector->Refresh();
	}
}

void EGDefEdTimelineEditor::NotifyFrameTimeChanged()
{
	m_TimelinePanel->Refresh();
}

void EGDefEdTimelineEditor::NotifyScriptChanged()
{
	m_TimelinePanel->Refresh();
	if( m_TimelinePanel )
	{
		PreviewTimeline( m_TimelinePanel->GetCurrentTimelineId() );
	}
}

void EGDefEdTimelineEditor::Refresh()
{
	m_TimelineSelector->Refresh();
	m_TimelinePanel->Refresh();
}
