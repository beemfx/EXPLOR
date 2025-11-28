#include "EGUiGridWidget.h"
#include "EGUiScrollbarWidget.h"
#include "EGUiLayout.h"
#include "EGMenu.h"
#include "EGCamera2.h"
#include "EGGlobalConfig.h"

EG_CLASS_DECL( EGUiGridWidget )

void EGUiGridWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	OnItemChangedDelegate.Bind( this , &ThisClass::DefaultOnGridWidgetItemChanged );

	m_ScaleVector = eg_vec4( m_Info->ScaleVec , 1.f );

	m_TemplateObj.Init( InInfo->EntDefCrc );

	egInitData GridInitData( CT_Clear );
	GridInitData.GridItemClassName = m_Info->GridInfo.ObjectClassName.Class ? m_Info->GridInfo.ObjectClassName.Class->GetName() : "";
	GridInitData.CrcId = m_Info->Id;
	GridInitData.ElementCrc = m_Info->EntDefCrc;
	GridInitData.ElementScaleVec = m_ScaleVector;
	GridInitData.GridColumns = m_Info->GridInfo.Cols;
	GridInitData.GridRows = m_Info->GridInfo.Rows;
	if( m_TemplateObj.IsValid() )
	{
		GridInitData.ItemWidth = m_TemplateObj.GetBaseBounds().GetWidth()*m_ScaleVector.x;
		GridInitData.ItemHeight = m_TemplateObj.GetBaseBounds().GetHeight()*m_ScaleVector.y;
	}

	eg_sampler_s OverrideSamplerState = eg_sampler_s::COUNT;
	if( InInfo->bUsePointFiltering || InInfo->bUseTextureEdgeClamp )
	{
		if( InInfo->bUsePointFiltering )
		{
			OverrideSamplerState = InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT : eg_sampler_s::TEXTURE_WRAP_FILTER_POINT;
		}
		else
		{
			OverrideSamplerState = InInfo->bUseTextureEdgeClamp ? eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT : eg_sampler_s::TEXTURE_WRAP_FILTER_DEFAULT;
		}
	}

	InitInternal( GridInitData , OverrideSamplerState );

	m_Grid.SetWrapType( InInfo->GridInfo.WrapType );

	if( InInfo->GridInfo.bAutoMask )
	{
		m_MaskObj.Init( eg_crc("Mask1x1") );
	}
}

EGUiGridWidget::~EGUiGridWidget()
{
	m_MaskObj.Deinit();
	m_TemplateObj.Deinit();
	DeinitInternal();
}

void EGUiGridWidget::ChangeSelectionByHitPoint( const eg_vec2& HitPoint )
{
	eg_uint NewIndex = m_Grid.GetIndexByHitPoint( HitPoint, nullptr );
	if( EGUiGrid2::INDEX_NONE != NewIndex )
	{
		m_Grid.SetSelectedIndex( NewIndex );
	}
}

EGUiScrollbarWidget* EGUiGridWidget::GetScrollbarOfThisGrid()
{
	EGUiScrollbarWidget* Scrollbar = nullptr;

	if( m_Info->GridInfo.ScrollbarId.Crc.IsNotNull() )
	{
		Scrollbar = EGCast<EGUiScrollbarWidget>( m_Owner->GetWidget( m_Info->GridInfo.ScrollbarId ) );
	}

	return Scrollbar;
}

const EGUiGridWidgetItem* EGUiGridWidget::GetWidgetByIndex( eg_uint Index ) const
{
	return const_cast<EGUiGridWidget*>(this)->GetWidgetByIndex( Index );
}

EGUiGridWidgetItem* EGUiGridWidget::GetWidgetByIndex( eg_uint Index )
{
	if( Index == GetSelectedIndex() )
	{
		return m_SelectedObj;
	}
	return m_GridItems.IsValidIndex( m_Grid.GetVisibilityIndex( Index ) ) ? m_GridItems.GetByIndex(m_Grid.GetVisibilityIndex( Index )) : nullptr;
}

