// (c) 2016 Beem Media

#include "EGWndPanel.h"

eg_int EGWndPanel::GetPane3SideWidth()
{
	return EGWnd_GetAppScaledSize( PANE3_SIDE_WIDTH );
}

eg_int EGWndPanel::GetPane3MidMinWidth()
{
	return EGWnd_GetAppScaledSize( PANE3_MID_MIN_WIDTH );
}

eg_int EGWndPanel::GetMainWndMinHeight()
{
	return EGWnd_GetAppScaledSize( MAIN_WND_MIN_HEIGHT );
}

eg_int EGWndPanel::GetPreviewPadding()
{
	return EGWnd_GetAppScaledSize( PREVIEW_PADDING );
}

eg_int EGWndPanel::GetHeaderSize()
{
	return EGWnd_GetAppScaledSize( HEADER_SIZE );
}

eg_int EGWndPanel::GetTabSize()
{
	return EGWnd_GetAppScaledSize( TAB_SIZE );
}

EGWndPanel::EGWndPanel( egWndPanelAppContainer& Parent )
: Super( Parent.BaseWindowParent )
{
	HandleResize();
	FullRedraw();
}

EGWndPanel::EGWndPanel( EGWndPanel* Parent , egWndPanelCustomLayout CustomLayout )
: Super( Parent )
, m_bUseCustomWindowRect( CustomLayout.bUseCustomDraw )
, m_OwnerPanel( Parent )
{
	unused( CustomLayout );
	HandleResize();
	FullRedraw();
}

EGWndPanel::EGWndPanel( EGWndPanel* Parent, eg_panel_fill FillMode )
: EGWndPanel( Parent , FillMode , eg_panel_size_t::Auto , 0 )
{
	
}

EGWndPanel::EGWndPanel( EGWndPanel* Parent, eg_panel_size_t SizeType, eg_int Size )
: EGWndPanel( Parent , eg_panel_fill::None , SizeType , Size )
{

}

EGWndPanel::EGWndPanel( EGWndPanel* Parent, eg_panel_fill FillMode, eg_panel_size_t SizeType, eg_int Size )
: Super( Parent )
, m_FillMode( FillMode )
, m_FillSizeType( SizeType )
, m_FillSize( Size )
, m_OwnerPanel( Parent )
{

}

EGWndPanel::~EGWndPanel()
{
	for( eg_size_t i=0; i<m_Children.Len(); i++ )
	{
		EG_SafeDelete( m_Children[i] );
	}
	DestroyWindow( m_hwnd );
	m_hwnd = nullptr;
}

void EGWndPanel::SetFillSize( eg_panel_size_t InType, eg_int InSize )
{
	m_FillSizeType = InType;
	m_FillSize = InSize;
	if( m_OwnerPanel )
	{
		m_OwnerPanel->HandleResize();
	}
	else
	{
		HandleResize();
	}
}

void EGWndPanel::SetFillMode( eg_panel_fill NewFillMode )
{
	m_FillMode = NewFillMode;
	HandleResize();
}

RECT EGWndPanel::GetParentClientRect() const
{
	if( m_OwnerPanel )
	{
		return m_OwnerPanel->GetViewRect();
	}

	RECT rcParent;
	SetRect( &rcParent , 0 , 0 , 320 , 200 );
	HWND Parent = GetParent( m_hwnd );
	if( Parent )
	{
		GetClientRect( Parent, &rcParent );
	}
	return rcParent;
};

void EGWndPanel::HandleResize()
{
	const DWORD POS_FLAGS = SWP_NOACTIVATE|SWP_NOZORDER;


	if( m_bUseCustomWindowRect )
	{
		RECT rcFill = GetCustomWindowRect();
		SetWindowPos( m_hwnd , nullptr , rcFill.left , rcFill.top , rcFill.right - rcFill.left , rcFill.bottom - rcFill.top , POS_FLAGS );
	}
	else
	{
		if( m_OwnerPanel )
		{
			RECT rcFill = m_OwnerPanel->GetChildFill( this );
			SetWindowPos( m_hwnd , nullptr , rcFill.left , rcFill.top , rcFill.right - rcFill.left , rcFill.bottom - rcFill.top , POS_FLAGS );
		}
		else
		{
			RECT rcFill = GetParentClientRect();
			SetWindowPos( m_hwnd , nullptr , rcFill.left , rcFill.top , rcFill.right - rcFill.left , rcFill.bottom - rcFill.top , POS_FLAGS );
		}
	}

	ReevaluateChildSizes();
	
	for( eg_size_t i=0; i<m_Children.Len(); i++ )
	{
		m_Children[i]->HandleResize();
	}
}

