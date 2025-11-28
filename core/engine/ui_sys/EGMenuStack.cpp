// (c) 2014 Beem Media

#include "EGMenuStack.h"
#include "EGMenu.h"
#include "EGRenderer.h"
#include "EGParse.h"
#include "EGEngineConfig.h"
#include "EGEntDict.h"
#include "EGEnt.h"
#include "EGCamera2.h"
#include "EGInputTypes.h"

EGMenuStack::EGMenuStack( EGClient* Owner )
: m_Stack()
, m_HasInput( false )
, m_v2LastMousePos(0,0)
, m_StackChanged( false )
, m_MouseObj()
, m_LastKnownAspectRatio(0.f)
, m_bHasInitialMousePos(false)
, m_Owner( Owner )
{
	SetMouseCursor( eg_crc("MouseCursor") );
}

EGMenuStack::~EGMenuStack()
{
	Clear();
	CullDeletedMenus();
	m_MouseObj.Deinit();
}

void EGMenuStack::AcquireInput( void )
{
	m_HasInput = true;
}

void EGMenuStack::UnacquireInput( void )
{
	m_HasInput = false;
}

EGMenu* EGMenuStack::Push( eg_string_crc MenuId )
{
	if( !EGMenu::IsIdValidMenu( MenuId ) )
	{
		EGLogf( eg_log_t::Error , "Attempted to push an invalid menu (%08X)." , MenuId.ToUint32() );
		assert( false );
		return nullptr;
	}

	EGMenu* NewMenu = nullptr;

	//If there was a menu on the stack, deactivate it.
	if( m_Stack.HasItems() )
	{
		UnacquireInput();
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		UnacquireInput();
	}

	NewMenu = InternalInitMenu( MenuId );

	m_bHasInitialMousePos = false;
	m_StackChanged = true;

	m_HasSeenDown.Clear();

	MenuStackChangedDelegate.Broadcast( this );

	return NewMenu;
}

void EGMenuStack::Pop( void )
{
	if( m_Stack.HasItems() )
	{
		//Pop the menu.
		UnacquireInput();
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
		m_StackDelete.Push( m_Stack.Top() );
		m_Stack.Pop();
		//If there is still a menu on the stack, activate it.
		if( m_Stack.HasItems() )
		{
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::ACTIVATE );
			if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.
		}
	}
	else
	{
		assert( false ); //Popped too many menus, bad flow.
	}
	m_bHasInitialMousePos = false;
	m_StackChanged = true;

	m_HasSeenDown.Clear();

	MenuStackChangedDelegate.Broadcast( this );
}

EGMenu* EGMenuStack::SwitchTo( eg_string_crc MenuId )
{
	if( !EGMenu::IsIdValidMenu( MenuId ) )
	{
		EGLogf( eg_log_t::Error , "Attempted to switch to an invalid menu (%08X)." , MenuId.ToUint32() );
		assert( false );
		return nullptr;
	}

	EGMenu* NewMenu = nullptr;

	if( m_Stack.HasItems() )
	{
		//We never activate the menu underneath this one.
		UnacquireInput();
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
		m_StackDelete.Push( m_Stack.Top() );
		m_Stack.Pop();

		NewMenu = InternalInitMenu( MenuId );
	}
	else
	{
		assert( false ); //Can't switch to a menu if there is no menu on the stack.
		//Push it
		this->Push( MenuId );
	}
	m_bHasInitialMousePos = false;
	m_StackChanged = true;
	m_HasSeenDown.Clear();

	MenuStackChangedDelegate.Broadcast( this );

	return NewMenu;
}

void EGMenuStack::Clear( void )
{
	// We deactivate the top menu, but everything else gets deinit only
	// no activates or deactivates.
	if( m_Stack.HasItems() )
	{
		UnacquireInput();
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
	}

	while( m_Stack.HasItems() )
	{
		UnacquireInput();
		m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
		m_StackDelete.Push( m_Stack.Top() );
		m_Stack.Pop();
	}
	m_bHasInitialMousePos = false;
	m_StackChanged = true;
	m_HasSeenDown.Clear();

	MenuStackChangedDelegate.Broadcast( this );
}

void EGMenuStack::PopTo( EGMenu* PopToMenu )
{
	if( PopToMenu )
	{
		if( PopToMenu == GetActiveMenu() )
		{
			// Nothing to do
			return;
		}

		// We deactivate the top menu, but everything else gets deinit only
		// no activates or deactivates.
		if( m_Stack.HasItems() )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		}

		while( m_Stack.HasItems() && PopToMenu != m_Stack.Top() )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
			m_StackDelete.Push( m_Stack.Top() );
			m_Stack.Pop();
		}

		if( GetActiveMenu() == PopToMenu )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::ACTIVATE );
			if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.
		}
		else
		{
			assert( false ); // Pop to menu didn't exist.
			assert( m_Stack.IsEmpty() );
		}

		m_bHasInitialMousePos = false;
		m_StackChanged = true;
		m_HasSeenDown.Clear();
	}
	else
	{
		Clear();
	}

	MenuStackChangedDelegate.Broadcast( this );
}

