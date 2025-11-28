// (c) 2020 Beem Media. All Rights Reserved.

#include "ExDebugCombatLauncherMenu.h"
#include "EGMenuStack.h"
#include "ExCombat.h"

EG_CLASS_DECL( ExDebugCombatLauncherMenu )

void ExDebugCombatLauncherMenu::OpenSampleCombat( EGClient* Client , eg_string_crc MonsterId , eg_int MonsterLevel , ex_class_t TeamClass )
{
	ExCombat::s_SampleCombatOpponentList.Clear();
	ExCombat::exSampleCombatMonsterInfo NewMonsterInfo;
	NewMonsterInfo.MonsterId = MonsterId;
	NewMonsterInfo.MonsterLevel = MonsterLevel;
	ExCombat::s_SampleCombatOpponentList.Append( NewMonsterInfo );
	ExCombat::s_SampleCombatPlayerTeamLevel = MonsterLevel;
	ExCombat::s_SampleCombatTeamClass = TeamClass;

	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	if( MenuStack )
	{
		MenuStack->Push( eg_crc("DebugCombatLauncherMenu") );
	}

	ExCombat::s_SampleCombatOpponentList.Clear();
	ExCombat::s_SampleCombatPlayerTeamLevel = 0;
	ExCombat::s_SampleCombatTeamClass = ex_class_t::Unknown;
}

void ExDebugCombatLauncherMenu::OnInit()
{
	Super::OnInit();

	MenuStack_PushTo( eg_crc("CombatMenu") );
}

void ExDebugCombatLauncherMenu::OnActivate()
{
	MenuStack_Pop();
}

eg_bool ExDebugCombatLauncherMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		MenuStack_Pop();
		return true;
	}

	return Super::OnInput( InputType );
}
