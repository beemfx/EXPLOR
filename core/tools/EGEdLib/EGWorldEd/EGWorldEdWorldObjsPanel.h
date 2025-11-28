// (c) 2018 Beem Media

#pragma once

#include "EGWndDragableLayoutEditor.h"

class EGWorldObject;

class EGWorldEdWorldObjsPanel : public EGWndDragableLayoutEditor
{
	EG_DECL_SUPER( EGWndDragableLayoutEditor )

private:

	struct egItemInfo
	{
		eg_string      Desc = "";
		eg_size_t      Index = 0;
		eg_uint        Depth = 0;
		EGWorldObject* Object = nullptr;

		eg_bool IsMoveable() const;
		eg_bool IsLocked() const;
		void ToggleLocked();
	};

	enum class eg_bmp_t
	{
		Child,
		Mesh,
		Text,

		COUNT,
	};


	typedef EGArray<egItemInfo> EGItemArray;

private:

	EGItemArray m_Items;
	HBITMAP     m_Bmps[enum_count(eg_bmp_t)];

	HDC     m_DrawBitmapDc = nullptr;
	mutable eg_uint m_DrawLastDepth = 0;

public:

	EGWorldEdWorldObjsPanel( EGWndPanel* Parent );
	~EGWorldEdWorldObjsPanel();

	// BEGIN EGWndDragableLayoutEditor
	virtual void OnPreDrawItems( HDC hdc );
	virtual void OnDrawItem( HDC hdc, eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const;
	virtual void OnPostDrawItems( HDC hdc );
	virtual eg_bool CanDragItem( eg_int ItemIndex );
	virtual eg_size_t GetNumItems() const override final{ return m_Items.Len(); }
	virtual void OnItemRightClicked( eg_int IndexClicked ) override final;
	virtual void OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex ) override final;
	virtual void OnDraggedOnto( eg_int IndexDragged , eg_int OnIndex ) override final;
	virtual void OnSelectedItemChanged( eg_int NewFocusedIndex ) override final;

	virtual void OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator ) override final;
	// END EGWndDragableLayoutEditor

	void RefreshPanel( eg_int NewSelectedIndex );
	void OnToggleLocked();

	EGWorldObject* GetSelectedWorldObject() const;
	void SetSelectedWorldObject( const EGWorldObject* Info );
};
