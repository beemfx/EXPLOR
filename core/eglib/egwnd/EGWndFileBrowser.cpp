// (c) 2019 Beem Media

#include "EGWndFileBrowser.h"
#include "EGPath2.h"
#include "EGWndDraw.h"
#include "EGWndTabbedPanel.h"

EGWndFileBrowser::EGWndFileBrowser( EGWndPanel* InParent , EGWndTabbedPanel* InTabOwner )
: Super( InParent , eg_panel_size_t::Auto , 0 )
, m_TabOwner( InTabOwner )
{
	EGWndPanel* ContentContainer = AddContainer( eg_panel_fill::Vertical  );
	if( ContentContainer )
	{
		m_Header = ContentContainer->AddChild<EGWndPanelHeader>( "Files..." );

		EGWndPanel* RootContainer = ContentContainer->AddContainer( eg_panel_fill::Horizontal );
		if( RootContainer )
		{
			m_FolderView = RootContainer->AddChild<EGWndFileBrowserFolderView>( this );
			m_FileViewShell = RootContainer->AddChild<EGWndFileBrowserFileViewShell>( this );
		}
	}
}

void EGWndFileBrowser::SetRoot( eg_cpstr16 Root )
{
	m_Root = Root;
	m_Root += L"/";
	m_Root = EGPath2_CleanPath( *m_Root , L'/' );
	m_CurrentDirectory = L"/";

	// m_FolderTree.DebugPrint(" ->");

	if( m_FolderView )
	{
		m_FolderView->InitView();
	}

	if( m_FileViewShell )
	{
		m_FileViewShell->InitView();
	}

	UpdateHeaderText();
}

void EGWndFileBrowser::SetDirectory( eg_cpstr16 NewDirectory )
{
	if( m_FileViewShell )
	{
		m_FileViewShell->SetDirectory( NewDirectory );
	}
}

void EGWndFileBrowser::UpdateViewedDirectory( eg_cpstr16 NewDirectory )
{
	m_CurrentDirectory = NewDirectory;

	UpdateHeaderText();
}

void EGWndFileBrowser::GetDirectoryContents( eg_cpstr16 Directory , EGArray<egOsFileInfo>& Out ) const
{
	const eg_d_string16 FullPath = EGPath2_CleanPath( *(m_Root + Directory) , L'/' );
	egOsFileInfo DirContents;
	EGOsFile_GetDirectoryContents( *FullPath , DirContents );
	for( const egOsFileInfo& File : DirContents.Children )
	{
		if( !File.Attributes.bHidden )
		{
			Out.Append( File );
		}
	}
	Out.Sort( []( const egOsFileInfo& Left , const egOsFileInfo& Right ) -> eg_bool
	{
		if( Left.Attributes.bDirectory && !Right.Attributes.bDirectory )
		{
			return true;
		}
		if( !Left.Attributes.bDirectory && Right.Attributes.bDirectory )
		{
			return false;
		}

		return EGString_CompareI( *Left.Name , *Right.Name ) < 0;
	} );
}

void EGWndFileBrowser::UpdateHeaderText()
{
	//const eg_cpstr16 FormatString = L"{0} ({1})";
	const eg_cpstr16 FormatString = L"{1}";
	const eg_d_string16 Header = EGSFormat16( FormatString , *m_Root , *m_CurrentDirectory );

	if( m_Header )
	{
		m_Header->SetHeaderText( *eg_d_string8( *Header ) );
	}

	if( m_TabOwner )
	{
		m_TabOwner->SetTabTextFor( this , *Header );
	}
}

//////////////////////////////////////////////////////////////////////////

EGWndFileBrowserFolderView::EGWndFileBrowserFolderView( EGWndPanel* InParent , EGWndFileBrowser* InOwnerBrowser )
: Super( InParent, EGWnd_GetFontSize( egw_font_t::DEFAULT ) )
, m_OwnerBrowser( InOwnerBrowser )
{
	SetFillSize( eg_panel_size_t::Percent, 20 );
}

void EGWndFileBrowserFolderView::InitView()
{

}

