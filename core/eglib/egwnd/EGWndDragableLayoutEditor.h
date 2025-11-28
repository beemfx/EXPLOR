// (c) 2017 Beem Media

#pragma once

#include "EGWndScrollingPanel.h"


class EGWndDragableLayoutEditor : public EGWndScrollingPanel
{
	EG_DECL_SUPER( EGWndScrollingPanel )

protected:

	enum class eg_insert_m
	{
		Before,
		AsChild,
		After,
	};

	enum class eg_drag_t
	{
		None,
		Between,
		BetweenAndOnto,
	};

protected:

	HCURSOR     m_DragCursor;
	HCURSOR     m_DragCantDropCursor;
	HCURSOR     m_DragMakeChildCursor;
	HCURSOR     m_RestoreCursor;
	HBRUSH      m_InsertBrush;

	eg_ivec2    m_MousePointAtStartOfDrag;
	eg_int      m_InsertIndex;
	eg_insert_m m_InsertMode;
	eg_int      m_ItemBeingMovedIndex;
	eg_int      m_SelectedIndex;
	eg_drag_t   m_DragType = eg_drag_t::BetweenAndOnto;
	eg_bool     m_bHasMouseMovedSinceDragStart = false;

public:

	EGWndDragableLayoutEditor( EGWndPanel* Parent );
	~EGWndDragableLayoutEditor();

	void SetDragType( eg_drag_t NewDragType );
	void SetSelectedIndex( eg_int Index );
	eg_int GetSelectedIndex() const { return m_SelectedIndex; }

	// BEGIN Override Interface
	virtual void OnPreDrawItems( HDC hdc );
	virtual void OnDrawItem( HDC hdc, eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const;
	virtual void OnPostDrawItems( HDC hdc );
	virtual eg_bool CanDragItem( eg_int ItemIndex ) { unused( ItemIndex ); return true; }
	virtual void OnItemRightClicked( eg_int IndexClicked ){ unused( IndexClicked ); EGLogf( eg_log_t::General , "Clicked %i" , IndexClicked ); }
	virtual void OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex ){ unused( IndexDragged , BeforeIndex ); EGLogf( eg_log_t::General , "Dragged %i before %i" , IndexDragged , BeforeIndex ); }
	virtual void OnDraggedOnto( eg_int IndexDragged , eg_int OnIndex ){ unused( IndexDragged , OnIndex ); EGLogf( eg_log_t::General , "Dragged %i onto %i" , IndexDragged , OnIndex ); }
	virtual void OnHoveredItemChanged( eg_int NewHoveredIndex ) override { Super::OnHoveredItemChanged( NewHoveredIndex ); unused( NewHoveredIndex ); }
	virtual void OnSelectedItemChanged( eg_int NewFocusedIndex ){ unused( NewFocusedIndex ); }
	// END Override Interface

	// BEGIN EGWndScrollingPanel
	virtual void OnPaint( HDC hdc ) override final;
	virtual void OnDrawBg( HDC hdc ) override final;
	virtual void OnWmMouseMove( const eg_ivec2& MousePos ) override final;
	virtual void OnWmMouseLeave() override final;
	virtual void OnWmRButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmRButtonUp( const eg_ivec2& MousePos ) override final;
	virtual void OnWmLButtonDown( const eg_ivec2& MousePos ) override final;
	virtual void OnWmLButtonUp( const eg_ivec2& MousePos ) override final;
	virtual eg_size_t GetNumItems() const override { return 0; }
	// END EGWndScrollingPanel

private:

	RECT GetDrawRectForItem( eg_int Index );
	RECT GetInsertBetweenRectForIndex( eg_int Index );
	void SetInsertIndex( eg_int Index , eg_insert_m InsertMode );
	void SetupDefaultCursors();
};