EGMenu* EGMenuStack::PopToSwitchTo( EGMenu* PopToMenu, eg_string_crc SwitchToMenuId )
{
	EGMenu* NewMenu = nullptr;

	if( PopToMenu )
	{
		if( PopToMenu == GetActiveMenu() )
		{
			return Push( SwitchToMenuId );
		}

		// We deactivate the top menu, but everything else gets deinit only
		// no activates or deactivates.
		if( m_Stack.HasItems() )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
		}

		while( m_Stack.HasItems() && PopToMenu != m_Stack.Top() )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
			m_StackDelete.Push( m_Stack.Top() );
			m_Stack.Pop();
		}

		if( GetActiveMenu() == PopToMenu )
		{
			UnacquireInput();
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEACTIVATE );
			m_Stack.Top()->ProcessEvent( EGMenu::eg_menuevent_t::DEINIT );
			m_StackDelete.Push( m_Stack.Top() );
			m_Stack.Pop();
			NewMenu = InternalInitMenu( SwitchToMenuId );
		}
		else
		{
			assert( false ); // Pop to menu didn't exist.
			assert( m_Stack.IsEmpty() );
			Push( SwitchToMenuId );
		}
	}
	else
	{
		Clear();
		NewMenu = Push( SwitchToMenuId );
	}

	m_bHasInitialMousePos = false;
	m_StackChanged = true;
	m_HasSeenDown.Clear();

	MenuStackChangedDelegate.Broadcast( this );

	return NewMenu;
}

EGMenu* EGMenuStack::GetParentMenu( const EGMenu* Menu ) const
{
	for( eg_size_t i=0; i < Len(); i++ )
	{
		if( GetMenuByIndex( i ) == Menu )
		{
			return i > 0 ? GetMenuByIndex( i-1 ) : nullptr;
		}
	}

	return nullptr;
}