void EGUiGridWidget::Draw_MaskObj( const eg_transform& ParentPose )
{
	eg_real ItemWidth = m_TemplateObj.GetBaseBounds().GetWidth()*m_ScaleVector.x;
	eg_real ItemHeight = m_TemplateObj.GetBaseBounds().GetHeight()*m_ScaleVector.y;

	eg_real GridWidth = ItemWidth*m_Info->GridInfo.Cols;
	eg_real GridHeight = ItemHeight*m_Info->GridInfo.Rows;

	eg_vec4 ScaleVec = eg_vec4( GridWidth , GridHeight , 1.f , 0.f );
	eg_transform BasePos = ParentPose;
	BasePos = eg_transform::BuildIdentity();

	eg_real xBase = (GridWidth-ItemWidth)/2.f;
	eg_real yBase = (-GridHeight + ItemHeight) / 2.f;

	// Adjust for the fact that the object may not be centered:
	xBase += m_TemplateObj.GetBaseBounds().Min.x*m_ScaleVector.x + ItemWidth/2.f;
	yBase += m_TemplateObj.GetBaseBounds().Min.y*m_ScaleVector.y + ItemHeight/2.f;

	eg_real zOffset = 0.f;//-1.f;
	eg_real yOffset = yBase;
	eg_real xOffset = xBase;

	BasePos.TranslateThis( xOffset , yOffset , zOffset );
	BasePos *= ParentPose;
	
	m_MaskObj.SetDrawInfo( BasePos , ScaleVec , false );

	eg_bool bShowMask = DebugConfig_DrawGridMasks.GetValue();
	
	if( !bShowMask ){ MainDisplayList->PushBlendState( eg_blend_s::BLEND_NONE_COLOR_NONE ); }
	MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_OFF_STEST_NONE_SWRITE_1 );
	m_MaskObj.Draw();
	if( !bShowMask ){ MainDisplayList->PopBlendState(); }
	MainDisplayList->PopDepthStencilState();
}

void EGUiGridWidget::Draw( eg_real AspectRatio )
{
	eg_transform Pose = GetFullPose( AspectRatio);

	if( m_Info->GridInfo.bAutoMask )
	{
		Draw_MaskObj( Pose );
	}

	if( m_Info->GridInfo.bAutoMask )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_DEFAULT_ZWRITE_ON_STEST_NONZERO );
		EGTextNode::SetUseStencilIgnore( true );
	}

	DrawGrid( Pose );

	if( m_Info->GridInfo.bAutoMask )
	{
		MainDisplayList->PopDepthStencilState();
		EGTextNode::SetUseStencilIgnore( false );
	}
}

void EGUiGridWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	unused( AspectRatio );

	m_MaskObj.Update( DeltaTime );

	eg_bool Moved = UpdateGrid( DeltaTime );

	if( Moved )
	{
		OnGridWidgetScrolled();
	}

	if( m_Info->GridInfo.bAutoMask )
	{
		eg_bool bShowMask = DebugConfig_DrawGridMasks.GetValue();
		m_MaskObj.RunEvent( bShowMask ? eg_crc("ShowForDebug") : eg_crc("Hide") );
	}

	// The grid also needs to update it's scrollbar:
	EGUiScrollbarWidget* Scrollbar = GetScrollbarOfThisGrid();
	if( Scrollbar )
	{
		eg_real PageSize;
		eg_real TrackPos;
		m_Grid.GetNormalizedTrackInfo( &PageSize, &TrackPos );
		if( PageSize <= ( 1.f - EG_SMALL_EPS ) ) //If Page size is greater than or equal to 1.f it means that there is no scrolling to be done.
		{
			const eg_aabb& TrackBox = Scrollbar->GetEntObj().GetBaseBounds();
			eg_real TrackExtend = Scrollbar->GetInfo()->TrackExtend;

			Scrollbar->GetEntObj().ClearCustomBones();
			eg_transform Transform;
			Transform = eg_transform::BuildIdentity();
			TrackPos = EGMath_GetMappedRangeValue( TrackPos, eg_vec2( 0.f, 1.f ), eg_vec2( 0, TrackBox.GetHeight() + TrackExtend ) );
			PageSize = EGMath_GetMappedRangeValue( PageSize, eg_vec2( 0.f, 1.f ), eg_vec2( 0, TrackBox.GetHeight() + TrackExtend ) );
			Transform.TranslateThis( 0, -TrackPos, 0 );
			Scrollbar->GetEntObj().SetCustomBone( eg_crc("Base") , eg_crc( "handle_top" ), Transform );
			// By specification the handle bottom starts in the center of the scrollbar, so
			// we need to move it from there.
			Transform = eg_transform::BuildIdentity();
			Transform.TranslateThis( 0, TrackBox.GetHeight()*.5f - PageSize - TrackPos, 0 );
			Scrollbar->GetEntObj().SetCustomBone( eg_crc("Base") , eg_crc( "handle_bottom" ), Transform );

			Transform = eg_transform::BuildTranslation( eg_vec3(0, -TrackExtend, 0) );
			Scrollbar->GetEntObj().SetCustomBone( eg_crc("Base") , eg_crc( "track_bottom" ), Transform );

			Scrollbar->SetVisible( m_bIsVisible );
		}
		else
		{
			Scrollbar->SetVisible( false );
		}
	}
}

