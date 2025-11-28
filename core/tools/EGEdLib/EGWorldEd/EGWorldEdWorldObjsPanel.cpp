// (c) 2018 Beem Media

#include "EGWorldEdWorldObjsPanel.h"
#include "EGWorldEd.h"
#include "../EGEdResLib/resource.h"
#include "EGResourceLib.h"
#include "EGWorldObject.h"
#include "EGWorldFile.h"

EGWorldEdWorldObjsPanel::EGWorldEdWorldObjsPanel( EGWndPanel* Parent ) 
: Super( Parent )
{
	m_DragCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGDEFED_BETWEEN_CURSOR" );
	m_DragCantDropCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGDEFED_DRAG_CURSOR_X" );
	m_DragMakeChildCursor = LoadCursorW( EGResourceLib_GetLibrary() , L"EGDEFED_MAKECHILD_CURSOR" );

	auto LoadImageFromRes = []( eg_cpstr16 Name ) -> HBITMAP
	{
		return static_cast<HBITMAP>(LoadImageW( EGResourceLib_GetLibrary() , Name , IMAGE_BITMAP , 0 , 0 , LR_SHARED ) );
	};

	m_Bmps[static_cast<eg_uint>(eg_bmp_t::Child)] = LoadImageFromRes ( L"EGDefEdBmpChild" );
	m_Bmps[static_cast<eg_uint>(eg_bmp_t::Mesh)] = LoadImageFromRes ( L"EGDefEdBmpMesh" );
	m_Bmps[static_cast<eg_uint>(eg_bmp_t::Text)] = LoadImageFromRes ( L"EGDefEdBmpText" );

	UpdateScroll();
}


EGWorldEdWorldObjsPanel::~EGWorldEdWorldObjsPanel()
{
	DeleteObject( m_InsertBrush );
}

void EGWorldEdWorldObjsPanel::OnPreDrawItems( HDC hdc )
{
	Super::OnPreDrawItems( hdc );

	m_DrawLastDepth = 0;
	m_DrawBitmapDc = CreateCompatibleDC( hdc );
}

void EGWorldEdWorldObjsPanel::OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
{
	Super::OnDrawItem( hdc , ItemIndex , rc , bIsBeingDroppedOn , bIsHovered , bIsSelected );

	const egItemInfo& Info = m_Items[ItemIndex];

	auto DrawIcon = [&hdc,this]( eg_bmp_t Type , eg_int x , eg_int y ) -> void
	{
		HGDIOBJ OldObj = SelectObject( m_DrawBitmapDc , m_Bmps[static_cast<eg_uint>(Type)] );
		BitBlt( hdc , x , y , 12 , 12 , m_DrawBitmapDc , 0 , 0 , SRCCOPY );
		SelectObject( m_DrawBitmapDc , OldObj );

	};

	const eg_int WidthPerIcon = 14;
	eg_bool bDepthGreater = m_DrawLastDepth < Info.Depth;
	m_DrawLastDepth = Info.Depth;

	if( bDepthGreater && m_DrawLastDepth > 0 )
	{
		DrawIcon( eg_bmp_t::Child , rc.left + 2 + WidthPerIcon*(m_DrawLastDepth-1) , rc.top + 2 );
	}
	eg_int NumIcons = Info.Depth;

	DrawIcon( eg_bmp_t::Mesh , rc.left + 2 + WidthPerIcon*NumIcons , rc.top + 2 );
	NumIcons++;

	RECT rcLineSize = rc;
	InflateRect( &rcLineSize , -5 , -1 );
	rcLineSize.left += WidthPerIcon*NumIcons;
	eg_string FinalText;
	if( Info.IsLocked() )
	{
		FinalText = EGString_Format( "(L) %s" , Info.Desc.String() );
	}
	else
	{
		FinalText = Info.Desc;
	}
	DrawTextA( hdc , FinalText , -1 , &rcLineSize , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX );
}

void EGWorldEdWorldObjsPanel::OnPostDrawItems( HDC hdc )
{
	DeleteDC( m_DrawBitmapDc );
	m_DrawBitmapDc = nullptr;

	Super::OnPostDrawItems( hdc );
}

eg_bool EGWorldEdWorldObjsPanel::CanDragItem( eg_int ItemIndex )
{
	return m_Items.IsValidIndex( ItemIndex ) && m_Items[ItemIndex].IsMoveable();
}

void EGWorldEdWorldObjsPanel::OnItemRightClicked( eg_int IndexClicked )
{
	eg_uint SubMenuIndex = 0;

	if( m_Items.IsValidIndex( IndexClicked ) )
	{
		SubMenuIndex = m_Items[IndexClicked].IsLocked() ? 2 : 0;
	}
	else
	{
		SubMenuIndex = 1;
	}

	POINT HitPos;
	GetCursorPos( &HitPos );

	TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGWORLDED_COMPONENT_POPUPS" ) , SubMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr );
}

