// (c) 2017 Beem Media

#include "EGWndPanelPropEditor.h"
#include "EGWndPropControl.h"
#include "EGResourceLib.h"


EGWndPanelPropEditor::EGWndPanelPropEditor( EGWndPanel* Parent )
: Super( Parent , 1 )
{
	UpdateScroll();
	SetWheelScrollAmount( 25 );

	m_RelationPen = CreatePen( PS_SOLID , 2 , RGB(160,10,8) );
}

EGWndPanelPropEditor::~EGWndPanelPropEditor()
{
	ClearControls();

	if( m_RelationPen )
	{
		DeleteObject( m_RelationPen );
		m_RelationPen = nullptr;
	}
}

void EGWndPanelPropEditor::OnPropChangedValue( const egRflEditor* Editor )
{
	unused( Editor );

	HandlePropChanged( Editor );
}

void EGWndPanelPropEditor::OnPropControlButtonPressed( const egRflEditor* Property )
{
	OnButtonPressed( Property );
}

void EGWndPanelPropEditor::PropEditorRebuild()
{
	if( !m_bRebuildQueued )
	{
		m_bRebuildQueued = true;
		PostMessageW( GetWnd() , WM_EG_PROP_CONTROL_REFRESH , 0 , 0 );
	}
}

void EGWndPanelPropEditor::SetEditObject( egRflEditor* RflEditor )
{
	if( m_CurEditor == RflEditor )
	{
		return;
	}

	ClearControls();
	m_CurProps.Clear();

	m_CurEditor = RflEditor;

	std::function<void(egRflEditor*,egRflEditor*,eg_int)> GetPropertiesRecursive = [this,&GetPropertiesRecursive]( egRflEditor* Ed , egRflEditor* Parent , eg_int Depth ) -> void
	{
		if( Ed && Ed->IsValid() )
		{
			egEditProperty NewEditProp( Ed , Parent , Depth );
			m_CurProps.Append( NewEditProp );
			if( Ed->IsExpanded() )
			{
				EGArray<egRflEditor*> Children;
				Ed->GetChildrenPtrs( Children );
				for( egRflEditor* Child : Children )
				{
					if( Child && Child->IsValid() && Child->IsEditable() )
					{
						GetPropertiesRecursive( Child , Ed  , Depth + 1 );
					}
				}
			}
		}
	};

	GetPropertiesRecursive( m_CurEditor , nullptr , 0 );

	for( egEditProperty& Item : m_CurProps )
	{
		egProp NewProp;
		NewProp.Info = Item;
		NewProp.Depth = Item.Depth;
		NewProp.Control = nullptr;
		m_Props.Append( NewProp );
	}

	m_TotalHeight = 0;
	for( egProp& Prop : m_Props )
	{
		Prop.Control = CreatePropControl( Prop.Info );
		Prop.Offset = m_TotalHeight;
		if( Prop.Control )
		{
			m_TotalHeight += Prop.Control->GetHeight();
		}
		m_TotalHeight += CTL_Y_PADDING;
	}

	RefreshControlSizes( GetViewRect().right );

	SetScroll( 0 );
	UpdateScroll();

	if( m_Props.IsEmpty() )
	{
		EGWnd_ClearFocus();
	}
}

void EGWndPanelPropEditor::RefreshAll()
{
	for( egProp& Prop : m_Props )
	{
		if( Prop.Control )
		{
			Prop.Control->RefreshFromSourceData();
		}
	}
}

void EGWndPanelPropEditor::OnPaint( HDC hdc )
{
	SetBkMode( hdc , TRANSPARENT );
	SetBkColor( hdc , EGWnd_GetColor( egw_color_t::BG_STATIC ) );
	SetTextColor( hdc , EGWnd_GetColor( egw_color_t::FG_STATIC ) );

	if( m_Props.IsEmpty() )
	{
		eg_cpstr Message = m_NoPropsMessage;
		RECT rc = GetViewRect();
		rc.top -= m_ScrollPos;
		rc.bottom -= m_ScrollPos;
		DrawTextA( hdc , Message , -1 , &rc , DT_WORDBREAK );
		return;
	}

	DrawRelations( hdc );
}

void EGWndPanelPropEditor::OnDrawBg( HDC hdc )
{
	FillRect( hdc , &GetViewRect() , EGWnd_GetBrush( egw_color_t::BG_STATIC ) );
}

void EGWndPanelPropEditor::OnScrollChanged( eg_int NewScrollPos , eg_int OldScrollPos )
{
	unused( OldScrollPos );

	eg_int Offset = -NewScrollPos;
	for( egProp& Prop : m_Props )
	{
		if( Prop.Control )
		{
			eg_ivec2 NewPos( CTL_X_PADDING + Prop.Depth*DEPTH_PADDING , Offset );
			Prop.Control->SetWndPos( NewPos.x , NewPos.y , Prop.Control->GetEditorWidth() , Prop.Control->GetHeight() );
			Offset += Prop.Control->GetHeight();
			Offset += CTL_Y_PADDING;
		}
	}
}

eg_size_t EGWndPanelPropEditor::GetNumItems() const
{
	return m_TotalHeight;
}

