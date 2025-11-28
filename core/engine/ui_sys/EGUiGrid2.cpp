// (c) 2020 Beem Media. All Rights Reserved.

#include "EGUiGrid2.h"

void EGUiGrid2::Init( const egUiGrid2Config& InitData )
{
	assert( InitData.VisibleCols >= 1 && InitData.VisibleRows >= 1 ); //This isn't going to work.
	m_Config = InitData;
	m_NumItems = 0;
	m_SelectedIndex = INDEX_NONE;
	m_ScrollPos = 0.f;
	m_TargetPos = 0.f;
	m_Velocity = 0.f;
	m_LeftOverSimulationTime = 0.f;
	m_HelpData.FirstItemIndex = INDEX_NONE; //This will force a call to m_CbOnTopRowChanged when we init.
	ComputeHelpData();
}

void EGUiGrid2::ByScrollbar_ScrollToHitPoint( const eg_vec2& ScrollbarHitPoint )
{
	eg_real ScrollPoint = 0.f;

	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown:
			ScrollPoint = 1.f - ScrollbarHitPoint.y;
			break;
		case eg_uigrid_scroll_dir::LeftRight:
			ScrollPoint = ScrollbarHitPoint.x;
			break;
	}

	// This math basically says that the middle scrollbar hit point is where
	// the middle of the scrollbar handle should be located.
	eg_real NewScrollPos = EGMath_GetMappedRangeValue( ScrollPoint , eg_vec2(0.f,1.f) , eg_vec2( 0.f , GetMaxScrollPos()+GetPageSize() ) );
	NewScrollPos -= GetPageSize()*.5f;
	ScrollToPos( NewScrollPos , true );
}

eg_vec2 EGUiGrid2::ByScrollbar_GetHitPointPos() const
{
	eg_real ScrollPos = m_ScrollPos;
	ScrollPos += GetPageSize()*.5f;

	eg_real ScrollPoint = EGMath_GetMappedRangeValue( ScrollPos , eg_vec2( 0.f , GetMaxScrollPos()+GetPageSize() ) , eg_vec2(0.f,1.f) );

	eg_vec2 Out(0,0);

	switch( m_Config.ScrollDir )
	{
	case eg_uigrid_scroll_dir::UpDown:
		Out.y = 1.f - ScrollPoint;
		break;
	case eg_uigrid_scroll_dir::LeftRight:
		Out.x = ScrollPoint;
		break;
	}

	return Out;
}

eg_bool EGUiGrid2::ByScrollbar_IsHitOnHandle( const eg_vec2 & ScrollbarHitPoint ) const
{
	eg_real PageSize = 0;
	eg_real TrackPos = 0;
	GetNormalizedTrackInfo( &PageSize , &TrackPos );

	eg_real ScrollPoint = 0.f;

	switch( m_Config.ScrollDir )
	{
	case eg_uigrid_scroll_dir::UpDown:
		ScrollPoint = 1.f - ScrollbarHitPoint.y;
		break;
	case eg_uigrid_scroll_dir::LeftRight:
		ScrollPoint = ScrollbarHitPoint.x;
		break;
	}

	return EG_IsBetween( ScrollPoint , TrackPos , TrackPos+PageSize );
}