eg_size_t EGWndFileBrowserFolderView::GetNumItems() const
{
	return 0;
}

void EGWndFileBrowserFolderView::OnDrawBg( EGWndDc& Dc )
{
	Dc.DcDrawFilledRect( GetViewArea() , EGWnd_GetDcBrush( egw_color_t::BG_STATIC ) );
}

void EGWndFileBrowserFolderView::OnPaint( EGWndDc& Dc )
{
	eg_recti rc = GetViewArea();
	Dc.DcSetBkMode( eg_wnd_dc_bk_mode::Transparent );
	Dc.DcSetFont( EGWnd_GetDcFont( egw_font_t::DEFAULT ) );
	Dc.DcSetTextColor( EGWnd_GetDcColor( egw_color_t::FG_STATIC ) );
	Dc.DcDrawText( L"TODO: Folder View", rc , EGWND_DC_F_CENTER|EGWND_DC_F_SINGLELINE|EGWND_DC_F_VCENTER );
}

//////////////////////////////////////////////////////////////////////////

EGWndFileBrowserFileViewShell::EGWndFileBrowserFileViewShell( EGWndPanel* InParent, EGWndFileBrowser* InOwnerBrowser )
: Super( InParent , eg_panel_fill::Vertical )
, m_OwnerBrowser( InOwnerBrowser )
{
	m_Header = AddChild<EGWndFileBrowserFileViewHeader>();
	m_FolderView = AddChild<EGWndFileBrowserFileViewBody>( InOwnerBrowser , m_Header );
}


void EGWndFileBrowserFileViewShell::InitView()
{
	if( m_FolderView )
	{
		m_FolderView->InitView();
	}
}

void EGWndFileBrowserFileViewShell::SetDirectory( eg_cpstr16 NewDirectory )
{
	if( m_FolderView )
	{
		m_FolderView->SetDirectory( NewDirectory );
	}
}

//////////////////////////////////////////////////////////////////////////

void EGWndFileBrowserFileViewHeader::OnDrawBg( EGWndDc& Dc )
{
	Dc.DcDrawFilledRect( GetViewArea() , egw_color_t::BG_STATIC );
}

void EGWndFileBrowserFileViewHeader::OnPaint( EGWndDc& Dc )
{
	eg_recti rc = GetViewArea();
	Dc.DcSetBrush( egw_color_t::BG_EDIT );
	Dc.DcSetFont( egw_font_t::DEFAULT_BOLD );
	Dc.DcSetTextColor( egw_color_t::FG_EDIT );
	Dc.DcSetBkMode( eg_wnd_dc_bk_mode::Transparent );

	auto DrawColumn = [&Dc,&rc](eg_cpstr16 Text , eg_int ColumnWidth ) -> void
	{
		rc.right = rc.left + ColumnWidth;

		Dc.DcDrawRectangle( rc );

		eg_recti FontRect = rc;
		FontRect.InflateThis( -2 , 0 );
		Dc.DcDrawText_LeftAlignedSingleLine( Text , FontRect );

		rc.left += ColumnWidth;
		rc.right = rc.left;
	};

	DrawColumn( L"Name" , GetFilenameWidth() );
	DrawColumn( L"Type" , GetFileTypeWidth() );
	DrawColumn( L"Size" , GetFileTypeWidth() );
}


//////////////////////////////////////////////////////////////////////////

EGWndFileBrowserFileViewBody::EGWndFileBrowserFileViewBody( EGWndPanel* InParent , EGWndFileBrowser* InOwnerBrowser , const EGWndFileBrowserFileViewHeader* InHeader ) 
: Super( InParent, EGWnd_GetFontSize( egw_font_t::DEFAULT ) )
, m_Header( InHeader )
, m_OwnerBrowser( InOwnerBrowser )
{
	SetDoubleClicksAllowed( true );
}

void EGWndFileBrowserFileViewBody::InitView()
{
	SetDirectory( L"" );
}