RECT EGWndPanel::GetChildFill( const EGWndPanel* ChildPanel ) const
{
	auto BadCall = []() -> RECT
	{
		assert( false ); // Why was this asked?
		RECT Out;
		SetRect( &Out , 0 , 0 , 320 , 200 );
		return Out;
	};
	
	if( nullptr == ChildPanel || m_Children.Len() == 0 || ChildPanel->GetParentPanel() != this )
	{
		return BadCall();
	}

	const RECT ViewRect = GetViewRect();

	if( m_FillMode == eg_panel_fill::None )
	{
		return ViewRect;
	}
	else if( m_FillMode == eg_panel_fill::Tabs )
	{
		RECT TabRect = ViewRect;
		TabRect.top = EGWnd_GetAppScaledSize( TAB_SIZE );
		return TabRect;
	}
	else if( m_FillMode == eg_panel_fill::Horizontal || m_FillMode == eg_panel_fill::Vertical )
	{		
		// Compute final size and offset
		eg_int Offset = 0;
		eg_int Size = EGWnd_GetAppScaledSize( 320 );

		for( EGWndPanel* Child : m_Children )
		{
			if( Child->IsVisible() )
			{
				if( Child == ChildPanel )
				{
					Size = Child->GetParentOwnedSize();
					break;
				}
				else
				{
					Offset += Child->GetParentOwnedSize();
				}
			}
		}

		RECT Out = ViewRect;
		if( m_FillMode == eg_panel_fill::Horizontal )
		{
			SetRect( &Out , Offset , 0 , Offset + Size , ViewRect.bottom );
		}
		else if( m_FillMode == eg_panel_fill::Vertical )
		{
			SetRect( &Out , 0 , Offset , ViewRect.right , Offset + Size );
		}

		return Out;
	}

	return BadCall();
}

eg_bool EGWndPanel::IsMouseOverPanel( POINT* ClientPosOut ) const
{
	eg_bool bMouseIsOver = false;

	POINT CursorPos;
	zero( &CursorPos );
	if( GetCursorPos( &CursorPos ) )
	{
		if( ScreenToClient( m_hwnd , &CursorPos ) )
		{
			RECT rcClient = GetViewRect();
			bMouseIsOver = PtInRect( &rcClient , CursorPos ) == TRUE;
			if( ClientPosOut )
			{
				*ClientPosOut = CursorPos;
			}
		}
	}

	return bMouseIsOver;
}

void EGWndPanel::SetVisible( eg_bool bVisible )
{
	Super::SetVisible( bVisible );

	if( m_OwnerPanel )
	{
		m_OwnerPanel->HandleResize();
	}
}

void EGWndPanel::ReevaluateChildSizes()
{
	if( m_Children.Len() == 0 )
	{
		return;
	}

	const RECT ViewRect = GetViewRect();

	if( m_FillMode == eg_panel_fill::None )
	{
		return;
	}
	else if( m_FillMode == eg_panel_fill::Tabs )
	{
		return;
	}
	else if( m_FillMode == eg_panel_fill::Horizontal || m_FillMode == eg_panel_fill::Vertical )
	{
		const eg_int FullDim = EG_Max<eg_int>( 0 , m_FillMode == eg_panel_fill::Horizontal ? ViewRect.right : ViewRect.bottom );
		// Compute size of fixed size panels:
		eg_int DimTaken = 0;
		EGArray<EGWndPanel*> AutoSizePanels;
		for( EGWndPanel* Child : m_Children )
		{
			if( Child->IsVisible() )
			{
				switch( Child->GetFillSizeType() )
				{
					case eg_panel_size_t::Fixed:
					{
						eg_int SizeFromFixed = EG_Max<eg_int>( 0 , Child->GetFillSize() );
						Child->SetParentOwnedSize( SizeFromFixed );
						DimTaken += SizeFromFixed;
					} break;
					case eg_panel_size_t::Auto:
					{
						AutoSizePanels.Append( Child );
					} break;
					case eg_panel_size_t::Percent:
					{
						eg_int SizeFromPct = EG_Max<eg_int>( 0 , static_cast<eg_int>( .01f * static_cast<eg_real>(Child->GetFillSize() * FullDim ) ) );
						Child->SetParentOwnedSize( SizeFromPct );
						DimTaken += SizeFromPct;
					} break;
				}
			}
		}

		if( AutoSizePanels.Len() > 0 )
		{
			const eg_int AutoSizeDim = EG_Max<eg_int>( 0 , FullDim - DimTaken );
			const eg_int NumChildren = AutoSizePanels.LenAs<eg_int>();
			const eg_int ChildSize = AutoSizeDim / NumChildren;
			const eg_int ChildRemainder = AutoSizeDim % NumChildren;

			for( EGWndPanel* Child : AutoSizePanels )
			{
				Child->SetParentOwnedSize( ChildSize );
			}

			// Add the extra space...
			for( eg_int i=0; i<ChildRemainder; i++ )
			{
				EGWndPanel* Child = AutoSizePanels[i%NumChildren];
				Child->SetParentOwnedSize( Child->GetParentOwnedSize() + 1 );
			}
		}
	}

	return;
}

RECT EGWndPanel::GetCustomWindowRect() const
{
	assert( m_bUseCustomWindowRect );
	return GetParentClientRect();
}
