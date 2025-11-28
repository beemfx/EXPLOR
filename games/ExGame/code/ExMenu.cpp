// (c) 2021 Beem Media. All Rights Reserved.

#include "ExMenu.h"


eg_bool ExMenu::HandlePossibleMenuToggle( ex_toggle_menu_t MenuType , const struct egLockstepCmds& Cmds )
{
	eg_cmd_t ToggleMenuCmd = CMDA_UNK;
	
	switch( MenuType )
	{
	case ex_toggle_menu_t::QuestLog:
		ToggleMenuCmd = CMDA_QUESTLOG;
		break;
	case ex_toggle_menu_t::Map:
		ToggleMenuCmd = CMDA_MAPMENU;
		break;
	case ex_toggle_menu_t::Inventory:
		ToggleMenuCmd = CMDA_QUICK_INVENTORY;
		break;
	case ex_toggle_menu_t::Character:
		ToggleMenuCmd = CMDA_VIEWCHARMENU;
		break;
	}

	if( Cmds.WasMenuPressed( ToggleMenuCmd ) )
	{
		if( EGClient* ClientOnwer = GetClientOwner() )
		{
			EGArray<eg_cmd_t> MenuCmds;
			MenuCmds.Append( CMDA_MENU_PRIMARY );
			MenuCmds.Append( CMDA_MENU_PRIMARY2 );
			MenuCmds.Append( CMDA_MENU_BACK );
			MenuCmds.Append( CMDA_MENU_BACK2 );
			MenuCmds.Append( CMDA_MENU_LMB );
			MenuCmds.Append( CMDA_MENU_RMB );
			MenuCmds.Append( CMDA_MENU_UP );
			MenuCmds.Append( CMDA_MENU_UP2 );
			MenuCmds.Append( CMDA_MENU_DOWN );
			MenuCmds.Append( CMDA_MENU_DOWN2 );
			MenuCmds.Append( CMDA_MENU_LEFT );
			MenuCmds.Append( CMDA_MENU_LEFT2 );
			MenuCmds.Append( CMDA_MENU_RIGHT );
			MenuCmds.Append( CMDA_MENU_RIGHT2 );
			MenuCmds.Append( CMDA_MENU_SCRLUP );
			MenuCmds.Append( CMDA_MENU_SCRLDOWN );
			if( false )
			{
				MenuCmds.Append( CMDA_MENU_BTN2 );
			}
			if( MenuType == ex_toggle_menu_t::Inventory || MenuType == ex_toggle_menu_t::QuestLog )
			{
				MenuCmds.Append( CMDA_MENU_BTN3 );
			}
			if( MenuType == ex_toggle_menu_t::Character || MenuType == ex_toggle_menu_t::Inventory )
			{
				MenuCmds.Append( CMDA_MENU_NEXT_PAGE );
				MenuCmds.Append( CMDA_MENU_PREV_PAGE );
			}
			if( false )
			{
				MenuCmds.Append( CMDA_MENU_NEXT_SUBPAGE );
				MenuCmds.Append( CMDA_MENU_PREV_SUBPAGE );
			}
			if( MenuType == ex_toggle_menu_t::Inventory )
			{
				MenuCmds.Append( CMDA_MENU_DELETE );
			}
			if( false )
			{
				MenuCmds.Append( CMDA_KBMENU_BKSP );
			}
			if( MenuType == ex_toggle_menu_t::Inventory )
			{
				MenuCmds.Append( CMDA_MENU_COMPARE );
			}
			MenuCmds.Append( CMDA_GAMEMENU_CLOSE );


			const eg_bool bHasBindConflicts = ClientOnwer->GetInputBindings().DoBindingConflictsExist( ToggleMenuCmd , MenuCmds );
			if( !bHasBindConflicts )
			{
				MenuStack_Pop();
				return true;
			}
		}
	}

	return false;
}