void EGWndFileBrowserFileViewBody::SetDirectory(eg_cpstr16 NewDirectory)
{
	m_CurrentDirectory = NewDirectory;
	m_CurrentDirectory.Append( "/" );
	m_CurrentDirectory = EGPath2_CleanPath( *m_CurrentDirectory , L'/' );

	RefreshView( true );
}

eg_size_t EGWndFileBrowserFileViewBody::GetNumItems() const
{
	return m_CurrentContents.Len();
}

void EGWndFileBrowserFileViewBody::OnDrawBg( EGWndDc& Dc )
{
	Dc.DcDrawFilledRect( GetViewArea() , EGWnd_GetDcBrush( egw_color_t::BG_STATIC ) );
}

void EGWndFileBrowserFileViewBody::OnPaint( EGWndDc& Dc)
{	
	eg_recti rc = GetViewArea();
	rc.top -= m_ScrollPos;

	Dc.DcSetBkMode( eg_wnd_dc_bk_mode::Transparent );

	auto DrawTextAndAdvanceY = [&Dc, &rc, this]( const egOsFileInfo& FileInfo , eg_bool bSelected ) -> void
	{
		Dc.DcSetTextColor( bSelected ? egw_color_t::FG_EDIT : egw_color_t::FG_STATIC );
		Dc.DcSetFont( bSelected ? EGWndDcFont(m_SelectedItemFont) : EGWndDcFont(m_ItemFont) );

		if( bSelected )
		{
			eg_recti SelectionRect = rc;
			SelectionRect.bottom = rc.top + m_ItemSize;
			Dc.DcDrawRectangle( SelectionRect , egw_color_t::BG_EDIT );
		}

		eg_recti ItemRect = rc;
		ItemRect.bottom = ItemRect.top + m_ItemSize;

		auto DrawColumn =[&Dc,&ItemRect]( eg_cpstr16 Text , eg_int ColumnWidth , eg_bool bRightAligned ) -> void
		{
			ItemRect.right = ItemRect.left + ColumnWidth;

			// Dc.DcDrawRectangle( ItemRect );

			eg_recti FontRect = ItemRect;
			FontRect.InflateThis( -2 , 0 );
			if( bRightAligned )
			{
				Dc.DcDrawText_RightAlignedSingleLine( Text , FontRect );
			}
			else
			{
				Dc.DcDrawText_LeftAlignedSingleLine( Text , FontRect );
			}

			ItemRect.left += ColumnWidth;
			ItemRect.right = ItemRect.left;
		};

		DrawColumn( *FileInfo.Name , m_Header->GetFilenameWidth() , false );
		DrawColumn( FileInfo.Attributes.bDirectory ? L"Directory" : L"File" , m_Header->GetFileTypeWidth() , false );
		DrawColumn( FileInfo.FileSize > 0 ? *EGSFormat16( L"{0:BYTESIZE}" , FileInfo.FileSize) : L"" , m_Header->GetFileTypeWidth() , true );

		rc.top += m_ItemSize;
	};

	Dc.DcSetFont( EGWndDcFont(m_ItemFont) );
	for( eg_size_t i = 0; i < m_CurrentContents.Len(); i++ )
	{
		DrawTextAndAdvanceY( m_CurrentContents[i] , m_CapturedItem < 0 ? i == m_HoveredItem : i == m_CapturedItem );
	}
}

eg_bool EGWndFileBrowserFileViewBody::OnWmUserMsg( UINT Msg , WPARAM wParam , LPARAM lParam )
{
	unused( wParam , lParam );
	
	if( Msg == WM_USER_DIRCHANGED )
	{
		RefreshView( false );
		return true;
	}
	return false;
}

void EGWndFileBrowserFileViewBody::OnWmMouseMove( const eg_ivec2& MousePos )
{
	Super::OnWmMouseMove( MousePos );

	TrackMouseLeave();

	SetHoveredItem( ClientPosToItemIndex( MousePos ) );
}

void EGWndFileBrowserFileViewBody::OnWmMouseLeave()
{
	Super::OnWmMouseLeave();

	SetHoveredItem( -1 );
}

