// (c) 2020 Beem Media. All rights reserved.

#pragma once

#include "EGUiWidget.h"
#include "EGEntObj.h"
#include "EGUiGrid2.h"
#include "EGUiGridWidgetItem.h"
#include "EGUiTypes.h"

class EGUiGridWidget2;
struct egUiGridWidgetCellChangedInfo2;
struct egUiGridWidgetCellClickedInfo2;

typedef EGDelegate<void,egUiGridWidgetCellClickedInfo2&> EGUiGridWidgetClickedDelegate2; // void CellClickedFunction( egUiGridWidgetCellClickedInfo2& CellInfo );
typedef EGDelegate<void,EGUiGridWidget2*,eg_uint> EGUiGridWidgetIndexDelegate2; // void CellIndexFunction( EGUiGridWidget2* GridOwner , eg_uint CellIndex );
typedef EGDelegate<void,egUiGridWidgetCellChangedInfo2&> EGUiGridWidgetChangedDelegate2; // void CellChangedFunction( egUiGridWidgetCellChangedInfo2& CellInfo );

struct egUiGridWidgetCellChangedInfo2
{
	EGUiGridWidget2* Owner = nullptr;
	eg_uint GridIndex = 0;
	eg_uint TopOffsetIndex = 0;
	EGUiGridWidgetItem* GridItem = nullptr;
	bool bIsSelected = false;
	bool bSelectionChanged = false;

	eg_bool IsNewlySelected() const { return bSelectionChanged && bIsSelected; }
	eg_bool IsNewlyDeselected() const { return bSelectionChanged && !bIsSelected; }
};

struct egUiGridWidgetCellClickedInfo2
{
	EGUiGridWidget2* Owner = nullptr;
	eg_int GridIndex = 0;
	EGUiGridWidgetItem* GridItem = nullptr;
	eg_bool bFromMouse = false;
	eg_vec2 HitPoint = CT_Clear;
};

class EGUiGridWidget2 : public EGUiWidget , public IEGUiGrid2OwnerInterface
{
	EG_CLASS_BODY( EGUiGridWidget2 , EGUiWidget )

	struct egInitData;

protected:

	EGEntObj                   m_MaskObj;
	EGEntObj                   m_TemplateObj;
	EGUiGrid2                  m_Grid;
	EGArray<EGUiGridWidgetItem*> m_GridItems;
	eg_string_crc              m_CrcId; //Id of this widget for text and material callbacks
	eg_vec4                    m_ScaleVector;
	eg_real                    m_GridWidgetScale = 1.f;
	eg_vec2                    m_AdditionalOffset = eg_vec2(0.f,0.f);
	eg_uint                    m_SavedSelection; // When moving a list in and out of focus, we usually want to restore the last selected index.
	eg_string_crc              m_ResetEvent = eg_crc("Reset");
	eg_bool                    m_SelectionChanged:1;
	eg_bool                    m_SelectionAnimationPlayed:1;
	eg_bool                    m_bFocusedByMouse:1;
	eg_bool                    m_bFocusOnHover = true;
	eg_bool                    m_bChangeSelectionOnHover = true;
	eg_bool                    m_bAudioMuted = false;
	eg_uint                    m_CaptureGridIndex = 0;

public:

	// New interface meant to supersede the above delegates
	EGUiGridWidgetIndexDelegate2 CellSelectionChangedDelegate;
	EGUiGridWidgetChangedDelegate2 CellChangedDelegate;
	EGUiGridWidgetClickedDelegate2 CellClickedDelegate;

public:

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	EGUiGridWidget2(){ }
	~EGUiGridWidget2();

	void SetGridWidgetScale( eg_real NewValue ) { m_GridWidgetScale = NewValue; }
	void SetAdditionalOffset( const eg_vec2& NewValue ) { m_AdditionalOffset = NewValue; }

	eg_vec2 ByScrollbar_GetHitPointPos() const { return m_Grid.ByScrollbar_GetHitPointPos(); }
	eg_bool ByScrollbar_IsHitOnHandle( const eg_vec2& ScrollbarHitPoint ) const { return m_Grid.ByScrollbar_IsHitOnHandle( ScrollbarHitPoint ); }
	void ByScrollbar_ScrollToHitPoint( const eg_vec2& ScrollbarHitPoint ) { return m_Grid.ByScrollbar_ScrollToHitPoint( ScrollbarHitPoint ); }
	void ChangeSelectionByHitPoint( const eg_vec2& HitPoint );
	eg_uint GetIndexByHitPoint( const eg_vec2& HitPoint , eg_vec2* AdjHitPointOut )const{ return m_Grid.GetIndexByHitPoint( HitPoint , AdjHitPointOut ); }
	void SetSelection( eg_uint Index ){ m_Grid.SetSelectedIndex( Index ); }
	void ScrollToIndex( eg_uint Index ){ m_Grid.ScrollToIndex( Index ); }
	eg_uint GetSelectedIndex() const { return m_Grid.GetSelectedIndex(); }
	eg_real GetScrollOffset() const { return m_Grid.GetScrollPos(); }
	const EGUiGridWidgetItem* GetSelectedWidget() const;
	EGUiGridWidgetItem* GetSelectedWidget();
	const EGUiGridWidgetItem* GetWidgetByIndex( eg_uint Index ) const;
	EGUiGridWidgetItem* GetWidgetByIndex( eg_uint Index );
	void SetGridWidgetFocusOnHover( eg_bool bNewValue ) { m_bFocusOnHover = bNewValue; }
	void SetGridWidgetChangeSelectionOnHover( eg_bool bNewValue ) { m_bChangeSelectionOnHover = bNewValue; }

