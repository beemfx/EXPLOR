// (c) 2017 Beem Media

#pragma once

#include "EGWndScrollingPanel.h"

class CLArchive;

class EGLpkFileListPanel : public EGWndScrollingPanel
{
private: typedef EGWndScrollingPanel Super;

public:

	static const eg_int FILENAME_WIDTH = 500;
	static const eg_int COLUMN_WIDTH = 100;

private:

	struct egFileInfo
	{
		eg_string_big Filename;
		eg_size_t     nOffset;
		eg_size_t     nSize;
		eg_size_t     nCmpSize;
		eg_bool       bIsCompressed;
	};

private:

	EGArray<egFileInfo> m_FileList;
	eg_int              m_ItemSize;

public:

	EGLpkFileListPanel( EGWndPanel* Parent );
	virtual ~EGLpkFileListPanel() override;

	void OnArchiveUpdated( CLArchive& Archive );


	virtual eg_size_t GetNumItems() const override;

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam ) override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void OnDrawBg( HDC hdc ) override;
	virtual void OnWmMouseLeave() override;
};

class EGLpkFileListPanelHeader : public EGWndPanel
{
private: typedef EGWndPanel Super;

public:

	EGLpkFileListPanelHeader( EGWndPanel* Parent );

	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
};