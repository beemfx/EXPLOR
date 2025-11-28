// (c) 2016 Beem Media

#pragma once

#include "EGWndScrollingPanel.h"

class EGDefEdLibPanel : public EGWndScrollingPanel
{
private:
	
	struct egLibItem
	{
		EGClass* Class = nullptr;
		eg_string DisplayName = CT_Clear;

		egLibItem() = default;
		egLibItem( EGClass* InClass );
	};

	typedef EGWndScrollingPanel Super;
	typedef EGArray<egLibItem> EGItemArray;

public:

	EGDefEdLibPanel( EGWndPanel* Parent );
	~EGDefEdLibPanel();

	virtual LRESULT WndProc( UINT Msg, WPARAM wParam, LPARAM lParam) override final;
	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
	virtual void OnWmMouseLeave() override final;

private:

	HCURSOR     m_DragCursor;
	HCURSOR     m_DragCantDropCursor;
	HCURSOR     m_RestoreCursor;
	EGItemArray m_Items;
	eg_bool     m_bCapturing:1;

private:
	
	virtual eg_size_t GetNumItems() const override final{ return m_Items.Len(); }
};