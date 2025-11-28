//(c) 2015 Beem Software

#include "EGOverlayMgr.h"
#include "EGMenu.h"
#include "EGRenderer.h"
#include "EGUiLayout.h"
#include "EGInputTypes.h"

EGMenu* EGOverlayMgr::AddOverlay( eg_string_crc MenuId, eg_real DrawPriority )
{
	if( !EGMenu::IsIdValidMenu( MenuId ) )
	{
		EGLogf( eg_log_t::Error , "An overlay was added, but didn't have a layout." );
		assert( false ); // No proc for this overlay.
		return nullptr;
	}

	egOverlayItem* NewOverlayItem = nullptr;
	
	NewOverlayItem = new ( eg_mem_pool::System ) egOverlayItem;

	if( NewOverlayItem )
	{
		const EGUiLayout* Layout = EGUiLayoutMgr::Get()->GetLayout( MenuId );
		EGClass* MenuClass = Layout->m_ClassName.Class;
		NewOverlayItem->MenuId = MenuId;
		NewOverlayItem->DrawPriority = DrawPriority;

		NewOverlayItem->Overlay = EGNewObject<EGMenu>( MenuClass , eg_mem_pool::DefaultHi );
		NewOverlayItem->Overlay->Init( MenuId , nullptr , this , m_AiClient );

		if( nullptr == NewOverlayItem->Overlay )
		{
			delete NewOverlayItem;
			assert( false ); //Out of menu memory?
			return nullptr;
		}

		//To figure out where to insert this overlay, just iterate through all the overlays until we find
		//an overlay with a less draw priority and insert it before that (and if there are none, just insert it)
		egOverlayItem* InsertBeforeItem = nullptr;
		for( egOverlayItem* CurItem : m_OverlayList )
		{
			if( CurItem->DrawPriority > NewOverlayItem->DrawPriority )
			{
				InsertBeforeItem = CurItem;
				break;
			}
		}

		if( InsertBeforeItem )
		{
			m_OverlayList.InsertBefore( InsertBeforeItem , NewOverlayItem );
		}
		else
		{
			m_OverlayList.InsertLast( NewOverlayItem );
		}

		NewOverlayItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::INIT );
		NewOverlayItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::ACTIVATE );
	}

	return NewOverlayItem->Overlay;
}

void EGOverlayMgr::RemoveOverlay( EGMenu* Overlay )
{
	egOverlayItem* FoundItem = nullptr;

	for( egOverlayItem* CurItem : m_OverlayList )
	{
		if( CurItem->Overlay == Overlay )
		{
			FoundItem = CurItem;
			break;
		}
	}

	if( FoundItem )
	{
		FoundItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		FoundItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
		m_OverlayList.Remove( FoundItem );
		EGDeleteObject( FoundItem->Overlay );
		EG_SafeDelete( FoundItem );
	}
	else
	{
		assert( false );
	}
}

void EGOverlayMgr::Clear()
{
	while( m_OverlayList.HasItems() )
	{
		egOverlayItem* Item = m_OverlayList.GetFirst();
		Item->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		Item->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
		m_OverlayList.Remove( Item );
		EGDeleteObject( Item->Overlay );
		EG_SafeDelete( Item );
	}
}

eg_bool EGOverlayMgr::OwnsOverlay( EGMenu* Overlay ) const
{
	for( const egOverlayItem* CurItem : m_OverlayList )
	{
		if( CurItem->Overlay == Overlay )
		{
			return true;
		}
	}

	return false;
}

void EGOverlayMgr::Update( eg_real DeltaTime , const struct egLockstepCmds* Input , eg_real AspectRatio )
{
	m_LastKnownAspectRatio = AspectRatio;

	for( egOverlayItem* CurItem : m_OverlayList )
	{
		CurItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::UPDATE , DeltaTime , AspectRatio );
		if( Input )
		{
			Update_HandleMouse( CurItem->Overlay , Input );
		}
	}
}

void EGOverlayMgr::Draw()
{
	MainDisplayList->SetMaterial( EGV_MATERIAL_NULL );
	MainDisplayList->ClearDS( 1.f , 0 );

	for( egOverlayItem* CurItem : m_OverlayList )
	{
		CurItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::PRE_DRAW , m_LastKnownAspectRatio );
		CurItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::DRAW , m_LastKnownAspectRatio );
		CurItem->Overlay->ProcessEvent( EGMenu::eg_menuevent_t::POST_DRAW , m_LastKnownAspectRatio );
	}
}

void EGOverlayMgr::Update_HandleMouse( EGMenu* Menu , const struct egLockstepCmds* Input )
{
	//As long as we are capturing input we find the window the mouse is
	//pointing at.
	const eg_real xPos = Input->m_MousePos.x;
	const eg_real yPos = Input->m_MousePos.y;

	const eg_bool bMouseMoved = m_bHasInitialMousePos && (m_v2LastMousePos.x != xPos || m_v2LastMousePos.y != yPos);

	m_v2LastMousePos.x = xPos;
	m_v2LastMousePos.y = yPos;

	m_bHasInitialMousePos = true;

	static const eg_cmd_t MouseMoveCmds[] =
	{
		CMDA_MENU_RMB,
		CMDA_MENU_LMB,
	};

	bool MouseMoveCmdActive = false;
	for( eg_uint i = 0; i < countof(MouseMoveCmds); i++ )
	{
		MouseMoveCmdActive = MouseMoveCmdActive || Input->WasPressed(MouseMoveCmds[i]) || Input->WasReleased(MouseMoveCmds[i]);
	}
	//If the mouse was moved, or if a mouse related command was pressed, send the mouse moved event.
	if( bMouseMoved || MouseMoveCmdActive )
	{
		// Menu->HandleMouseEvent( eg_menumouse_e::MOVE , xPos , yPos );
	}

	// Menu->HandleMouseEvent( eg_menumouse_e::POSITION , xPos , yPos );

	if( Input->WasPressed(CMDA_MENU_LMB) )
	{
		Menu->HandleMouseEvent( eg_menumouse_e::PRESSED , xPos , yPos );
	}

	if( Input->WasReleased(CMDA_MENU_LMB) )
	{
		Menu->HandleMouseEvent( eg_menumouse_e::RELEASED , xPos , yPos );
	}
}