eg_uint EGUiGrid2::GetIndexByHitPoint( const eg_vec2& HitPoint , eg_vec2* AdjHitPointOut )const
{
	// DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Grid Mouse (%g,%g)" , HitPoint.x , HitPoint.y ) );

	//This is a little wonky, because HitPoint is not in screen space. It is
	//in "widget space" where the bottom-left of the widget is [0,0] and the
	//upper-right of the widget is [1,1] so we need to do some transformations
	//to figure out exactly which item in the list was hit.

	eg_vec2 HitPointGridSpace;
	HitPointGridSpace.x = EGMath_GetMappedRangeValue( HitPoint.x , eg_vec2(0.f,1.f) , eg_vec2(0.f,m_Config.ItemWidth*m_Config.VisibleCols) );
	HitPointGridSpace.y = EGMath_GetMappedRangeValue( HitPoint.y , eg_vec2(0.f,1.f) , eg_vec2(m_Config.ItemHeight*m_Config.VisibleRows,0.f) );
	
	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown:
			HitPointGridSpace.y += m_ScrollPos;
			break;
		case eg_uigrid_scroll_dir::LeftRight:
			HitPointGridSpace.x += m_ScrollPos;
			break;
	} 

	eg_vec2 AdjHitPoint( HitPointGridSpace.x/m_Config.ItemWidth , HitPointGridSpace.y/m_Config.ItemHeight );

	eg_uint HitColumn = EGMath_floor( HitPointGridSpace.x/m_Config.ItemWidth );
	eg_uint HitRow = EGMath_floor( HitPointGridSpace.y/m_Config.ItemHeight );

	AdjHitPoint.x -= HitColumn;
	AdjHitPoint.y -= HitRow;
	AdjHitPoint.y = 1.f - AdjHitPoint.y;

	//assert( EG_IsBetween(AdjHitPoint.x , 0.f , 1.f ) );
	//assert( EG_IsBetween(AdjHitPoint.y , 0.f , 1.f ) );

	//EGLogf( "Grid Hit Column: %u , Row: %u" , HitColumn , HitRow );

	eg_uint Index = RowColToIndex( HitColumn , HitRow );

	if( Index >= m_NumItems )
	{
		Index = INDEX_NONE;
		AdjHitPoint = eg_vec2(0,0);
	}

	if( AdjHitPointOut )
	{
		*AdjHitPointOut = AdjHitPoint;
	}

	return Index;
}

void EGUiGrid2::GetNormalizedTrackInfo( eg_real* OutPageSize , eg_real* OutTrackPos )const
{
	*OutPageSize = GetPageSize()/GetListSize();
	*OutTrackPos = m_ScrollPos/GetListSize();
}

eg_bool EGUiGrid2::HandleInputEvent( eg_uigrid_input_event Event )
{
	eg_int SavedSelectedIndex = m_SelectedIndex;

	if( 0 == m_NumItems )
	{
		SetSelectedIndex( INDEX_NONE );
		SetPosImmediateByPos( 0.f );
		return m_SelectedIndex != SavedSelectedIndex;
	}

	// Scrolling events are special:
	if(eg_uigrid_input_event::WheelUp == Event )
	{
		ScrollToPos( m_TargetPos - GetItemHeight() , false );
		return true;
	}
	if(eg_uigrid_input_event::WheelDown == Event )
	{
		ScrollToPos( m_TargetPos + GetItemHeight() , false );
		return true;
	}

	if( INDEX_NONE == m_SelectedIndex )
	{
		SetSelectedIndex(0);
		ScrollToIndex(0);
		return INDEX_NONE != m_SelectedIndex;
	}

	eg_int NumRows=0;
	eg_int NumCols=0;

	eg_bool WrapRows = false;
	eg_bool WrapCols = false;

	eg_uint MaxIndex = m_NumItems-1;

	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown:
			NumRows = MaxIndex/m_Config.VisibleCols + 1;
			NumCols = m_Config.VisibleCols;
			WrapCols = (m_Config.WrapType == eg_grid_wrap_t::WRAP);
			break;
		case eg_uigrid_scroll_dir::LeftRight:
			NumRows = m_Config.VisibleRows;
			NumCols = MaxIndex/m_Config.VisibleRows + 1;
			WrapRows = (m_Config.WrapType == eg_grid_wrap_t::WRAP);
			break;
	}

	eg_int Col=0;
	eg_int Row=0;
	IndexToRowCol( m_SelectedIndex , &Col , &Row );

	//Since we may have wrap around, if the row or column we are on is full
	//we want to use the actual number of items on that row or column for
	//the wrap.
	if( eg_uigrid_scroll_dir::LeftRight == m_Config.ScrollDir && Col == (NumCols-1) )
	{
		eg_uint FullMaxIndex = RowColToIndex( NumCols-1 , NumRows-1 );
		eg_int SizeDiff = (m_NumItems-1) - FullMaxIndex;
		NumRows = NumRows + SizeDiff;
	}

	if( eg_uigrid_scroll_dir::UpDown == m_Config.ScrollDir && Row == (NumRows-1) )
	{
		eg_uint FullMaxIndex = RowColToIndex( NumCols-1 , NumRows-1 );
		eg_int SizeDiff = (m_NumItems-1) - FullMaxIndex;
		NumCols = NumCols + SizeDiff;
	}

	switch( Event )
	{
		case eg_uigrid_input_event::Up: 
		{
			Row--;
			if( Row < 0 )
			{
				if( WrapRows )
				{
					Row = NumRows-1;
				}
				else
				{
					Row++;
				}
			}
		} break;
		case eg_uigrid_input_event::Down:
		{
			Row++;
			if( Row >= NumRows )
			{
				if( WrapRows )
				{
					Row = 0;
				}
				else
				{
					Row--;
				}
			}
		} break;
		case eg_uigrid_input_event::Left:
		{
			Col--;
			if( Col < 0 )
			{
				if( WrapCols )
				{
					Col = NumCols-1;
				}
				else
				{
					Col++;
				}
			}
		} break;
		case eg_uigrid_input_event::Right:
		{
			Col++;
			if( Col >= NumCols )
			{
				if( WrapCols )
				{
					Col = 0;
				}
				else
				{
					Col--;
				}
			}
		} break;
		case eg_uigrid_input_event::WheelUp:
		case eg_uigrid_input_event::WheelDown:
			assert( false ); //Handled differently.
			break;
	}

	eg_uint NewIndex = RowColToIndex( Col , Row );
	if( NewIndex >= m_NumItems )
	{
		NewIndex = m_NumItems - 1;
	}

	SetSelectedIndex( NewIndex );
	ScrollToIndex( NewIndex );

	return m_SelectedIndex != SavedSelectedIndex;
}

