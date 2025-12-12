// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "ExSpellInfo.h"
#include "ExCombatant.h"
#include "ExCombat.h"

class ExChoiceWidgetMenu;
class ExCharacterPortraitWidget2;
class ExCombatPartyWidget;
struct egUiGridWidgetCellChangedInfo2;
struct egUiGridWidgetCellClickedInfo2;
struct exCombatAnimInfo;
class ExItemCardWidget;

class ExCastSpellInWorldMenu : public ExMenu
{
	EG_CLASS_BODY( ExCastSpellInWorldMenu , ExMenu )

protected:

	struct exFighterWrapper
	{
		ExCombatant Combatant;
	};

protected:

	const ExFighter* m_Fighter;
	EGArray<exFighterWrapper> m_Party;
	eg_int m_FighterIndex = -1;
	ExSpellList m_SpellList;
	eg_int m_ChosenSpell = -1;
	eg_bool m_bDismissOnActivate = false;
	eg_int m_ChosenTarget = -1;
	eg_bool m_bCountingDownToClose = false;
	eg_real m_CountdownTimer = .5f;
	EGRandom m_Rng = CT_Default;

	ExCombatPartyWidget* m_CharacterPortraitsGrid;
	EGUiWidget* m_FadeWidget;
	EGUiWidget* m_HelpBg;
	ExHintBarWidget* m_HelpText; // Specifially not named HintBar so we can control it directly not the menu.
	ExItemCardWidget* m_SpellInfoWidget;

public:

	static eg_bool CanCastAnyInWorldSpells( const ExGame* Game );

protected:

	virtual void OnInit() override;
	virtual void OnDeinit() override;
	virtual void OnActivate() override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override;

	eg_loc_text GetChoiceText( eg_uint Index ) const;
	void OnChoiceMade( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void OnChoiceHighlighted( ExChoiceWidgetMenu* SourceMenu , eg_uint ChoiceIndex );
	void OnChoiceAborted( ExChoiceWidgetMenu* SourceMenu );
	void OnPortraitCellChanged( egUiGridWidgetCellChangedInfo2& CellInfo );
	void OnPortraitClicked( egUiGridWidgetCellClickedInfo2& CellInfo );

	void DismissSpellMenu( eg_bool bImmediateAndLoad , eg_bool bWasCancel );

	eg_bool DoesSpellNeedTarget() const;
	void ProcessSpell();
	void HandleAnim( const exCombatAnimInfo& AnimInfo );

	void PopulateHints();

	void SetViewedSpell( eg_uint SpellChoice );
	void SetSpellCardOffset( const eg_transform& Transform );
};