void EGUiGridWidget::OnFocusGained( eg_bool FromMouse , const eg_vec2& WidgetHitPoint )
{
	m_bFocusedByMouse = FromMouse;

	SelectWidgetInternal( FromMouse , WidgetHitPoint );

	egUIWidgetEventInfo ItemInfo( eg_widget_event_t::FocusGained );
	ItemInfo.WidgetId = m_Info->Id;
	ItemInfo.Widget = m_SelectedObj;
	ItemInfo.GridWidgetOwner = this;
	ItemInfo.bIsGrid = true;
	ItemInfo.bSelected = true;
	ItemInfo.bNewSelection = true;
	ItemInfo.GridIndex = GetSelectedIndex();
	OnFocusGainedDelegate.ExecuteIfBound( ItemInfo );
}

void EGUiGridWidget::OnFocusLost()
{
	DeselectWidgetInternal();

	egUIWidgetEventInfo ItemInfo( eg_widget_event_t::FocusLost );
	ItemInfo.WidgetId = m_Info->Id;
	ItemInfo.Widget = m_SelectedObj;
	ItemInfo.GridWidgetOwner = this;
	ItemInfo.bIsGrid = true;
	ItemInfo.bSelected = false;
	ItemInfo.bNewSelection = true;
	ItemInfo.GridIndex = GetSelectedIndex();
	m_bFocusedByMouse = false;
	OnFocusLostDelegate.ExecuteIfBound( ItemInfo );
}

void EGUiGridWidget::SetMuteAudio( eg_bool bMute )
{
	for( EGUiGridWidgetItem* GridItem : m_GridItems )
	{
		GridItem->SetMuteAudio( bMute );
	}
	m_SelectedObj->SetMuteAudio( bMute );
}

eg_bool EGUiGridWidget::HandleInput( eg_menuinput_t Event, const eg_vec2& WidgetHitPoint , eg_bool bFromMouse )
{
	eg_bool Handled = false;

	eg_uint PrevSelectedIndex = GetSelectedIndex();
	Handled = HandledInputInternal( Event );
	eg_uint NewSelectedIndex = GetSelectedIndex();
	if( Handled && ( PrevSelectedIndex != NewSelectedIndex ) )
	{
		egUIWidgetEventInfo ItemInfo( eg_widget_event_t::FocusGained );
		ItemInfo.WidgetId = m_Info->Id;
		ItemInfo.Widget = this;
		ItemInfo.bIsGrid = true;
		ItemInfo.bSelected = true;
		ItemInfo.bNewSelection = true;
		ItemInfo.GridIndex = NewSelectedIndex;
		OnFocusGainedDelegate.ExecuteIfBound( ItemInfo );
	}

	if( Handled )
	{
		return true;
	}

	if( eg_menuinput_t::BUTTON_PRIMARY == Event )
	{
		eg_vec2 AdjButtonHitPoint = WidgetHitPoint;
		eg_uint HitIndex = GetIndexByHitPoint( WidgetHitPoint, &AdjButtonHitPoint );
		if( !bFromMouse || HitIndex == GetSelectedIndex() )
		{
			{
				egUIWidgetEventInfo ObjPressInfo( eg_widget_event_t::ItemClicked );
				ObjPressInfo.WidgetId = m_Info->Id;
				ObjPressInfo.GridWidgetOwner = this;
				ObjPressInfo.Widget = m_SelectedObj;
				ObjPressInfo.GridIndex = GetSelectedIndex();
				ObjPressInfo.HitPoint = AdjButtonHitPoint;
				ObjPressInfo.bFromMouse = bFromMouse;
				OnItemPressedDelegate.ExecuteIfBound( ObjPressInfo );
			}
		}

		{
			CellClickedDelegate.ExecuteIfBound( this , bFromMouse ? HitIndex : GetSelectedIndex() );
		}

		return true;
	}

	return false;
}

