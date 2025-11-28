// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"

class ExCombatant;
class EGRandom;
class ExGame;
struct exSpellInfo;

ExCombatant* ExCombatAI_ChooseCombatantToAttack( EGArray<ExCombatant>& Team , EGRandom& Rng , eg_bool bForceInOrder , ex_target_strategy Strategy , eg_bool bRanged );
void ExCombatAI_TestChoosCombatantToAttack( const ExGame* InGame );

const exSpellInfo* ExCombatAI_ChooseSpellToUse( const ExCombatant* Combatant , EGRandom& Rng );
