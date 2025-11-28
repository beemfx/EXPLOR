// (c) 2018 Beem Media

#pragma once

#include "ExMenu.h"
#include "ExDialogMenu.h"
#include "ExCombat.h"

class ExCombatPartyWidget;
class ExCombatMenu_CombatantAction;
class ExCombatMenuOpponentsWidget;
class ExItemCardWidget;
struct egUiGridWidgetCellClickedInfo2;

class ExCombatMenu : public ExMenu , public IExDialogListener
{
	EG_CLASS_BODY( ExCombatMenu , ExMenu )

public:

	enum class ex_menu_s
	{
		UNK,
		BEGIN,
		PLAYER_TURN,
		PLAYER_TURN_MAKING_CHOICE,
		PLAYER_CHOOSING_TARGET_TEAMA,
		PLAYER_CHOOSING_TARGET_TEAMB,
		PLAYER_TURN_CHOICE_COMMITTED,
		AI_TURN,
		COMPLETE,
	};

private:

	EGClient*                     m_OwnerClient;
	EGUiWidget*                   m_CombatOrder;
	ExCombatPartyWidget*          m_TeamA;
	EGUiWidget*                   m_HelpBg;
	ExHintBarWidget*              m_HelpText; // Specifially not named HintBar so we can control it directly not the menu.
	ExHintBarWidget*              m_QuickCombatHelpText;
	EGUiWidget*                   m_AdditionalMonstersBg;
	EGUiTextWidget*               m_AdditionalMonstersText;
	EGUiWidget*                   m_NextMonsterBg;
	EGUiTextWidget*               m_NextMonsterText;
	ExCombat*                     m_Combat;
	ExCombat::exTurnInput         m_TurnInput = ExCombat::exTurnInput( CT_Default );
	ex_menu_s                     m_MenuState = ex_menu_s::UNK;
	ExCombatantList               m_TeamAList;
	ExCombatantList               m_TeamBList;
	ExCombatantList               m_CombatOrderList;
	ExCombatMenu_CombatantAction* m_BaseChoiceCb;
	ExCombatMenuOpponentsWidget*  m_OpponentsWidget;
	ExItemCardWidget*             m_SpellInfoWidget;
	ExItemCardWidget*             m_CombatChoiceInfoWidget;
	eg_real                       m_TimeTilAutoTurn;

public:

	ExCombatMenu();

	virtual ~ExCombatMenu() override;
	virtual void Refresh() override final;
	void ChangeMenuState( ex_menu_s NewState );

	void ApplyAmbush();

	virtual void OnInit() override final;
	virtual void OnDeinit() override final;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final;
	virtual void OnActivate() override final;
	virtual void OnDeactivate() override final;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final;
	virtual void OnDialogClosed( eg_string_crc ListenerParm , eg_string_crc Choice ) override final;
	virtual void OnInputCmds( const struct egLockstepCmds& Cmds ) override final;
	void OnTurnComplete();
	void SendTurnInputToCombat();
	void SetCombatAction( ex_combat_actions Action );
	void SetCombatActionIndex( eg_uint Index );
	void CommitCombatantChoice();
	void Continue();
	void OnPlayerPortraitClicked( egUiGridWidgetCellClickedInfo2& CellInfo );
	void OnEnemyTargetChosen( const ExCombatant* Target );
	void AttemptTargetSelection( ExCombatant* Target );
	void RevealCombatLog();
	void ForceFullCombatLog();
	eg_bool IsCombatLogFullyRevealed();
	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo );
	void RefreshGrids();
	void ChangeMusicForCombatComplete();
	void ChangeMusicForMenuDismissed();
	void SetViewedSpell( ExCombatant* Combatant , eg_uint SpellChoice );
	void SetSpellCardOffset( const eg_transform& Transform );
	void SetViewedCombatActionOffset( const eg_transform& Transform );
	void SetViewedCombatChoice( ExCombatant* Combatant , ex_combat_actions Action , const exSpellInfo* Spell = nullptr );
	void PopulateHints();
	void ClearHints();
	eg_bool ShouldShowNextCombatant() const;
	void RefreshNextCombatant();
};