eg_bool EGUiGridWidget::IsHitPointValid( const eg_vec2& WidgetHitPoint ) const
{
	eg_uint HitIndex = GetIndexByHitPoint( WidgetHitPoint , nullptr );
	return EGUiGrid2::INDEX_NONE != HitIndex;
}

eg_bool EGUiGridWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	// If we're a grid widget, we may need to change the selection anyway.
	if( !bIsMouseCaptured && m_bChangeSelectionOnHover )
	{
		eg_uint PrevSelectedIndex = GetSelectedIndex();
		ChangeSelectionByHitPoint( WidgetHitPoint );
		eg_uint NewSelectedIndex = GetSelectedIndex();
		return true;
	}

	return Super::OnMouseMovedOver( WidgetHitPoint , bIsMouseCaptured );
}

eg_bool EGUiGridWidget::OnMouseMovedOn( const eg_vec2& WidgetHitPoint )
{
	Super::OnMouseMovedOn( WidgetHitPoint );
	
	if( m_Owner && IsFocusable() && FocusOnHover() )
	{
		eg_uint NewIndex = m_Grid.GetIndexByHitPoint( WidgetHitPoint , nullptr );
		m_Owner->SetFocusedWidget( this , NewIndex , true );
		m_bFocusedByMouse = true;
	}

	return true;
}

void EGUiGridWidget::OnMouseMovedOff()
{
	Super::OnMouseMovedOff();
	// EGLogf( eg_log_t::Warning , "Mouse Off" );
}

eg_bool EGUiGridWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	unused( WidgetHitPoint );

	if( m_Owner )
	{
		m_CaptureGridIndex = m_Grid.GetIndexByHitPoint( WidgetHitPoint , nullptr );
		if( EG_IsBetween<eg_uint>( m_CaptureGridIndex , 0 , m_Grid.GetNumItems() ) )
		{
			m_Owner->BeginMouseCapture( this );
		}
	}
	return true;
}

eg_bool EGUiGridWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint, EGUiWidget* WidgetReleasedOn )
{
	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		m_Owner->EndMouseCapture();
		if( this == WidgetReleasedOn )
		{
			eg_uint ReleasedIndex = m_Grid.GetIndexByHitPoint( WidgetHitPoint , nullptr );
			if( m_CaptureGridIndex == ReleasedIndex )
			{
				HandleInput( eg_menuinput_t::BUTTON_PRIMARY , WidgetHitPoint , true );
			}
		}
	}
	return true;
}

void EGUiGridWidget::OnSetSelectedIndex( eg_uint Index )
{
	Super::OnSetSelectedIndex( Index );
	
	SetSelection( Index );
	ScrollToIndex( Index );
}

