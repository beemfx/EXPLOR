// (c) 2017 Beem Media

#pragma once

#include "EGUiGridWidget2.h"
#include "ExCombatTeamWidgetBase.h"
#include "ExCombatTypes.h"
#include "EGRandom.h"
#include "EGAudioTypes.h"

class ExGame;
class ExCombat;
class ExCombatant;
class ExCharacterPortraitWidget2;

class ExCombatPartyWidget : public EGUiGridWidget2 , public ExCombatTeamWidgetBase
{
	EG_CLASS_BODY( ExCombatPartyWidget , EGUiGridWidget2 )

private:

	ExCharacterPortraitWidget2* m_PortraitWidgets[6];
	eg_uint                     m_HighlightedPortrait = countof(m_PortraitWidgets);
	EGFixedArray<egs_sound,2>   m_MaleGotHit;
	EGFixedArray<egs_sound,2>   m_FemaleGotHit;
	EGRandom                    m_Rng;
	EGTween<eg_real>            m_ScaleTarget;
	eg_bool                     m_bBigPortraits = true;
	eg_bool                     m_bIsCastSpellInWorld = false;

public:

	void SetScaledUp( eg_bool bNewValue );
	void SetIsCastSpellInWorld( eg_bool bNewValue ) { m_bIsCastSpellInWorld = bNewValue; }

	virtual void Init( EGMenu* InOwner , const egUiWidgetInfo* InInfo ) override;
	void SetHighlightedPortrait( eg_uint Index );
	void SetHighlightedFighter( ExCombatant* Combatant );
	void SetUsingBigPortraits( eg_bool bNewValue ) { m_bBigPortraits = bNewValue; }

	// BEGIN EGUiGridWidget
	virtual void OnConstruct() override final;
	virtual void OnDestruct() override final;
	virtual void Update( float DeltaTime , float AspectRatio ) override final;
	virtual void Draw( eg_real AspectRatio ) override final;
	// END EgUiGridWidget

	void OnGridWidgetItemChanged( egUiGridWidgetCellChangedInfo2& CellInfo );
	void OnGridWidgetItemChangedFromCastSpellInWorld( egUiGridWidgetCellChangedInfo2& CellInfo , ExGame* Game );


	// BEGIN ExCombatTeamWidgetBase
	virtual void SetCombatantList( const ExCombatTeam& Team , const ExCombatantList& List ) override final;
	virtual void PlayAnimation( const exCombatAnimInfo& AnimInfo ) override final;
	// END ExCombatTeamWidgetBase

	static eg_transform GetAnimationOffset( eg_int PortraitIndex , eg_real StatusOffset , eg_bool bBigPortraits );
};
