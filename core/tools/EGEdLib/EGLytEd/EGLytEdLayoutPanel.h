// (c) 2016 Beem Media

#pragma once

#include "EGWndDragableLayoutEditor.h"
#include "EGLytEdLibPanel.h"

class EGLytEdLayoutPanel: public EGWndDragableLayoutEditor
{
	EG_DECL_SUPER( EGWndDragableLayoutEditor )

private:

	struct egItemInfo
	{
		eg_string              Desc;
		eg_size_t              Index;
		struct egUiWidgetInfo* ObjInfo;

		eg_bool IsMoveable() const;
	};

	typedef EGArray<egItemInfo> EGItemArray;

private:

	EGItemArray m_Items;

public:

	EGLytEdLayoutPanel( EGWndPanel* Parent );
	~EGLytEdLayoutPanel();

	// BEGIN EGWndDragableLayoutEditor
	virtual void OnDrawItem( HDC hdc, eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const;
	virtual eg_bool CanDragItem( eg_int ItemIndex );
	virtual eg_size_t GetNumItems() const override final{ return m_Items.Len(); }
	virtual void OnItemRightClicked( eg_int IndexClicked ) override final;
	virtual void OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex ) override final;
	virtual void OnSelectedItemChanged( eg_int NewFocusedIndex ) override final;

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override final;
	// END EGWndDragableLayoutEditor

	void RefreshPanel( eg_int NewSelectedIndex );
	void OnToggleLocked();
	struct egUiWidgetInfo* GetEditObject() const;
	void SetSelectedObjByObjInfo( const egUiWidgetInfo* Info );

public:

	static EGLytEdLayoutPanel* GetPanel(){ return GlobalMenuLayoutPanel; }

private:

	static EGLytEdLayoutPanel* GlobalMenuLayoutPanel;
};