eg_aabb EGUiGridWidget::GetBoundsInMouseSpace( eg_real AspectRatio, const eg_mat& MatView, const eg_mat& MatProj ) const
{
	//Form the button bounding box.

	//1) Transform the location of the button to screen space.
	const eg_transform Pose = GetFullPose( AspectRatio );

	//3) Create a bound box with the specified bounds.

	eg_aabb ButtonDims;
	if( m_TemplateObj.IsValid() )
	{
		ButtonDims = m_TemplateObj.GetBaseBounds();
	}
	else
	{
		zero( &ButtonDims );
	}

	ButtonDims.Max.x += ButtonDims.GetWidth()*( m_Info->GridInfo.Cols - 1 );
	ButtonDims.Min.y -= ButtonDims.GetHeight()*( m_Info->GridInfo.Rows - 1 );

	ButtonDims.Min.x *= m_ScaleVector.x;
	ButtonDims.Min.y *= m_ScaleVector.y;
	ButtonDims.Min.z *= m_ScaleVector.z;
	ButtonDims.Min.w *= m_ScaleVector.w;

	ButtonDims.Max.x *= m_ScaleVector.x;
	ButtonDims.Max.y *= m_ScaleVector.y;
	ButtonDims.Max.z *= m_ScaleVector.z;
	ButtonDims.Max.w *= m_ScaleVector.w;

	ButtonDims.Min.w = 1.f;
	ButtonDims.Max.w = 1.f;

	eg_vec4 Corners[8];
	ButtonDims.Get8Corners( Corners, countof( Corners ) );

	for( eg_vec4& Corner : Corners )
	{
		Corner *= Pose;

		eg_vec2 CornerMouseSpace = EGCamera2::WorldSpaceToMouseSpace( Corner, AspectRatio, MatView, MatProj );
		Corner.x = CornerMouseSpace.x;
		Corner.y = CornerMouseSpace.y;
		Corner.z = 0.f;
		Corner.w = 1.f;
	}

	ButtonDims.CreateFromVec4s( Corners, countof( Corners ) );

	return ButtonDims;
}

void EGUiGridWidget::QueryTextNodes( EGArray<eg_d_string>& Out )
{
	if( m_GridItems.GetFirst() )
	{
		m_GridItems.GetFirst()->QueryTextNodes( Out );
	}
}

void EGUiGridWidget::RunEvent( eg_string_crc Event )
{
	for( EGUiGridWidgetItem* GridItem : m_GridItems )
	{
		GridItem->RunEvent( Event );
	}
	m_SelectedObj->RunEvent( Event );
}

void EGUiGridWidget::SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t )
{
	unused( NodeId , SkeletonId , AnimationId , t );
	assert( false ); // Not implemented for grids
}

void EGUiGridWidget::SetCBone( eg_string_crc NodeId, eg_string_crc BoneId, const eg_transform& Pose )
{
	unused( NodeId , BoneId , Pose );
	assert( false ); // Not implemented for grids
}

void EGUiGridWidget::SetTexture( eg_string_crc NodeId, eg_string_crc GroupId, eg_cpstr TexturePath )
{
	unused( NodeId , GroupId , TexturePath );
	assert( false ); // Not implemented for grids
}

void EGUiGridWidget::SetPalette( eg_uint PaletteIndex, const eg_vec4& Palette )
{
	unused( PaletteIndex , Palette );
	assert( false ); // Not implemented for grids
}

void EGUiGridWidget::SetText( eg_string_crc TextNodeCrcId, const eg_loc_text& NewText )
{
	unused( TextNodeCrcId , NewText );
	assert( false ); // Text is handled differently for grids.
}

eg_uint EGUiGridWidget::GetSelectedIndex()
{
	return m_Grid.GetSelectedIndex();
}

void EGUiGridWidget::RefreshGridWidget( eg_uint NumItems )
{
	SetMuteAudio( true );
	m_Grid.Resize( NumItems );
	SetMuteAudio( false );
}

void EGUiGridWidget::DefaultOnGridWidgetItemChanged( const egUIWidgetEventInfo& EventInfo )
{
	if( EventInfo.IsNewlySelected() )
	{
		if( m_FocusedEvent )
		{
			EventInfo.Widget->RunEvent( m_FocusedEvent );
		}
	}
	else if( EventInfo.IsNewlyDeselected() )
	{
		if( m_NotFocusedEvent )
		{
			EventInfo.Widget->RunEvent( m_NotFocusedEvent );
		}
	}
}

void EGUiGridWidget::OnGridWidgetScrolled()
{
	if( m_Owner )
	{
		if( m_bFocusedByMouse && m_Owner->GetFocusedWidget() == this && m_Owner->GetWasLastInputByMouse() )
		{
			eg_vec2 MousePos = m_Owner->GetMousePos();
			m_Owner->HandleMouseEvent( eg_menumouse_e::MOVE , MousePos.x , MousePos.y );
		}
	}
}

