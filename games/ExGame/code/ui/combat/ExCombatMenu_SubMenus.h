// (c) 2016 Beem Media

#pragma once

#include "EGLocText.h"
#include "ExCombatTypes.h"

class ExCombatMenu;
class ExCombatant;
class ExChoiceWidgetMenu;

class ExCombatMenu_SpellsList : public EGObject
{
	EG_CLASS_BODY( ExCombatMenu_SpellsList , EGObject )

private:

	ExCombatMenu* m_Owner;
	ExCombatant* m_Combatant;

public:

	void Init( ExCombatMenu* InOwner ){ m_Owner = InOwner; m_Combatant = nullptr; }

	void PushChoiceMenu( ExCombatant* Combatant );
	void OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void OnChoiceAborted( ExChoiceWidgetMenu* SourceMenu );
	eg_loc_text GetChoiceText( eg_uint Index ) const;
};


class ExCombatMenu_CombatantAction : public EGObject
{
	EG_CLASS_BODY( ExCombatMenu_CombatantAction , EGObject )

private:

	ExCombatMenu* m_Owner;
	ExCombatant* m_Combatant;
	ExCombatActionsArray m_Actions;
	ExCombatMenu_SpellsList* m_SpellsList;

public:

	void Init( ExCombatMenu* InOwner ) 
	{
		m_Owner = InOwner;
		m_Actions = CT_Clear; 
		m_SpellsList = EGNewObject<ExCombatMenu_SpellsList>(); 
		if( m_SpellsList )
		{
			m_SpellsList->Init( InOwner );
		}
	}

	virtual void OnDestruct() override
	{
		EGDeleteObject( m_SpellsList );
		Super::OnDestruct();
	}

	void PushChoiceMenu( ExCombatant* Combatant );
	static eg_string_crc GetTextForAction( ex_combat_actions Action );

	void OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	eg_loc_text GetChoiceText( eg_uint Index ) const;
};