void EGWndFileBrowserFileViewBody::OnWmLButtonDown( const eg_ivec2& MousePos )
{
	if( IsCapturing() )
	{
		EndCapture( MousePos );
	}

	m_DoubleClickItem = m_HoveredItem;

	if( m_CurrentContents.IsValidIndex( m_HoveredItem ) )
	{
		m_CapturedItem = m_HoveredItem;
		BeginCapture( MousePos );
	}
}

void EGWndFileBrowserFileViewBody::OnWmLButtonUp( const eg_ivec2& MousePos )
{
	if( IsCapturing() )
	{
		const eg_int CapturedItem = m_CapturedItem;
		const eg_bool bClickedHovered = CapturedItem == m_HoveredItem && m_CurrentContents.IsValidIndex( m_HoveredItem );
		m_CapturedItem = -1;
		EndCapture( MousePos );

		InvalidateByIndex( CapturedItem );
		InvalidateByIndex( m_HoveredItem );
		UpdateWindow( m_hwnd );

		if( bClickedHovered )
		{
			HandleItemSingleClicked( m_HoveredItem , m_CurrentContents[m_HoveredItem] );
		}
	}
}

void EGWndFileBrowserFileViewBody::OnWmRButtonDown( const eg_ivec2& MousePos )
{
	if( IsCapturing() )
	{
		EndCapture( MousePos );
	}

	m_DoubleClickItem = m_HoveredItem;

	if( m_CurrentContents.IsValidIndex( m_HoveredItem ) )
	{
		m_CapturedItem = m_HoveredItem;
		BeginCapture( MousePos );
	}
}

void EGWndFileBrowserFileViewBody::OnWmRButtonUp( const eg_ivec2& MousePos )
{
	if( IsCapturing() )
	{
		const eg_int CapturedItem = m_CapturedItem;
		const eg_bool bClickedHovered = CapturedItem == m_HoveredItem && m_CurrentContents.IsValidIndex( m_HoveredItem );
		m_CapturedItem = -1;
		EndCapture( MousePos );

		InvalidateByIndex( CapturedItem );
		InvalidateByIndex( m_HoveredItem );
		UpdateWindow( m_hwnd );

		if( bClickedHovered )
		{
			HandleItemRightClicked( m_HoveredItem , m_CurrentContents[m_HoveredItem] );
		}
	}
}

void EGWndFileBrowserFileViewBody::OnWmLButtonDoubleClick( const eg_ivec2& MousePos )
{
	unused( MousePos );

	if( m_DoubleClickItem == m_HoveredItem )
	{
		HandleItemDoubleClicked( m_HoveredItem , m_CurrentContents[m_HoveredItem] );
	}
	m_DoubleClickItem = -1;
}

void EGWndFileBrowserFileViewBody::ChangeBrowse( eg_cpstr16 DirName )
{
	if( EGString_EqualsI( DirName , L".." ) )
	{
		egPathParts2 Path = EGPath2_BreakPath( *m_CurrentDirectory );
		if( Path.Folders.Len() > 0 )
		{
			Path.Folders.DeleteByIndex( Path.Folders.Len()-1 );
			SetDirectory( *Path.GetDirectory() );
		}
	}
	else
	{
		eg_d_string16 NewDirectory = m_CurrentDirectory + DirName;
		SetDirectory( *NewDirectory );
	}
}

void EGWndFileBrowserFileViewBody::HandleItemSingleClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo )
{
	unused( ItemIndex , ClickedInfo );
}

void EGWndFileBrowserFileViewBody::HandleItemDoubleClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo )
{
	unused( ItemIndex );

	if( ClickedInfo.Attributes.bDirectory )
	{
		ChangeBrowse( *ClickedInfo.Name );
	}
	else
	{
		// MessageBoxA( GetWnd(), *EGSFormat8( "You clicked: {0}", *ClickedInfo.Name ), nullptr, MB_OK );
		if( m_OwnerBrowser )
		{
			const eg_d_string16 FullPath = GetDirFullPath() + ClickedInfo.Name;
			m_OwnerBrowser->FileDoubleClickedDelegate.ExecuteIfBound( *FullPath , ClickedInfo );
		}
	}
}