	void Draw_MaskObj( const eg_transform& ParentPose );

	//
	// EGUiOBject Interface:
	//
	virtual void Draw( eg_real AspectRatio ) override;
	virtual void Update( eg_real DeltaTime, eg_real AspectRatio ) override;
	virtual void OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint ) override;
	virtual void OnFocusLost() override final;
	virtual void SetMuteAudio( eg_bool bMute ) override;
	virtual eg_bool HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse ) override;
	virtual eg_bool IsFocusable() const override { return true; }
	virtual eg_bool FocusOnHover() const override { return m_bFocusOnHover; }
	virtual eg_bool IsWidget() const { return true; }
	virtual eg_bool IsHitPointValid( const eg_vec2& WidgetHitPoint ) const override;
	virtual eg_bool OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured  ) override;
	virtual eg_bool OnMouseMovedOn( const eg_vec2& WidgetHitPoint ) override;
	virtual void OnMouseMovedOff() override;
	virtual eg_bool OnMousePressed( const eg_vec2& WidgetHitPoint );
	virtual eg_bool OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn );
	virtual void OnSetSelectedIndex( eg_uint Index ) override;
	eg_aabb GetBoundsInMouseSpace( eg_real AspectRatio, const eg_mat& MatView, const eg_mat& MatProj ) const override;
	virtual void QueryTextNodes( EGArray<eg_d_string>& Out ) override;

	//
	// ISdkMenuObj Interface:
	//
	virtual void RunEvent( eg_string_crc Event ) override final;
	virtual void SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t ) override final;
	virtual void SetCBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Pose ) override final;
	virtual void SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath ) override final;
	virtual void SetPalette( eg_uint PaletteIndex , const eg_vec4& Palette ) override final;
	virtual void SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText ) override final;
	virtual eg_uint GetSelectedIndex() override final;

	eg_bool IsItemSelected() const;

	eg_bool IsAudioMuted() const { return m_bAudioMuted; }

	void RefreshGridWidget( eg_uint NumItems );
	void SetWrapType( eg_grid_wrap_t WrapType ){ m_Grid.SetWrapType( WrapType ); }

	eg_uint GetNumVisibleColumns() const { return m_Grid.GetNumVisibleColumns(); }
	eg_uint GetNumVisibleRows() const { return m_Grid.GetNumVisibleRows(); }

protected:
	
	virtual void OnGridWidgetScrolled();

private:

	void InitInternal( const struct egInitData& InitData , eg_sampler_s OverrideSamplerState );
	void DeinitInternal();

	void DrawGrid( const eg_transform& Pose );
	eg_bool UpdateGrid( eg_real DeltaTime );
	void SelectWidgetInternal( eg_bool FromMouse, const eg_vec2& WidgetHitPoint );
	void DeselectWidgetInternal();
	eg_bool HandledInputInternal( eg_menuinput_t Event );

	eg_aabb GetBoundingBox( const eg_mat& WorldPose );
	eg_aabb GetBoundingBoxForVisibleItem( eg_uint VisItemIndex, const eg_mat& WorldPose );

	void RefreshDisplayItems( eg_bool bSelectionChanged );

	virtual void UiGrid_OnSelectionChanged( const EGUiGrid2* UiGrid , eg_uint NewSelectedIndex ) override;
	virtual void UiGrid_OnUpdateCells( const EGUiGrid2* Grid , eg_uint NewFirstVisibleIndex , eg_uint NewNumVisibleItems ) override;

private:

	class EGUiScrollbarWidget* GetScrollbarOfThisGrid();

private:

	struct egInitData
	{
		eg_string_crc CrcId;
		eg_string_crc ElementCrc;
		eg_string_small GridItemClassName;
		egUiGrid2Config GridInit;
		eg_uint GridColumns = 0;
		eg_uint GridRows = 0;
		eg_real ItemWidth = 0.f;
		eg_real ItemHeight = 0.f;
		eg_vec4 ElementScaleVec = eg_vec4( 1.f, 1.f, 1.f, 1.f );

		egInitData( eg_ctor_t Ct )
		: CrcId( Ct )
		, ElementCrc( Ct )
		, GridItemClassName( Ct )
		{

		}
	};

};