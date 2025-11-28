// (c) 2017 Beem Media

#include "EGDefEdComponentsPanel.h"
#include "EGDefEdFile.h"
#include "EGDefEdPropPanel.h"
#include "../EGEdResLib/resource.h"
#include "EGDefEd.h"
#include "EGResourceLib.h"
#include "EGComponent.h"

EGDefEdComponentsPanel* EGDefEdComponentsPanel::GlobalMenuLayoutPanel = nullptr;

EGDefEdComponentsPanel::EGDefEdComponentsPanel( EGWndPanel* Parent ) 
: Super( Parent )
{
	assert( nullptr == GlobalMenuLayoutPanel );
	GlobalMenuLayoutPanel = this;

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


EGDefEdComponentsPanel::~EGDefEdComponentsPanel()
{
	DeleteObject( m_InsertBrush );
	
	assert( this == GlobalMenuLayoutPanel );
	GlobalMenuLayoutPanel = nullptr;
}

void EGDefEdComponentsPanel::OnPreDrawItems( HDC hdc )
{
	Super::OnPreDrawItems( hdc );

	m_DrawLastDepth = 0;
	m_DrawBitmapDc = CreateCompatibleDC( hdc );
}

void EGDefEdComponentsPanel::OnDrawItem( HDC hdc , eg_int ItemIndex , const RECT& rc , eg_bool bIsBeingDroppedOn , eg_bool bIsHovered , eg_bool bIsSelected ) const
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
	DrawTextA( hdc , Info.Desc , -1 , &rcLineSize , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX );
}

void EGDefEdComponentsPanel::OnPostDrawItems( HDC hdc )
{
	DeleteDC( m_DrawBitmapDc );
	m_DrawBitmapDc = nullptr;

	Super::OnPostDrawItems( hdc );
}

eg_bool EGDefEdComponentsPanel::CanDragItem( eg_int ItemIndex )
{
	return m_Items.IsValidIndex( ItemIndex ) && m_Items[ItemIndex].IsMoveable();
}

void EGDefEdComponentsPanel::OnItemRightClicked( eg_int IndexClicked )
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

	TrackPopupMenu( GetSubMenu( LoadMenuW( EGResourceLib_GetLibrary() , L"EGDEFED_COMPONENT_POPUPS" ) , SubMenuIndex ) , TPM_RIGHTBUTTON , HitPos.x , HitPos.y , 0 , GetWnd() , nullptr );
}

void EGDefEdComponentsPanel::OnDragBefore( eg_int IndexDragged , eg_int BeforeIndex )
{
	EGComponent* ObjToMove = m_Items.IsValidIndex( IndexDragged ) ? m_Items[IndexDragged].CompInfo : nullptr;
	EGComponent* ObjToMoveBefore = m_Items.IsValidIndex( BeforeIndex ) ? m_Items[BeforeIndex].CompInfo : nullptr;

	if( ObjToMove != ObjToMoveBefore  )
	{
		EGDefEdFile::Get().MoveObject( ObjToMove , ObjToMoveBefore , nullptr );
	}
}

void EGDefEdComponentsPanel::OnDraggedOnto( eg_int IndexDragged , eg_int OnIndex )
{
	EGComponent* ObjToMove = m_Items.IsValidIndex( IndexDragged ) ? m_Items[IndexDragged].CompInfo : nullptr;
	EGComponent* ObjForParent = m_Items.IsValidIndex( OnIndex ) ? m_Items[OnIndex].CompInfo : nullptr;

	if( ObjToMove != ObjForParent )
	{
		EGDefEdFile::Get().MoveObject( ObjToMove , nullptr , ObjForParent );
	}
}

