// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"

class ExDebugCombatLauncherMenu : public ExMenu
{
	EG_CLASS_BODY( ExDebugCombatLauncherMenu , ExMenu )

public:

	static void OpenSampleCombat( EGClient* Client , eg_string_crc MonsterId , eg_int MonsterLevel , ex_class_t TeamClass );

protected:

	virtual void OnInit() override;
	virtual void OnActivate() override;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override;
};