void EGMenuStack::Update( eg_real SecondsElapsed , const struct egLockstepCmds* Input , eg_real AspectRatio )
{
	m_LastKnownAspectRatio = AspectRatio;
	CullDeletedMenus();

	m_StackChanged = false; //Always reset the stack changed state.

	m_MouseObj.Update( SecondsElapsed );

	//Do update for all menus
	for( eg_uint i=0; i<m_Stack.Len(); i++ )
	{
		EGMenu* Menu = m_Stack[i];
		Menu->ProcessEvent( EGMenu::eg_menuevent_t::UPDATE , SecondsElapsed , m_LastKnownAspectRatio );
		if( HasStackChanged() )return;//Don't process anything more, since the menu may have been destroyed.
	}

	//Process Input for the topmost menu.
	if( m_Stack.HasItems() )
	{
		EGMenu* Menu = m_Stack.Top();
		Update_HandleMouse( Menu , Input );
		if( HasStackChanged() )return;//Don't process anything more since the menu may have been destroyed.

		//Post menu events to the topmost menu.
		static const struct
		{
			eg_cmd_t     Action1;
			eg_cmd_t     Action2;
			eg_cmd_t     Action3;
			eg_menuinput_t Message;
			eg_bool      IsMenuPress:1;
			eg_bool      IsAltNav:1;
		} 
		ActionToMsgTable[] =
		{
			{ CMDA_MENU_PRIMARY      , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_PRIMARY , true  , false },
			{ CMDA_MENU_BACK         , CMDA_UNK           , CMDA_MENU_RMB , eg_menuinput_t::BUTTON_BACK    , true  , false },
			{ CMDA_MENU_UP           , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_UP      , false , false },
			{ CMDA_MENU_DOWN         , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_DOWN    , false , false },
			{ CMDA_MENU_LEFT         , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_LEFT    , false , false },
			{ CMDA_MENU_RIGHT        , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_RIGHT   , false , false },
			{ CMDA_MENU_SCRLUP       , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::SCROLL_UP      , false , false },
			{ CMDA_MENU_SCRLDOWN     , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::SCROLL_DOWN    , false , false },
			{ CMDA_MENU_PRIMARY2     , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_PRIMARY , true  , true  },
			{ CMDA_MENU_BACK2        , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_BACK    , true  , true  },
			{ CMDA_MENU_UP2          , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_UP      , false , true  },
			{ CMDA_MENU_DOWN2        , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_DOWN    , false , true  },
			{ CMDA_MENU_LEFT2        , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_LEFT    , false , true  },
			{ CMDA_MENU_RIGHT2       , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::BUTTON_RIGHT   , false , true  },
			{ CMDA_MENU_NEXT_PAGE    , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::NEXT_PAGE      , false , false },
			{ CMDA_MENU_PREV_PAGE    , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::PREV_PAGE      , false , false },
			{ CMDA_MENU_NEXT_SUBPAGE , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::NEXT_SUBPAGE   , false , false },
			{ CMDA_MENU_PREV_SUBPAGE , CMDA_UNK           , CMDA_UNK      , eg_menuinput_t::PREV_SUBPAGE   , false , false },
		};

		auto CheckIfSeenDown = [this,&Input]( eg_cmd_t Cmd ) -> void
		{
			if( Input->WasPressed( Cmd ) )
			{
				m_HasSeenDown.SetIsActive( Cmd , true );
			}
		};

		auto WasMenuPressed = [this,&Input]( eg_cmd_t Cmd ) -> eg_bool
		{
			return Input->WasMenuPressed( Cmd ) && m_HasSeenDown.IsActive( Cmd );
		};

		for(eg_uint i=0; i<countof(ActionToMsgTable); i++)
		{
			CheckIfSeenDown( ActionToMsgTable[i].Action1 );
			CheckIfSeenDown( ActionToMsgTable[i].Action2 );
			CheckIfSeenDown( ActionToMsgTable[i].Action3 );

			//Some actions are found out using WasMenuPressed (which may actually be
			//a release depending on preference, others are always the actual press)
			//Part of the reason this is done, is because it's likely that BUTTON_BACK
			//is mapped to the same key that brings up the pause menu (escape) and we
			//don't want that to be a press and back to be a release because then 
			//the pause menu will just go away immediately.
			eg_bool WasPressed = false;
			eg_bool WasRepeated = false;
			if( ActionToMsgTable[i].IsAltNav && Menu->IsAltNavIgnored() )
			{
				// Don't count if alt nav is ignored.
			}
			else if( ActionToMsgTable[i].IsMenuPress )
			{
				WasPressed = WasMenuPressed(ActionToMsgTable[i].Action1) || WasMenuPressed(ActionToMsgTable[i].Action2) || WasMenuPressed(ActionToMsgTable[i].Action3);
			}
			else
			{
				WasPressed = Input->WasPressed(ActionToMsgTable[i].Action1) || Input->WasPressed(ActionToMsgTable[i].Action2) || Input->WasPressed(ActionToMsgTable[i].Action3);
				if( !WasPressed )
				{
					WasPressed = Input->WasRepeated(ActionToMsgTable[i].Action1) || Input->WasRepeated(ActionToMsgTable[i].Action2) || Input->WasRepeated(ActionToMsgTable[i].Action3);
					WasRepeated = true;
				}
			}

			if(WasPressed)
			{
				Menu->HandleInput( ActionToMsgTable[i].Message );
				if( HasStackChanged() )return;//Don't process anything more, since the menu may have been destroyed.
			}
		}

		Menu->ProcessInputCmds( *Input );
		if( HasStackChanged() )return;//Don't process anything more, since the menu may have been destroyed.
	}

	CullDeletedMenus();
}

void EGMenuStack::CullDeletedMenus( void )
{
	//Clean up: delete any menus that were deleted.
	while( m_StackDelete.HasItems() )
	{
		EG_SafeRelease( m_StackDelete.Top() );
		m_StackDelete.Pop();
	}
}

EGMenu* EGMenuStack::InternalInitMenu( eg_string_crc MenuId )
{
	const EGUiLayout* Layout = EGUiLayoutMgr::Get()->GetLayout( MenuId );
	EGClass* MenuClass = Layout->m_ClassName.Class;
	EGMenu* NewMenu = EGNewObject<EGMenu>( MenuClass , eg_mem_pool::DefaultHi );
	if( NewMenu )
	{
		NewMenu->Init( MenuId, this, nullptr, m_Owner );

		//Push the new menu.
		m_Stack.Push( NewMenu );
		m_StackChanged = false;
		NewMenu->ProcessEvent( EGMenu::eg_menuevent_t::INIT );
		if( HasStackChanged() )
		{
			if( !IsMenuOnStack( NewMenu ) )
			{
				NewMenu = nullptr;
			}
			return NewMenu; //Don't process anything more, since the menu may have been destroyed.
		}
		NewMenu->ProcessEvent( EGMenu::eg_menuevent_t::ACTIVATE );
		if( HasStackChanged() )
		{
			if( !IsMenuOnStack( NewMenu ) )
			{
				NewMenu = nullptr;
			}
			return NewMenu; //Don't process anything more, since the menu may have been destroyed.
		}
		NewMenu->ProcessEvent( EGMenu::eg_menuevent_t::UPDATE, 0.f, m_LastKnownAspectRatio );
		if( HasStackChanged() )
		{
			if( !IsMenuOnStack( NewMenu ) )
			{
				NewMenu = nullptr;
			}
			return NewMenu; //Don't process anything more, since the menu may have been destroyed.
		}
	}

	if( !IsMenuOnStack( NewMenu ) )
	{
		NewMenu = nullptr;
	}
	return NewMenu;
}