void EGUiGridWidget::InitInternal( const egInitData& InitData , eg_sampler_s OverrideSamplerState )
{
	EGClass* GridItemClass = FindUiObjectClass( &EGUiGridWidgetItem::GetStaticClass(), nullptr, InitData.GridItemClassName , InitData.ElementCrc , false , true );

	m_SelectedObj = EGNewObject<EGUiGridWidgetItem>( GridItemClass, eg_mem_pool::DefaultHi );
	m_SelectedObj->InitGridInfo( this, InitData.ElementCrc , m_ResetEvent );
	if( OverrideSamplerState != eg_sampler_s::COUNT )
	{
		m_SelectedObj->SetOverrideSamplerState( OverrideSamplerState );
	}

	egUiGrid2Config GridInitData;
	GridInitData.VisibleCols = InitData.GridColumns;
	GridInitData.VisibleRows = InitData.GridRows;
	GridInitData.ItemWidth = InitData.ItemWidth;
	GridInitData.ItemHeight = InitData.ItemHeight;
	GridInitData.ScrollDir = eg_uigrid_scroll_dir::UpDown;
	GridInitData.Owner = this;
	m_SavedSelection = EGUiGrid2::INDEX_NONE;

	m_Grid.Init( GridInitData );

	eg_uint NumClones = m_Grid.GetMaxVisibleItems();
	for( eg_uint i = 0; i < NumClones; i++ )
	{
		EGUiGridWidgetItem* NewItem = EGNewObject<EGUiGridWidgetItem>( GridItemClass , eg_mem_pool::DefaultHi );
		NewItem->InitGridInfo( this , InitData.ElementCrc , m_ResetEvent );
		if( OverrideSamplerState != eg_sampler_s::COUNT )
		{
			NewItem->SetOverrideSamplerState( OverrideSamplerState );
		}
		m_GridItems.InsertLast( NewItem );
	}

	m_ScaleVector = InitData.ElementScaleVec;
	m_CrcId = InitData.CrcId;
}

void EGUiGridWidget::DeinitInternal()
{
	while( m_GridItems.HasItems() )
	{
		EGUiGridWidgetItem* GridItem = m_GridItems.GetFirst();
		m_GridItems.Remove( GridItem );
		EGDeleteObject( GridItem );
	}

	EGDeleteObject( m_SelectedObj );
}

void EGUiGridWidget::DrawGrid( const eg_transform& Pose )
{
	assert( m_Grid.GetNumVisibleItems() <= m_GridItems.Len() );

	const eg_uint VisItems = EG_Min( m_Grid.GetNumVisibleItems(), static_cast<eg_uint>( m_GridItems.Len() ) );

	for( eg_uint i = 0; i < VisItems; i++ )
	{
		egUiGrid2ItemInfo ItemInfo = m_Grid.GetVisibleItemInfo( i );
		EGUiGridWidgetItem* GridItem = m_GridItems.GetByIndex( i );
		if( ItemInfo.Index == m_Grid.GetSelectedIndex() )
		{
			GridItem = m_SelectedObj;
		}

		eg_transform FinalPose( CT_Default );
		FinalPose = eg_transform::BuildTranslation( eg_vec3(ItemInfo.Offset.x, ItemInfo.Offset.y, 0.f) );
		FinalPose = FinalPose*Pose;
		GridItem->DrawAsGridItem( FinalPose , m_ScaleVector , m_Info->IsLit );

		if( DebugConfig_DrawMenuButtons.GetValue() )
		{
			MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
			MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
			MainDisplayList->SetWorldTF( eg_mat::I );
			eg_aabb Box = GetBoundingBoxForVisibleItem( i, eg_mat( Pose ) );
			MainDisplayList->DrawAABB( Box, eg_color(eg_color32( 255, 15, 240 )) );
			MainDisplayList->PopDefaultShader();
			MainDisplayList->PopDepthStencilState();
		}
	}

	if( DebugConfig_DrawMenuButtons.GetValue() )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::TEXTURE );
		MainDisplayList->SetWorldTF( eg_mat::I );
		eg_aabb Box = GetBoundingBox( eg_mat( Pose ) );
		MainDisplayList->DrawAABB( Box, eg_color(eg_color32( 255, 0, 255 )) );
		MainDisplayList->PopDefaultShader();
		MainDisplayList->PopDepthStencilState();
	}
}

