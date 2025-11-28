// (c) 2020 Beem Media. All Rights Reserved.

/*
A few notes: If the grid is scrolling up and down then the indexes read like
an English book with the first row being 0 , 1 , 2 , ...
If the grid scrolling is left to right, then the indexes go in columns, so if
there were two rows the first row would be indexed 0 , 2 , 4 , ...
*/

#pragma once

#include "EGUiTypes.h"

class EGUiGrid2;
class IEGUiGrid2OwnerInterface;

enum class eg_uigrid_scroll_dir
{
	UpDown,
	LeftRight,
};

enum class eg_uigrid_input_event
{
	Down,
	Up,
	Right,
	Left,
	WheelDown,
	WheelUp,
};

class IEGUiGrid2OwnerInterface
{
public:

	virtual void UiGrid_OnSelectionChanged( const EGUiGrid2* UiGrid , eg_uint NewSelectedIndex ) { unused( UiGrid , NewSelectedIndex ); }
	virtual void UiGrid_OnUpdateCells( const EGUiGrid2* Grid , eg_uint NewFirstVisibleIndex , eg_uint NewNumVisibleItems ) { unused( Grid , NewFirstVisibleIndex , NewNumVisibleItems ); }
};

struct egUiGrid2Config
{
	eg_uint VisibleCols = 0;
	eg_uint VisibleRows = 0;
	eg_uigrid_scroll_dir ScrollDir = eg_uigrid_scroll_dir::UpDown;
	eg_grid_wrap_t WrapType = eg_grid_wrap_t::WRAP;
	eg_real ItemWidth = 0.f;
	eg_real ItemHeight = 0.f;
	IEGUiGrid2OwnerInterface* Owner = nullptr;
};

struct egUiGrid2ItemInfo
{
	eg_vec2 Offset = CT_Default;
	eg_uint Index = 0;
};

class EGUiGrid2
{
public:

	static const eg_uint INDEX_NONE = 0xFFFFFFFF;

public:

	void Init( const egUiGrid2Config& InitData );
	void Resize( eg_uint NewNumItems ); //Will possibly change selected index, and scroll immediately if the new size is smaller.
	void Update( eg_real DeltaTime );
	eg_bool HandleInputEvent( eg_uigrid_input_event Event );
	void SetSelectedIndex( eg_uint Index ); //Sets a target index, but won't scroll (usually for mouse hovering).
	void ScrollToIndex( eg_uint Index ); //Causes updates to scroll until this index is visible, but doesn't change the selection (clamped).
	void ScrollToPos( eg_real Pos , eg_bool bJump ); //Causes updates to scroll until this position is reached (clamped).
	void JumpToFinalScrollPos();
	void SetPosImmediateByIndex( eg_uint Index );
	void SetPosImmediateByPos( eg_real Pos );
	void SetWrapType( eg_grid_wrap_t WrapType ){ m_Config.WrapType = WrapType; }
	//Helpers for setting up things like scrollbars:
	eg_real GetScrollPos()const{ return m_ScrollPos; }
	eg_real GetPageSize()const;
	eg_real GetListSize()const;
	//For interacting with the state of the grid:
	eg_int GetSelectedIndex()const{ assert( (0 <= m_SelectedIndex && m_SelectedIndex < m_NumItems) || (INDEX_NONE == m_SelectedIndex) ); return m_SelectedIndex; }
	eg_uint GetNumItems()const{ return m_NumItems; }
	eg_uint GetNumVisibleItems()const;
	eg_uint GetMaxVisibleItems()const;
	egUiGrid2ItemInfo GetVisibleItemInfo( eg_uint VisibleIndex )const;
	eg_int GetVisibilityIndex( eg_uint ItemIndex ) const;
	//Additional helper functions:
	eg_real GetMaxScrollPos()const;
	eg_uint GetNumVisibleColumns()const{ return m_Config.VisibleCols; }
	eg_uint GetNumVisibleRows()const{ return m_Config.VisibleRows; }
	eg_real GetItemWidth()const{ return m_Config.ItemWidth; };
	eg_real GetItemHeight()const{ return m_Config.ItemHeight; }

	eg_uint GetIndexByHitPoint( const eg_vec2& HitPoint , eg_vec2* AdjHitPointOut )const;
	void ByScrollbar_ScrollToHitPoint( const eg_vec2& ScrollbarHitPoint );
	eg_vec2 ByScrollbar_GetHitPointPos()const;
	eg_bool ByScrollbar_IsHitOnHandle( const eg_vec2& ScrollbarHitPoint )const;
	void GetNormalizedTrackInfo( eg_real* OutPageSize , eg_real* OutTrackPos )const;

private:

	struct egHelpData
	{
		eg_real DirDim = 0.f;
		eg_uint ItemsInDir = 0;
		eg_uint ItemsPerDir = 0;
		eg_uint FirstItemIndex = 0;
		eg_uint NumVisibleItems = 0;
		eg_real ScrollOffset = 0.f;
	};

private:

	egUiGrid2Config m_Config;
	egHelpData      m_HelpData;
	eg_uint         m_NumItems = 0;
	eg_uint         m_SelectedIndex = 0;
	eg_real         m_ScrollPos = 0.f;
	eg_real         m_TargetPos = 0.f;
	eg_real         m_Velocity = 0.f;
	eg_real         m_LeftOverSimulationTime = 0.f;

private:

	void ComputeHelpData();
	void IndexToRowCol( eg_uint Index , eg_int* ColOut , eg_int* RowOut ) const;
	eg_uint RowColToIndex( eg_int Col , eg_int Row ) const;
};
