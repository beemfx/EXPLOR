// (c) 2019 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGWndScrollingPanel.h"
#include "EGOsFile.h"
#include "EGDelegate.h"

class EGWndFileBrowserFolderView;
class EGWndFileBrowserFileViewShell;
class EGWndFileBrowserFileViewHeader;
class EGWndFileBrowserFileViewBody;
class EGWndTabbedPanel;
class EGWndDc;

class EGWndFileBrowser : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

public:

	EGRawDelegate<void,eg_cpstr16/*FullPath*/,const egOsFileInfo&/*FileInfo*/> FileDoubleClickedDelegate;
	EGRawDelegate<void,eg_cpstr16/*FullPath*/,const egOsFileInfo&/*FileInfo*/> FileRightClickedDelegate;

private:

	eg_d_string16 m_Root;
	eg_d_string16 m_CurrentDirectory;

	EGWndPanelHeader*              m_Header = nullptr;
	EGWndFileBrowserFolderView*    m_FolderView = nullptr;
	EGWndFileBrowserFileViewShell* m_FileViewShell = nullptr;

	EGWndTabbedPanel*const m_TabOwner = nullptr;

public:

	EGWndFileBrowser( EGWndPanel* InParent , EGWndTabbedPanel* InTabOwner );

	void SetRoot( eg_cpstr16 Root );
	void SetDirectory( eg_cpstr16 NewDirectory );
	void UpdateViewedDirectory( eg_cpstr16 NewDirectory );
	eg_cpstr16 GetViewedDirectory() const { return *m_CurrentDirectory; }
	eg_cpstr16 GetRoot() const { return *m_Root; }
	void GetDirectoryContents( eg_cpstr16 Directory , EGArray<egOsFileInfo>& Out ) const;

private:

	void UpdateHeaderText();
};

class EGWndFileBrowserFolderView : public EGWndScrollingPanel
{
	EG_DECL_SUPER( EGWndScrollingPanel )

private:

	EGWndFileBrowser*const m_OwnerBrowser;

public:

	EGWndFileBrowserFolderView( EGWndPanel* InParent , EGWndFileBrowser* InOwnerBrowser );

	void InitView();

	virtual eg_size_t GetNumItems() const override;
	virtual void OnDrawBg( EGWndDc& Dc ) override;
	virtual void OnPaint( EGWndDc& Dc ) override;
};

class EGWndFileBrowserFileViewShell : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	EGWndFileBrowser*const          m_OwnerBrowser;
	EGWndFileBrowserFileViewHeader* m_Header = nullptr;
	EGWndFileBrowserFileViewBody*   m_FolderView = nullptr;

public:

	EGWndFileBrowserFileViewShell( EGWndPanel* InParent , EGWndFileBrowser* InOwnerBrowser );

	void InitView();
	void SetDirectory( eg_cpstr16 NewDirectory );
};

class EGWndFileBrowserFileViewHeader : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	static const eg_int FILENAME_WIDTH = 300;
	static const eg_int COLUMN_WIDTH = 150;

public:

	EGWndFileBrowserFileViewHeader( EGWndPanel* InParent )
	: Super( InParent , eg_panel_size_t::Fixed , EGWnd_GetFontSize( egw_font_t::DEFAULT_BOLD ) )
	{

	}

	virtual void OnDrawBg( EGWndDc& Dc ) override;
	virtual void OnPaint( EGWndDc& Dc ) override;

	eg_int GetFilenameWidth() const { return EGWnd_GetAppScaledSize( FILENAME_WIDTH ); }
	eg_int GetFileTypeWidth() const { return EGWnd_GetAppScaledSize( COLUMN_WIDTH ); }
};

class EGWndFileBrowserFileViewBody : public EGWndScrollingPanel
{
	EG_DECL_SUPER( EGWndScrollingPanel )

private:

	EGWndFileBrowser*const                     m_OwnerBrowser;
	const EGWndFileBrowserFileViewHeader*const m_Header;
	eg_d_string16                              m_CurrentDirectory;
	EGArray<egOsFileInfo>                      m_CurrentContents;
	eg_int                                     m_CapturedItem = -1;
	eg_int                                     m_DoubleClickItem = -1;
	HANDLE                                     m_DirChangedHandle = INVALID_HANDLE_VALUE;
	HANDLE                                     m_WaitForDirChangedHandle = INVALID_HANDLE_VALUE;

	static const UINT WM_USER_DIRCHANGED = WM_EG_USER+20;

public:

	EGWndFileBrowserFileViewBody( EGWndPanel* InParent , EGWndFileBrowser* InOwnerBrowser , const EGWndFileBrowserFileViewHeader* InHeader );
	~EGWndFileBrowserFileViewBody(){ EndListeningForChange(); }

	void InitView();
	void SetDirectory( eg_cpstr16 NewDirectory );

	virtual eg_size_t GetNumItems() const override;
	virtual void OnDrawBg( EGWndDc& Dc ) override;
	virtual void OnPaint( EGWndDc& Dc ) override;
	virtual eg_bool OnWmUserMsg( UINT Msg , WPARAM wParam , LPARAM lParam ) override;
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override;
	virtual void OnWmMouseLeave() override;
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos ) override;
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ) override;
	virtual void OnWmRButtonDown( const eg_ivec2& MousePos ) override;
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ) override;
	virtual void OnWmLButtonDoubleClick( const eg_ivec2& MousePos ) override;

	void ChangeBrowse( eg_cpstr16 DirName );

protected:

	void HandleItemSingleClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo );
	void HandleItemDoubleClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo );
	void HandleItemRightClicked( eg_int ItemIndex , const egOsFileInfo& ClickedInfo );

	eg_d_string16 GetDirFullPath() const;

	void RefreshView( eg_bool bResetScroll );

private:

	void BeginListeningForChange();
	void EndListeningForChange();

	static VOID CALLBACK DirectoryContentsChangedCallback( _In_ PVOID lpParameter , _In_ BOOLEAN TimerOrWaitFired );
};