eg_bool EGMenuStack::IsMenuOnStack( EGMenu* Menu ) const
{
	for( eg_uint i=0; i<m_Stack.Len(); i++ )
	{
		if( m_Stack[i] == Menu )
		{
			return true;
		}
	}

	return false;
}

void EGMenuStack::SetMouseCursor( eg_string_crc NewCursor )
{
	m_MouseObj.Deinit();
	m_MouseObj.Init( NewCursor );
	m_MouseObj.RunEvent( eg_crc("Default") );
}

void EGMenuStack::Update_HandleMouse( EGMenu* Menu, const struct egLockstepCmds* Input )
{
	//As long as we are capturing input we find the window the mouse is
	//pointing at.
	const eg_real xPos = Input->m_MousePos.x;
	const eg_real yPos = Input->m_MousePos.y;

	const eg_bool bMouseMoved = m_bHasInitialMousePos && ( m_v2LastMousePos.x != xPos || m_v2LastMousePos.y != yPos );

	m_v2LastMousePos.x = xPos;
	m_v2LastMousePos.y = yPos;

	m_bHasInitialMousePos = true;

	static const eg_cmd_t MouseMoveCmds[] =
	{
		CMDA_MENU_RMB,
		CMDA_MENU_LMB,
	};

	bool MouseMoveCmdActive = false;
	for( eg_uint i=0; i<countof(MouseMoveCmds); i++ )
	{
		MouseMoveCmdActive = MouseMoveCmdActive || Input->WasPressed( MouseMoveCmds[i] ) || Input->WasReleased( MouseMoveCmds[i] );
	}
	//If the mouse was moved, or if a mouse related command was pressed, send the mouse moved event.
	if( bMouseMoved || MouseMoveCmdActive )
	{
		Menu->HandleMouseEvent( eg_menumouse_e::MOVE , xPos , yPos );
		if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.
	}

	Menu->HandleMouseEvent( eg_menumouse_e::POSITION , xPos , yPos );
	if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.

	if( Input->WasPressed( CMDA_MENU_LMB ) )
	{
		Menu->HandleMouseEvent( eg_menumouse_e::PRESSED , xPos , yPos );
		if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.
	}

	if( Input->WasReleased( CMDA_MENU_LMB ) )
	{
		Menu->HandleMouseEvent( eg_menumouse_e::RELEASED , xPos , yPos );
		if( HasStackChanged() )return; //Don't process anything more, since the menu may have been destroyed.
	}
}

void EGMenuStack::Draw()
{
	MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	// MainDisplayList->PushSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT ); // Do we really want this? // Removed for DEGAME and probably was never good.
	MainDisplayList->SetMaterial(EGV_MATERIAL_NULL);
	
	for( eg_uint i=0; i<m_Stack.Len(); i++ )
	{
		MainDisplayList->ClearDS( 1.f , 0 );
		m_Stack[i]->ProcessEvent( EGMenu::eg_menuevent_t::PRE_DRAW , m_LastKnownAspectRatio );
		m_Stack[i]->ProcessEvent( EGMenu::eg_menuevent_t::DRAW , m_LastKnownAspectRatio );
		m_Stack[i]->ProcessEvent( EGMenu::eg_menuevent_t::POST_DRAW , m_LastKnownAspectRatio );
	}

	// MainDisplayList->PopSamplerState();
	MainDisplayList->PopDefaultShader();
}

void EGMenuStack::DrawCursor( eg_real AspectRatio , eg_real x , eg_real y )
{
	static const eg_real MOUSE_ORTHO_SIZE = 2.f;
	EGCamera2 MouseCam( CT_Default );
	MouseCam.InitOrthographic( AspectRatio , MOUSE_ORTHO_SIZE*2.f , -MOUSE_ORTHO_SIZE , MOUSE_ORTHO_SIZE );
	MainDisplayList->SetProjTF(MouseCam.GetProjMat());
	MainDisplayList->SetViewTF(MouseCam.GetViewMat());

	MainDisplayList->ClearDS( 1.0f , 0 );

	eg_vec4 ScaleVec = eg_vec4(MOUSE_ORTHO_SIZE,MOUSE_ORTHO_SIZE,MOUSE_ORTHO_SIZE,1.f);
	eg_transform Pose( CT_Default );
	Pose = eg_transform::BuildTranslation( eg_vec3(x*MOUSE_ORTHO_SIZE , y*MOUSE_ORTHO_SIZE ,0) );

	m_MouseObj.SetDrawInfo( Pose , ScaleVec , false );
	m_MouseObj.Draw();
}