void EGDefEdComponentsPanel::OnSelectedItemChanged( eg_int NewFocusedIndex )
{
	if( m_Items.IsValidIndex( NewFocusedIndex ) )
	{
		egItemInfo& FocusObj = m_Items[NewFocusedIndex];
		EGDefEdPropPanel::GetPanel()->SetNoPropsMessage( EGDefEdPropPanel::GetNoPropsMessage( FocusObj.CompInfo == nullptr , FocusObj.IsLocked() ) );
		if( FocusObj.CompInfo )
		{
			EGDefEdPropPanel::GetPanel()->SetEditObject( FocusObj.IsLocked() ? nullptr : FocusObj.CompInfo->GetEditor() );
		}
	}
	else
	{
		EGDefEdPropPanel::GetPanel()->SetNoPropsMessage( EGDefEdPropPanel::GetNoPropsMessage( true , false ) );
		EGDefEdPropPanel::GetPanel()->SetEditObject( nullptr );
	}
}

void EGDefEdComponentsPanel::OnWmMenuCmd( eg_int CmdId , eg_bool bFromAccelerator )
{
	unused( bFromAccelerator );

	switch( CmdId )
	{
		case ID_OPERATION_INSERTCOMPONENT:
		{
			MessageBoxW( GetWnd() , L"Not implemented" , L"EGDefEd" , MB_OK );
		} break;
		case ID_OPERATION_DELETEOBJECT:
		{
			EGComponent* Comp = m_Items.IsValidIndex( m_SelectedIndex ) ? m_Items[m_SelectedIndex].CompInfo : nullptr;
			if( Comp && !Comp->IsLockedInTool() )
			{
				EGDefEdFile::Get().DeleteObject( Comp );
			}
		} break;;
		case ID_OPERATION_TOGGLELOCKED:
		{
			OnToggleLocked();
		} break;
	}

}

void EGDefEdComponentsPanel::RefreshPanel( eg_int NewSelectedIndex )
{
	m_Items.Clear();

	const eg_size_t NumObjs = EGDefEdFile::Get().GetNumObjects();

	for( eg_size_t i=0; i<NumObjs; i++ )
	{
		EGComponent* CompInfo = EGDefEdFile::Get().GetComponentInfoByIndex( i );

		egItemInfo NewInfo;
		NewInfo.Desc = CompInfo->GetToolDesc();
		NewInfo.Index = i;
		NewInfo.CompInfo = CompInfo;
		NewInfo.Depth = CompInfo->GetDepth();
		m_Items.Append( NewInfo );
	}

	SetSelectedIndex( NewSelectedIndex );

	UpdateScroll();

	FullRedraw();
}

void EGDefEdComponentsPanel::OnToggleLocked()
{
	if( m_Items.IsValidIndex( m_SelectedIndex ) )
	{
		egItemInfo& ItemInfo = m_Items[m_SelectedIndex];
		ItemInfo.ToggleLocked();
		RefreshPanel( m_SelectedIndex );
		OnSelectedItemChanged( GetSelectedIndex() );
		EGDefEd_SetDirty();
	}
}

EGComponent* EGDefEdComponentsPanel::GetEditComponent() const
{
	return EGDefEdFile::Get().GetComponentInfoByIndex( GetSelectedIndex() );
}

void EGDefEdComponentsPanel::SetSelectedComponentByDef( const EGComponent* Info )
{
	for( eg_size_t i=0; i<m_Items.Len(); i++ )
	{
		if( m_Items[i].CompInfo == Info )
		{
			SetSelectedIndex( static_cast<eg_int>(i) );
			FullRedraw();
			break;
		}
	}
}

eg_bool EGDefEdComponentsPanel::egItemInfo::IsMoveable() const
{
	return CompInfo && !CompInfo->IsLockedInTool();
}

eg_bool EGDefEdComponentsPanel::egItemInfo::IsLocked() const
{
	return CompInfo && CompInfo->IsLockedInTool();
}

void EGDefEdComponentsPanel::egItemInfo::ToggleLocked()
{
	if( CompInfo )
	{
		CompInfo->SetLockedInTool( !CompInfo->IsLockedInTool() );
	}
}