void EGUiGrid2::SetSelectedIndex( eg_uint Index )
{ 
	assert( (0 <= Index && Index < m_NumItems) || (INDEX_NONE == Index));
	eg_bool bChanged = m_SelectedIndex != Index; 
	m_SelectedIndex = Index; 
	if( bChanged && m_Config.Owner )
	{
		m_Config.Owner->UiGrid_OnSelectionChanged( this , m_SelectedIndex );
	}
}

void EGUiGrid2::Update( eg_real DeltaTime )
{
	//
	// Original Author for Smooth Scrolling was Brian Harris for 2K Sports
	//
	////////////////////////////////////////////////////////////////////////////////
	//!
	// A cubic bezier curve evaluator whose control points are a product of the
	// previous iterations outputs and a Target. The reason for this is to produce
	// a smooth animation when the Target changes in mid animation.
	//
	// The calculation has been simplified for execution speed. The four control
	// points for the cubic bezier curve are the current position, the current
	// position plus one third of the velocity, the target position minus one third
	// of the velocity and the target position.
	//
	// Setting the last two control points to a factor of the target position
	// achieves ease out. The ease in effect is produced by the velocity being zero
	// at the start of the animation and increasing as the animation progresses.
	//
	////////////////////////////////////////////////////////////////////////////////

	auto EvaluateCubic = []( eg_real* CurrentPos , eg_real* CurrentVelocity , eg_real TargetPos , eg_real Time , eg_real AnimationClampPositionThreshold , eg_real AnimationClampVelocityThreshold ) -> eg_bool
	{
		// Flag for if we have reached the target on this update call.
		eg_bool TargetReachedThisFrame = false;

		// Store the velocity and position from the previous iteration.
		eg_real v = *CurrentVelocity;
		eg_real p = *CurrentPos;

		// Calculate the coefficients of the bezier curve equation.
		eg_real d = TargetPos - p; // Delta from Target to Current Position
		eg_real a = - d - d; // -2*Delta
		eg_real b = d + d + d - v - v - v; // 3*Delta - 3*v

													  // Calculate the new position
													  // *CurrentPos = a*t^3 + b*t^2 + v*t + p;
		*CurrentPos = ( ( ( ( a * Time + b ) * Time ) + v ) * Time ) + p;

		// Calculate the new velocity
		// *CurrentVelocity = 3*a*t^2 + 2*b*t + v;
		*CurrentVelocity = ( ( ( ( a + a + a ) * Time ) + ( b + b ) ) * Time ) + v;

		// If we're within the threshold, terminate the animation and jump to the target.
		// This eliminates an oscillation due to the velocity driven bezier curve.
		if ( EG_Abs( *CurrentPos - TargetPos ) < AnimationClampPositionThreshold && EG_Abs( *CurrentVelocity ) < AnimationClampVelocityThreshold ) 
		{
			if ( *CurrentVelocity != 0 ) TargetReachedThisFrame = true;
			*CurrentVelocity = 0.0;
			*CurrentPos = TargetPos;
		}

		// Return whether or not the target was reached this frame.
		return TargetReachedThisFrame;
	};

	// Update the smooth scroller:
	auto UpdateScroller = [this,&EvaluateCubic]( eg_real DeltaTime ) -> eg_bool
	{
		const eg_real AnimationClampPositionThreshold = 0.01f;  // The threshold for when to stop the animation and clamp to the target position.
		const eg_real AnimationClampVelocityThreshold = 0.003f; // The threshold for when to stop the animation and stop the velocity.
		const eg_real AnimationSpeed = 9.0;  // The size of integration step to make at 1 Hz. At 60 Hz the step will be 0.15.
		const eg_real SimulationDeltaTime = 1.0f / 120.0f; // A very small time step (120 Hz) to allow the speed to be much higher.
		const eg_real MinimumFrameRate = 1.0f / 20.0f;     // The minimum frame rate for this function is 20 Hz.
		eg_real SimulationTime = 0.0f;                    // Store how simulated time has elapsed in this function.
		eg_bool AnimationEnded = false;                  // Store if the animation finished this frame.
																  // Add in the left over time from last frame so we make up any lost time.
		DeltaTime += m_LeftOverSimulationTime;

		// Protect against frame rate hitches slower than 20 Hz. If the frame rate
		// is slower than 20 Hz, process the update as though it were running at 20 Hz.
		if ( DeltaTime > MinimumFrameRate ) {
			DeltaTime = MinimumFrameRate;
	}

		// Call EvaluateCubic() multiple times with a small time step to improve
		// numerical accuracy and help with low frame rates being too unstable.
		while ( SimulationTime + SimulationDeltaTime <= DeltaTime ) 
		{
			// Calculate the new position and velocity.
			AnimationEnded = EvaluateCubic( &m_ScrollPos, &m_Velocity, m_TargetPos, AnimationSpeed * SimulationDeltaTime, AnimationClampPositionThreshold, AnimationClampVelocityThreshold );

			// Accumulate one simulation.
			SimulationTime += SimulationDeltaTime;

			// The animation has finished, we don't need to do any more evaluations and there isn't any need for left over time.
			if( m_Velocity == 0 )
			{
				SimulationTime = DeltaTime;
				break;
			}
		}

		// Save off how much is left over for next time.
		m_LeftOverSimulationTime = DeltaTime - SimulationTime;

		return AnimationEnded;
	};

	UpdateScroller( DeltaTime );

	ComputeHelpData();
}

