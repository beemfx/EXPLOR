// (c) 2016 Beem Media

#include "EGLytEdLayoutPanel.h"
#include "EGUiLayout.h"
#include "EGLytEdFile.h"
#include "EGLytEdPropPanel.h"
#include "EGLytEd.h"
#include "EGUiWidgetInfo.h"
#include "EGResourceLib.h"
#include "../EGEdResLib/resource.h"

EGLytEdLayoutPanel* EGLytEdLayoutPanel::GlobalMenuLayoutPanel = nullptr;

EGLytEdLayoutPanel::EGLytEdLayoutPanel( EGWndPanel* Parent ) 
: Super( Parent )
{
	assert( nullptr == GlobalMenuLayoutPanel );
	GlobalMenuLayoutPanel = this;

	m_DragCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGLAYOUT_BETWEEN_CURSOR" );
	m_DragCantDropCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGLAYOUT_DRAG_CURSOR_X" );
	m_DragType = eg_drag_t::Between;

	RefreshPanel( -1 );

	UpdateScroll();
}


EGLytEdLayoutPanel::~EGLytEdLayoutPanel()
{
	assert( this == GlobalMenuLayoutPanel );
	GlobalMenuLayoutPanel = nullptr;
}

void EGLytEdLayoutPanel::OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
{
	Super::OnDrawItem( hdc , ItemIndex , rc , bIsBeingDroppedOn , bIsHovered , bIsSelected );
	
	const egItemInfo& Item = m_Items[ItemIndex];
	RECT rcLineSize = rc;
	InflateRect( &rcLineSize , -5 , -1 );
	DrawTextA( hdc , Item.Desc , -1 , &rcLineSize , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX );
}

eg_bool EGLytEdLayoutPanel::CanDragItem( eg_int ItemIndex )
{
	return m_Items.IsValidIndex( ItemIndex ) && m_Items[ItemIndex].IsMoveable();
}

void EGLytEdLayoutPanel::OnItemRightClicked( eg_int IndexClicked )
{
	POINT HitPos;
	GetCursorPos( &HitPos );

	eg_uint SubMenuIndex = 0;

	if( m_Items.IsValidIndex( IndexClicked ) )
	{
		SubMenuIndex = 0;
	}
	else
	{
		SubMenuIndex = 1;
	}

	TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGLAYOUT_MENULAYOUT_POPUPS" ) , SubMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr );
}

void EGLytEdLayoutPanel::OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex )
{
	egUiWidgetInfo* ObjToMove = m_Items.IsValidIndex( IndexDragged ) ? m_Items[IndexDragged].ObjInfo : nullptr;
	egUiWidgetInfo* ObjToMoveBefore = m_Items.IsValidIndex( BeforeIndex ) ? m_Items[BeforeIndex].ObjInfo : nullptr;

	if( (BeforeIndex-1) != (IndexDragged) )
	{
		EGLytEdFile::Get().MoveObject( ObjToMove , ObjToMoveBefore );
	}
}

void EGLytEdLayoutPanel::OnSelectedItemChanged( eg_int NewFocusedIndex )
{
	if( m_Items.IsValidIndex( NewFocusedIndex ) )
	{
		egItemInfo& FocusObj = m_Items[NewFocusedIndex];
		egRflEditor* EditObject = nullptr;
		if( FocusObj.ObjInfo )
		{
			FocusObj.ObjInfo->RefreshEditableProperties();
			EditObject = FocusObj.ObjInfo->GetEditor();
		}
		EGLytEdPropPanel::GetPanel()->SetNoPropsMessage( EGLytEdPropPanel::GetNoPropsMessage( FocusObj.ObjInfo == nullptr , FocusObj.ObjInfo ? FocusObj.ObjInfo->bIsLocked : false ) );
		EGLytEdPropPanel::GetPanel()->SetEditObject( FocusObj.ObjInfo && FocusObj.ObjInfo->bIsLocked ? nullptr : EditObject );
	}
	else
	{
		EGLytEdPropPanel::GetPanel()->SetNoPropsMessage( EGLytEdPropPanel::GetNoPropsMessage( true , false ) );
		EGLytEdPropPanel::GetPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
	}
}

void EGLytEdLayoutPanel::OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator )
{
	unused( bFromAccelerator );
	
	switch( CmdId )
	{
	case ID_OPERATION_INSERTCLEARZ:
	{
		egUiWidgetInfo* ObjToInsertBefore = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().InsertClearZ( ObjToInsertBefore );
	} break;
	case ID_OPERATION_INSERTTEXT:
	{
		egUiWidgetInfo* ObjToInsertBefore = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().InsertText( ObjToInsertBefore );
	} break;
	case ID_OPERATION_INSERTLIGHT:
	{
		egUiWidgetInfo* ObjToInsertBefore = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().InsertLight( ObjToInsertBefore );
	} break;
	case ID_OPERATION_INSERTIMAGE:
	{
		egUiWidgetInfo* ObjToInsertBefore = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().InsertImage( ObjToInsertBefore );
	} break;
	case ID_OPERATION_INSERTCAMERA:
	{
		egUiWidgetInfo* ObjToInsertBefore = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().InsertCamera( ObjToInsertBefore );
	} break;
	case ID_OPERATION_DELETEOBJECT:
	{
		egUiWidgetInfo* Obj = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
		EGLytEdFile::Get().DeleteObject( Obj );
	} break;
	case ID_OPERATION_TOGGLELOCKED:
	{
		OnToggleLocked();

	} break;
	default:
	{
		assert( false ); // What is this?
	} break;
	}
}

void EGLytEdLayoutPanel::RefreshPanel( eg_int NewSelectedIndex )
{
	m_Items.Clear();

	const eg_size_t NumObjs = EGLytEdFile::Get().GetNumObjects();
	for( eg_size_t i=0; i<NumObjs; i++ )
	{
		egUiWidgetInfo* ObjInfo = EGLytEdFile::Get().GetObjectInfoByIndex( i );

		egItemInfo NewInfo;
		NewInfo.Desc = ObjInfo->GetToolDesc();
		NewInfo.Index = i;
		NewInfo.ObjInfo = ObjInfo;
		m_Items.Append( NewInfo );
	}

	SetSelectedIndex( NewSelectedIndex );

	UpdateScroll();

	FullRedraw();
}

void EGLytEdLayoutPanel::OnToggleLocked()
{
	egUiWidgetInfo* Obj = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].ObjInfo : nullptr;
	if( Obj )
	{
		Obj->bIsLocked = !Obj->bIsLocked;
		RefreshPanel( m_SelectedIndex );
		OnSelectedItemChanged( GetSelectedIndex() );
		EGLytEd_SetDirty();
	}
}

struct egUiWidgetInfo* EGLytEdLayoutPanel::GetEditObject() const
{
	return EGLytEdFile::Get().GetObjectInfoByIndex( GetSelectedIndex() );
}

void EGLytEdLayoutPanel::SetSelectedObjByObjInfo( const egUiWidgetInfo* Info )
{
	for( eg_size_t i=0; i<m_Items.Len(); i++ )
	{
		if( m_Items[i].ObjInfo == Info )
		{
			SetSelectedIndex( static_cast<eg_int>(i) );
			FullRedraw();
			break;
		}
	}
}

eg_bool EGLytEdLayoutPanel::egItemInfo::IsMoveable() const
{
	return ObjInfo && !ObjInfo->bIsLocked;
}
