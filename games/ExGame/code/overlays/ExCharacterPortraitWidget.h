// (c) 2016 Beem Media. All rights reserved.

#pragma once

#include "EGUiGridWidgetItem.h"
#include "ExCombatTypes.h"

class EGUiWidget;
class ExFighter;
class ExGame;
class ExCombatant;
class EGMeshComponent;

class ExCharacterPortraitWidget2 : public EGUiGridWidgetItem
{
	EG_CLASS_BODY( ExCharacterPortraitWidget2 , EGUiGridWidgetItem )

	static const eg_real BAR_MIN;
	static const eg_real BAR_MAX;

	const ExFighter*   m_Character;
	eg_uint            m_PartyIndex;
	ex_combat_s        m_CombatState = ex_combat_s::OUT_OF_COMBAT;
	eg_real            m_CombatStatusOffsetStart = 0.f;
	eg_real            m_CombatStatusOffsetEnd = 0.f;
	eg_real            m_CombatStatusOffsetTime = 0.f;
	EGMeshComponent*   m_LevelUpIcon = nullptr;
	EGMeshComponent*   m_BoostIcon = nullptr;
	eg_bool            m_bAnimateBars = false;
	exResourceBarTween m_HealthTween;
	exResourceBarTween m_MagicTween;

public:
	
	void InitPortrait( const ExFighter* Character, eg_uint PartyIndex, const ExGame* Game );
	void SetAnimateBars( eg_bool bNewValue ) { m_bAnimateBars = bNewValue; }
	virtual void Update( eg_real DeltaTime , eg_real AspectRatio ) override;
	virtual void SetVisible( eg_bool Visible ) { EGUiMeshWidget::SetVisible( Visible ); }
	void Refresh();
	void SetHighlighted( eg_bool bHighlighted );
	const ExFighter* GetCharacter() const { return m_Character; }
	void UpdateCombatantView( eg_real DeltaTime , const ExCombatant* Combatant );
	eg_real GetCombatStatusOffset() const;

private:
	
	void UpdateBars( eg_real DeltaTime );
	void SetTargetOffset( eg_real NewOffset , eg_real PrevOffset );
};