LRESULT EGWndPanelPropEditor::WndProc( UINT Msg , WPARAM wParam , LPARAM lParam )
{
	switch( Msg )
	{
		case WM_EG_REFRESH_OFFSETS:
		{
			OnScrollChanged( m_ScrollPos , m_ScrollPos );
		} return 0;
		case WM_EG_PROP_CONTROL_REFRESH:
		{
			m_bRebuildQueued = false;
			RebuildControls();
		} return 0;
	}

	return Super::WndProc( Msg , wParam , lParam );
}

void EGWndPanelPropEditor::OnWmSize( const eg_ivec2& NewClientSize )
{
	RefreshControlSizes( NewClientSize.x );
	OnScrollChanged( m_ScrollPos , m_ScrollPos );

	// We got to call the Super last because OnUpdateScroll is called which may 
	// resize this very window if the scrollbar changes visibility.
	Super::OnWmSize( NewClientSize );
}

void EGWndPanelPropEditor::DrawRelations( HDC hdc )
{
	SetPen( hdc , m_RelationPen );

	const eg_int HORZ_OFFSET = DEPTH_PADDING/2;

	auto DrawLine = [&hdc,this]( const eg_ivec2& Start , const eg_ivec2& End ) -> void
	{
		MoveToEx( hdc , Start.x , Start.y - m_ScrollPos , nullptr );
		LineTo( hdc , End.x , End.y - m_ScrollPos );
	};

	m_DepthRelations.Clear( false );
	m_DepthRelations.Push(eg_ivec2(0,0));
	eg_int LastDepth = -1;
	eg_ivec2 LastPropLoc(0,0);

	for( const egProp& Prop : m_Props )
	{			
		eg_ivec2 PropLoc( Prop.Depth*DEPTH_PADDING , Prop.Offset );

		if( Prop.Depth > LastDepth )
		{
			const eg_int TimesToPush = Prop.Depth - LastDepth;

			for( eg_int i=0; i<TimesToPush; i++ )
			{
				m_DepthRelations.Push( LastPropLoc );	
			}	
		}
		else if( Prop.Depth < LastDepth )
		{
			const eg_int TimesToPop = LastDepth - Prop.Depth;
			for( eg_int i=0; i<TimesToPop; i++ )
			{
				assert( m_DepthRelations.HasItems() );
				if( m_DepthRelations.HasItems() )
				{
					m_DepthRelations.Pop();
				}
			}
		}

		LastDepth = Prop.Depth;
		LastPropLoc = PropLoc;

		eg_ivec2 ParentLoc = m_DepthRelations.Top();

		if( Prop.Depth > 0 )
		{
			const eg_int ConnectLineY = Prop.Offset + 9;
			eg_ivec2 SelfCon(PropLoc.x,ConnectLineY);
			eg_ivec2 ParentCon( ParentLoc.x + DEPTH_PADDING/2 , ParentLoc.y );
			eg_ivec2 JointCon( ParentCon.x , SelfCon.y );

			DrawLine( JointCon , SelfCon );
			DrawLine( JointCon , ParentCon );
		}
	}
}

eg_string EGWndPanelPropEditor::GetNoPropsMessage( eg_bool bNoObject, eg_bool bLocked )
{
	if( bNoObject )
	{
		return "Nothing selected.";
	}

	if( bLocked )
	{
		return "This object is locked. Unlock it to edit it's properties.";
	}

	return "No editable properties.";
}

void EGWndPanelPropEditor::ClearControls()
{
	for( egProp& Prop : m_Props )
	{
		EG_SafeDelete( Prop.Control );
	}

	m_Props.Clear();
}

void EGWndPanelPropEditor::RebuildControls()
{
	eg_int SavedScrollPos = m_ScrollPos;
	egRflEditor* SavedEditor = m_CurEditor;

	ClearControls();
	m_CurProps.Clear();
	m_CurEditor = nullptr;

	if( SavedEditor )
	{
		SetEditObject( SavedEditor );
	}
	SetScroll( SavedScrollPos );
	FullRedraw();
}

void EGWndPanelPropEditor::RefreshControlSizes( eg_int ClientWidth )
{
	for( egProp& Prop : m_Props )
	{
		if( Prop.Control )
		{
			eg_int ControlWidth = ClientWidth;
			ControlWidth -= Prop.Depth* DEPTH_PADDING;
			ControlWidth -= CTL_X_PADDING*2;
			Prop.Control->SetEditorWidth( ControlWidth );
		}
	}
}

EGWndPropControl* EGWndPanelPropEditor::CreatePropControl( const egEditProperty& Info )
{
	egPropControlInfo ControlInfo;
	ControlInfo.DisplayName = Info.DisplayName;
	ControlInfo.Editor = Info.PropEditor;
	ControlInfo.Parent = Info.ParentEditor;

	eg_edit_ctrl_t ControlType = eg_edit_ctrl_t::Unk;

	if( Info.PropEditor )
	{
		ControlType = Info.PropEditor->GetEditControlType();
	}

	return EGEditProperty_CreateControlForType( this , ControlType , ControlInfo );
}

void EGWndPanelPropEditor::HandlePropChanged( const egRflEditor* Editor )
{
	unused( Editor );
}