void EGUiGrid2::ScrollToPos( eg_real Pos , eg_bool bJump )
{
	m_TargetPos = EG_Clamp( Pos , 0.f , GetMaxScrollPos() );
	if( bJump )
	{
		m_Velocity  = 0.f;
		m_ScrollPos = m_TargetPos;
	}
}

void EGUiGrid2::JumpToFinalScrollPos()
{
	m_Velocity = 0.f;
	m_ScrollPos = m_TargetPos;
}

void EGUiGrid2::ScrollToIndex( eg_uint Index )
{
	eg_real ItemTop = (Index/m_HelpData.ItemsPerDir)*m_HelpData.DirDim;
	eg_real ItemBottom = ItemTop + m_HelpData.DirDim;

	if( ItemTop < m_ScrollPos )
	{
		//If the top of the item is above the scroll position, scroll up to
		//so that the top of the item is visible.
		ScrollToPos( ItemTop , false );
	}
	else if( ItemBottom > (m_ScrollPos+GetPageSize()) )
	{
		//If the bottom of the item is past the end of the page, scroll down
		//so that the bottom of the item is visible.
		ScrollToPos( ItemBottom - GetPageSize() , false );
	}
}

void EGUiGrid2::Resize( eg_uint NewNumItems )
{
	eg_uint OldNumVisibileItems = GetNumVisibleItems();
	m_NumItems = NewNumItems;

	if( INDEX_NONE != m_SelectedIndex && m_SelectedIndex >= NewNumItems )
	{
		SetSelectedIndex( NewNumItems-1 );
		#if defined( __DEBUG__ )
		GetSelectedIndex(); //FOR DEBUGGING: This will assert if the index somehow got out of range.
		#endif
	}

	const eg_real MaxScrollPos = GetMaxScrollPos();

	if( m_ScrollPos > MaxScrollPos )
	{
		m_ScrollPos = MaxScrollPos;
	}

	if( m_TargetPos > MaxScrollPos )
	{
		m_TargetPos = MaxScrollPos;
	}

	ComputeHelpData();

	eg_bool bNumVisChanged = OldNumVisibileItems != GetNumVisibleItems();
	
	if( m_Config.Owner )//&& bNumVisChanged && m_CbOnTopRowChanged )
	{
		m_Config.Owner->UiGrid_OnUpdateCells( this , m_HelpData.FirstItemIndex , GetNumVisibleItems() );
	}
}

