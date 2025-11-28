// (c) 2018 Beem Media

#pragma once


#include "EGWnd.h"
#include "EGWndChildBase.h"

class EGWndTextNodeWndControl : public EGWndChildBase
{
	EG_DECL_SUPER( EGWndChildBase )

public:

	enum class eg_commit_action
	{
		None,
		PressEnter,
		PressShiftEnter,
	};

private:

	eg_d_string       m_Text = CT_Clear;
	HWND              m_EditControl = nullptr;
	HFONT             m_Font = nullptr;
	eg_commit_action  m_CommitAction = eg_commit_action::PressEnter;
	eg_bool           m_bMultiline = false;
	eg_bool           m_bWordWrap = true;
	eg_bool           m_bStatic = false;
	eg_bool           m_bFocused = false;
	eg_bool           m_bDirty = false;
	eg_bool           m_bWantTab = false;
	eg_bool           m_bCommitOnFocusLost = true;
	eg_bool           m_bUseDirtyState = true;
	eg_bool           m_bTabRegistered = false;

	static const eg_recti DEFAULT_POSE;

public:

	EGWndTextNodeWndControl( EGWndBase* Parent );
	EGWndTextNodeWndControl( EGWndBase* Parent , const eg_recti& Pose );
	~EGWndTextNodeWndControl();

	virtual HBRUSH OnWmCtlColorEdit( HWND WndControl , HDC hdc ) override;
	virtual HBRUSH OnWmCtlColorStatic( HWND WndControl , HDC hdc ) override;
	virtual void OnWmControlCmd( HWND WndControl , eg_int NotifyId , eg_int ControlId ) override;
	virtual void OnWmSize( const eg_ivec2& NewClientSize ) override;
	virtual void OnWmEgCommit( HWND CommitCtrl  ) override;
	virtual void OnPaint( HDC hdc ) override;

	void SetText( eg_cpstr NewText );
	eg_string_big GetText() const;

	void SetStatic( eg_bool bStatic );
	void SetCommitAction( eg_commit_action NewCommitAction );
	void SetMultiline( eg_bool bMultiline );
	void SetWordWrap( eg_bool bNewWordWrap );
	void SetWantTab( eg_bool bNewWantTab ) { m_bWantTab = bNewWantTab; }
	void SetCommitOnFocusLost( eg_bool bNewValue ) { m_bCommitOnFocusLost = bNewValue; }
	void SetUseDirtyState( eg_bool bNewValue );
	void SetFont( HFONT Font );
	void SetDirty( eg_bool bDirty );
	eg_bool IsDirty() const { return m_bDirty; }
	void SetCursorStart();
	void SetCursorEnd();
	void SetCursorPos( eg_int NewCursorPos );
	eg_int GetCursorPos();

protected:

	void UpdateAppearance();
	void RebuildControl();
	void SyncText();

	static LRESULT CALLBACK EditControlSubclassProc( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam , UINT_PTR uIdSubclass , DWORD_PTR dwRefData );
};