void EGWorldEdWorldObjsPanel::OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex )
{
	EGWorldObject* ObjToMove = m_Items.IsValidIndex( IndexDragged ) ? m_Items[IndexDragged].Object : nullptr;
	EGWorldObject* ObjToMoveBefore = m_Items.IsValidIndex( BeforeIndex ) ? m_Items[BeforeIndex].Object : nullptr;

	if( ObjToMove != ObjToMoveBefore  )
	{
		EGWorldEd_MoveObject( ObjToMove , ObjToMoveBefore , nullptr );
	}
}

void EGWorldEdWorldObjsPanel::OnDraggedOnto( eg_int IndexDragged , eg_int OnIndex )
{
	EGWorldObject* ObjToMove = m_Items.IsValidIndex( IndexDragged ) ? m_Items[IndexDragged].Object : nullptr;
	EGWorldObject* ObjForParent = m_Items.IsValidIndex( OnIndex ) ? m_Items[OnIndex].Object : nullptr;

	if( ObjToMove != ObjForParent )
	{
		EGWorldEd_MoveObject( ObjToMove , nullptr , ObjForParent );
	}
}

void EGWorldEdWorldObjsPanel::OnSelectedItemChanged( eg_int NewFocusedIndex )
{
	EGWorldEd_SetWorldObjectToEdit( m_Items.IsValidIndex(NewFocusedIndex) ? m_Items[NewFocusedIndex].Object : nullptr );
}

void EGWorldEdWorldObjsPanel::OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator )
{
	unused( bFromAccelerator );

	switch( CmdId )
	{
	case ID_OPERATION_INSERTCOMPONENT:
	{
		MessageBoxW( GetWnd() , L"Not implemented" , L"EG World Editor" , MB_OK );
	} break;
	case ID_OPERATION_DELETEOBJECT:
	{
		EGWorldObject* Object = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].Object : nullptr;
		if( Object && !Object->IsLockedInTool() )
		{
			EGWorldEd_DeleteWorldObject( Object );
		}
	} break;;
	case ID_OPERATION_TOGGLELOCKED:
	{
		OnToggleLocked();
	} break;
	}

}

void EGWorldEdWorldObjsPanel::RefreshPanel( eg_int NewSelectedIndex )
{
	m_Items.Clear();

	EGArray<EGWorldObject*> AllObjs;
	EGWorldEd_GetWorldFile().GetAllWorldObjects( AllObjs );

	const eg_size_t NumObjs = AllObjs.Len();

	for( eg_size_t i=0; i<NumObjs; i++ )
	{
		EGWorldObject* Object = AllObjs[i];
		if( Object )
		{
			egItemInfo NewInfo;
			NewInfo.Desc = *Object->GetToolDesc();
			NewInfo.Index = i;
			NewInfo.Object = Object;
			NewInfo.Depth = Object->GetDepth();
			m_Items.Append( NewInfo );
		}
	}

	SetSelectedIndex( NewSelectedIndex );

	UpdateScroll();

	FullRedraw();
}

void EGWorldEdWorldObjsPanel::OnToggleLocked()
{
	if( m_Items.IsValidIndex( m_SelectedIndex ) )
	{
		egItemInfo& ItemInfo = m_Items[m_SelectedIndex];
		ItemInfo.ToggleLocked();
		RefreshPanel( m_SelectedIndex );
		OnSelectedItemChanged( GetSelectedIndex() );
		EGWorldEd_SetDirty();
	}
}

EGWorldObject* EGWorldEdWorldObjsPanel::GetSelectedWorldObject() const
{
	return m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].Object : nullptr;
}

void EGWorldEdWorldObjsPanel::SetSelectedWorldObject( const EGWorldObject* Info )
{
	for( eg_int i=0; i<m_Items.LenAs<eg_int>(); i++ )
	{
		if( m_Items[i].Object == Info )
		{
			SetSelectedIndex( i );
			FullRedraw();
			break;
		}
	}
}

eg_bool EGWorldEdWorldObjsPanel::egItemInfo::IsMoveable() const
{
	return Object && !Object->IsLockedInTool();
}

eg_bool EGWorldEdWorldObjsPanel::egItemInfo::IsLocked() const
{
	return Object && Object->IsLockedInTool();
}

void EGWorldEdWorldObjsPanel::egItemInfo::ToggleLocked()
{
	if( Object )
	{
		Object->SetLockedInTool( !Object->IsLockedInTool() );
	}
}