eg_bool EGUiGridWidget::UpdateGrid( eg_real DeltaTime )
{
	eg_real OldPos = m_Grid.GetScrollPos();

	for( EGUiGridWidgetItem* GridItem : m_GridItems )
	{
		GridItem->Update( DeltaTime, 0.f );
	}
	m_SelectedObj->Update( DeltaTime, 0.f );
	m_Grid.Update( DeltaTime );
	eg_bool Moved = EG_Abs( OldPos - m_Grid.GetScrollPos() ) > EG_SMALL_EPS;
	return Moved;
}

void EGUiGridWidget::SelectWidgetInternal( eg_bool FromMouse, const eg_vec2& WidgetHitPoint )
{
	if( FromMouse )
	{
		ChangeSelectionByHitPoint( WidgetHitPoint );
	}
	else
	{
		if( EGUiGrid2::INDEX_NONE == m_SavedSelection || !EG_IsBetween<eg_uint>( m_SavedSelection, 0, m_Grid.GetNumItems() - 1 ) )
		{
			if( m_Grid.GetNumItems() > 0 )
			{
				m_SavedSelection = 0;
			}
		}
		m_Grid.SetSelectedIndex( m_SavedSelection );
	}
}

void EGUiGridWidget::DeselectWidgetInternal()
{
	m_SavedSelection = m_Grid.GetSelectedIndex();
	m_Grid.SetSelectedIndex( EGUiGrid2::INDEX_NONE );
}

eg_bool EGUiGridWidget::HandledInputInternal( eg_menuinput_t Event )
{
	eg_bool bHandled = true;

	switch( Event )
	{
	case eg_menuinput_t::BUTTON_DOWN: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::Down ); break;
	case eg_menuinput_t::BUTTON_UP: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::Up ); break;
	case eg_menuinput_t::BUTTON_LEFT: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::Left ); break;
	case eg_menuinput_t::BUTTON_RIGHT: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::Right ); break;
	case eg_menuinput_t::SCROLL_UP: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::WheelUp ); break;
	case eg_menuinput_t::SCROLL_DOWN: bHandled = m_Grid.HandleInputEvent( eg_uigrid_input_event::WheelDown ); break;
	default: bHandled = false; break;
	}

	return bHandled;
}

eg_aabb EGUiGridWidget::GetBoundingBox( const eg_mat& WorldPose )
{
	eg_aabb Box;

	eg_real Left = m_SelectedObj->GetObjBaseBounds().Min.x*m_ScaleVector.x;
	eg_real Right = Left + m_Grid.GetItemWidth()*m_Grid.GetNumVisibleColumns();

	eg_real Top = m_SelectedObj->GetObjBaseBounds().Max.y*m_ScaleVector.y;
	eg_real Bottom = Top - m_Grid.GetItemHeight()*m_Grid.GetNumVisibleRows();

	Box.Min = eg_vec4( Left, Bottom, 0, 1 );
	Box.Max = eg_vec4( Right, Top, 0, 1 );

	Box.Min *= WorldPose;
	Box.Max *= WorldPose;

	return Box;
}

eg_aabb EGUiGridWidget::GetBoundingBoxForVisibleItem( eg_uint VisItemIndex, const eg_mat& WorldPose )
{
	eg_aabb GridBox = GetBoundingBox( WorldPose );

	egUiGrid2ItemInfo ItemInfo = m_Grid.GetVisibleItemInfo( VisItemIndex );

	eg_aabb Box = GridBox;
	Box.Min.y = Box.Max.y - m_Grid.GetItemHeight();
	Box.Max.x = Box.Min.x + m_Grid.GetItemWidth();

	Box.Min += eg_vec4( ItemInfo.Offset.x, ItemInfo.Offset.y, 0, 0 );
	Box.Max += eg_vec4( ItemInfo.Offset.x, ItemInfo.Offset.y, 0, 0 );

	return Box;
}