eg_uint EGUiGrid2::GetNumVisibleItems()const
{
	return m_HelpData.NumVisibleItems;
}

eg_uint EGUiGrid2::GetMaxVisibleItems()const
{
	return m_HelpData.ItemsInDir*m_HelpData.ItemsPerDir;
}

egUiGrid2ItemInfo EGUiGrid2::GetVisibleItemInfo( eg_uint VisibleIndex )const
{
	egUiGrid2ItemInfo InfoOut;
	InfoOut.Index = m_HelpData.FirstItemIndex + VisibleIndex;

	if( eg_uigrid_scroll_dir::UpDown == m_Config.ScrollDir )
	{
		InfoOut.Offset.x = (VisibleIndex%m_HelpData.ItemsPerDir)*m_Config.ItemWidth;
		InfoOut.Offset.y = (m_HelpData.ScrollOffset - (VisibleIndex/m_HelpData.ItemsPerDir)*m_HelpData.DirDim);
	}
	else if( eg_uigrid_scroll_dir::LeftRight == m_Config.ScrollDir )
	{
		InfoOut.Offset.y = -static_cast<eg_real>((VisibleIndex%m_HelpData.ItemsPerDir)*m_Config.ItemHeight);
		InfoOut.Offset.x = -static_cast<eg_real>((m_HelpData.ScrollOffset - (VisibleIndex/m_HelpData.ItemsPerDir)*m_HelpData.DirDim));
	}

	return InfoOut;
}

eg_int EGUiGrid2::GetVisibilityIndex( eg_uint ItemIndex ) const
{
	return ItemIndex - m_HelpData.FirstItemIndex;
}

void EGUiGrid2::SetPosImmediateByIndex( eg_uint Index )
{
	ScrollToIndex( Index );
	m_Velocity  = 0.f;
	m_ScrollPos = m_TargetPos;
	ComputeHelpData();
}