void EGWndFileBrowserFileViewBody::HandleItemRightClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo )
{
	unused( ItemIndex );

	const eg_d_string16 FullPath = GetDirFullPath() + ClickedInfo.Name;

	if( m_OwnerBrowser )
	{
		m_OwnerBrowser->FileRightClickedDelegate.ExecuteIfBound( *FullPath , ClickedInfo );
	}
}

eg_d_string16 EGWndFileBrowserFileViewBody::GetDirFullPath() const
{
	const eg_d_string16 Out = eg_d_string16(m_OwnerBrowser->GetRoot()) + m_CurrentDirectory + L"/";
	return EGPath2_CleanPath( *Out , L'/' );
}

void EGWndFileBrowserFileViewBody::RefreshView( eg_bool bResetScroll )
{
	BeginListeningForChange();

	m_CurrentContents.Clear();
	if( !m_CurrentDirectory.EqualsI( L"/" ) )
	{
		egOsFileInfo BackDir;
		BackDir.Name = L"..";
		BackDir.Attributes.bDirectory = true;
		m_CurrentContents.Append( BackDir );
	}
	if( m_OwnerBrowser )
	{
		m_OwnerBrowser->GetDirectoryContents( *m_CurrentDirectory , m_CurrentContents );
	}
	/*
	for( const egOsFileInfo& Info : m_CurrentContents )
	{
		Info.DebugPrint( "" );
	}
	*/

	if( m_OwnerBrowser )
	{
		m_OwnerBrowser->UpdateViewedDirectory( *m_CurrentDirectory );
	}

	UpdateScroll();

	if( bResetScroll )
	{
		SetScroll( 0 );
	}

	FullRedraw();
}

void EGWndFileBrowserFileViewBody::BeginListeningForChange()
{
	EndListeningForChange();

	assert( INVALID_HANDLE_VALUE == m_DirChangedHandle && INVALID_HANDLE_VALUE == m_WaitForDirChangedHandle );
	const eg_d_string16 CurrentDir = GetDirFullPath();
	m_DirChangedHandle = FindFirstChangeNotificationW( *CurrentDir , FALSE , FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_ATTRIBUTES|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE );
	assert( INVALID_HANDLE_VALUE != m_DirChangedHandle );
	if( INVALID_HANDLE_VALUE != m_DirChangedHandle )
	{
		BOOL bRes = RegisterWaitForSingleObject( &m_WaitForDirChangedHandle , m_DirChangedHandle , DirectoryContentsChangedCallback , this , INFINITE , WT_EXECUTEDEFAULT|WT_EXECUTEONLYONCE );
		assert( bRes );
	}
}

void EGWndFileBrowserFileViewBody::EndListeningForChange()
{
	if( INVALID_HANDLE_VALUE != m_WaitForDirChangedHandle )
	{
		const BOOL bRes = UnregisterWait( m_WaitForDirChangedHandle );
		assert( bRes );
		m_WaitForDirChangedHandle = INVALID_HANDLE_VALUE;
	}

	if( INVALID_HANDLE_VALUE != m_DirChangedHandle )
	{
		const BOOL bRes = FindCloseChangeNotification( m_DirChangedHandle );
		assert( bRes );
		m_DirChangedHandle = INVALID_HANDLE_VALUE;
	}
}

VOID CALLBACK EGWndFileBrowserFileViewBody::DirectoryContentsChangedCallback( _In_ PVOID lpParameter , _In_ BOOLEAN TimerOrWaitFired )
{
	EGWndFileBrowserFileViewBody* _this = reinterpret_cast<EGWndFileBrowserFileViewBody*>(lpParameter);
	if( _this && FALSE == TimerOrWaitFired )
	{
		EGLogf( eg_log_t::Verbose , "Contents of viewed directory changed..." );
		PostMessageW( _this->GetWnd() , WM_USER_DIRCHANGED , 0 , 0 );
	}
}