void EGUiGridWidget::RefreshDisplayItems( eg_bool bSelectionChanged )
{
	eg_bool bSelectedItemIsVisible = false;

	eg_uint SelectedIndex = m_Grid.GetSelectedIndex();

	for( eg_uint i = 0; i < m_Grid.GetNumVisibleItems(); i++ )
	{
		egUiGrid2ItemInfo ItemInfo = m_Grid.GetVisibleItemInfo( i );

		EGUiGridWidgetItem* ItemWidget = m_GridItems.GetByIndex( i );
		if( ItemInfo.Index == SelectedIndex )
		{
			ItemWidget = m_SelectedObj;
			bSelectedItemIsVisible = true;
		}

		{
			egUIWidgetEventInfo NewItemInfo(eg_widget_event_t::UpdateGridItem);
			NewItemInfo.bIsGrid = true;
			NewItemInfo.WidgetId = m_CrcId;
			NewItemInfo.Widget = ItemWidget;
			NewItemInfo.GridWidgetOwner = this;
			NewItemInfo.GridIndex = ItemInfo.Index;
			NewItemInfo.bSelected = ItemInfo.Index == SelectedIndex;
			NewItemInfo.bNewSelection = bSelectionChanged;
			OnItemChangedDelegate.ExecuteIfBound( NewItemInfo );
		}

		{
			egUiGridWidgetCellChangedInfo CellChangedInfo;
			CellChangedInfo.Owner = this;
			CellChangedInfo.GridIndex = ItemInfo.Index;
			CellChangedInfo.TopOffsetIndex = i;
			CellChangedInfo.GridItem = ItemWidget;
			CellChangedInfo.bIsSelected = ItemInfo.Index == SelectedIndex;
			CellChangedInfo.bSelectionChanged = bSelectionChanged;
			CellChangedDelegate.ExecuteIfBound( CellChangedInfo );
		}
	}

	if( EGUiGrid2::INDEX_NONE != SelectedIndex && !bSelectedItemIsVisible )
	{
		{
			egUIWidgetEventInfo NewItemInfo( eg_widget_event_t::UpdateGridItem );
			NewItemInfo.bIsGrid = true;
			NewItemInfo.WidgetId = m_CrcId;
			NewItemInfo.Widget = m_SelectedObj;
			NewItemInfo.GridWidgetOwner = this;
			NewItemInfo.GridIndex = SelectedIndex;
			NewItemInfo.bSelected = true;
			NewItemInfo.bNewSelection = bSelectionChanged;
			OnItemChangedDelegate.ExecuteIfBound( NewItemInfo );
		}

		{
			egUiGridWidgetCellChangedInfo CellChangedInfo;
			CellChangedInfo.Owner = this;
			CellChangedInfo.GridIndex = SelectedIndex;
			CellChangedInfo.TopOffsetIndex = EGUiGrid2::INDEX_NONE;
			CellChangedInfo.GridItem = m_SelectedObj;
			CellChangedInfo.bIsSelected = true;
			CellChangedInfo.bSelectionChanged = bSelectionChanged;
			CellChangedDelegate.ExecuteIfBound( CellChangedInfo );
		}
	}

	if( bSelectionChanged )
	{
		CellSelectionChangedDelegate.ExecuteIfBound( this , m_Grid.GetSelectedIndex() );
	}
}

void EGUiGridWidget::UiGrid_OnSelectionChanged( const EGUiGrid2* UiGrid , eg_uint NewSelectedIndex )
{
	unused( UiGrid , NewSelectedIndex );
	
	RefreshDisplayItems( true );
}

void EGUiGridWidget::UiGrid_OnUpdateCells( const EGUiGrid2* Grid , eg_uint NewFirstVisibleIndex , eg_uint NewNumVisibleItems )
{
	unused( Grid , NewFirstVisibleIndex , NewNumVisibleItems );

	//EGLogf( "Top row changed %u->%u" , NewFirstVisibleIndex , NewNumVisibleItems );
	RefreshDisplayItems( false );
}