void EGUiGrid2::SetPosImmediateByPos( eg_real Pos )
{
	ScrollToPos( Pos , true );
	m_Velocity  = 0.f;
	m_ScrollPos = m_TargetPos;
	ComputeHelpData();
}

eg_real EGUiGrid2::GetPageSize()const
{
	eg_real Out = 0.f;
	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown   : Out = m_Config.VisibleRows * m_Config.ItemHeight; break;
		case eg_uigrid_scroll_dir::LeftRight: Out = m_Config.VisibleCols * m_Config.ItemWidth; break;
	}
	return Out;
}

eg_real EGUiGrid2::GetListSize()const
{
	eg_uint NumDirItems = 0;
	eg_real DirDim = 0.f;
	eg_uint MaxIndex = m_NumItems > 0 ? m_NumItems-1 : 0;

	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown   : NumDirItems = MaxIndex / m_Config.VisibleCols + 1; DirDim = m_Config.ItemHeight; break;
		case eg_uigrid_scroll_dir::LeftRight: NumDirItems = MaxIndex / m_Config.VisibleRows + 1; DirDim = m_Config.ItemWidth; break;
	}

	return DirDim*NumDirItems;
}

eg_real EGUiGrid2::GetMaxScrollPos()const
{
	eg_real MaxPos = GetListSize() - GetPageSize();
	// If this list doesn't fill up a full page then the page size will be
	// bigger than the list size, so we clamp to 0.
	if( MaxPos < 0.f )
	{
		MaxPos = 0.f;
	}
	return MaxPos;
}

void EGUiGrid2::ComputeHelpData()
{
	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown   : m_HelpData.DirDim = m_Config.ItemHeight; m_HelpData.ItemsInDir = m_Config.VisibleRows+1; m_HelpData.ItemsPerDir = m_Config.VisibleCols; break;
		case eg_uigrid_scroll_dir::LeftRight: m_HelpData.DirDim = m_Config.ItemWidth;  m_HelpData.ItemsInDir = m_Config.VisibleCols+1; m_HelpData.ItemsPerDir = m_Config.VisibleRows; break;
	}

	eg_uint OldFirstItemIndex = m_HelpData.FirstItemIndex;

	eg_uint FirstDirIndex = static_cast<eg_uint>(EGMath_floor( m_ScrollPos / m_HelpData.DirDim ));
	m_HelpData.FirstItemIndex = FirstDirIndex * m_HelpData.ItemsPerDir;

	m_HelpData.NumVisibleItems = EG_Clamp<eg_uint>( m_HelpData.ItemsInDir*m_HelpData.ItemsPerDir , 0 , m_NumItems - m_HelpData.FirstItemIndex );

	m_HelpData.ScrollOffset = m_ScrollPos - m_HelpData.DirDim*FirstDirIndex;

	eg_bool bTopChanged = OldFirstItemIndex != m_HelpData.FirstItemIndex;
	if( bTopChanged && m_Config.Owner )
	{
		m_Config.Owner->UiGrid_OnUpdateCells( this , m_HelpData.FirstItemIndex , GetNumVisibleItems() );
	}
}

void EGUiGrid2::IndexToRowCol( eg_uint Index , eg_int* ColOut , eg_int* RowOut )const
{
	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown:
		{
			*ColOut = Index%m_Config.VisibleCols;
			*RowOut = Index/m_Config.VisibleCols;
		} break;
		case eg_uigrid_scroll_dir::LeftRight:
		{
			*ColOut = Index/m_Config.VisibleRows;
			*RowOut = Index%m_Config.VisibleRows;
		} break;
	}
}

eg_uint EGUiGrid2::RowColToIndex( eg_int Col , eg_int Row )const
{
	eg_uint Index = 0;
	switch( m_Config.ScrollDir )
	{
		case eg_uigrid_scroll_dir::UpDown:
		{
			Index = Row*m_Config.VisibleCols + Col;
		} break;
		case eg_uigrid_scroll_dir::LeftRight:
		{
			Index = Col*m_Config.VisibleRows + Row;
		} break;
	}

	return Index;
}