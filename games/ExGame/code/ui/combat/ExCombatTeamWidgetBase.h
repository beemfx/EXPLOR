// (c) 2017 Beem Media

#pragma once

#include "ExCombatTypes.h"
#include "EGEntObj.h"

class ExCombat;
class ExCombatTeam;

class ExCombatTeamWidgetBase : public IExCombatAnimHandler
{
protected:

	struct exAnimInfo
	{
		EGEntObj             EntObj;	
		eg_string_crc        StartAnim;
		eg_real              AnimDuration;
	};

	struct exPlayingAnimInfo
	{
		EGEntObj             EntObj;
		eg_transform         Pose = CT_Default;
		eg_vec4              ScaleVec = eg_vec4(1.f,1.f,1.f,1.f);
		eg_real              AnimTimeLeft;
	};

	typedef EGFixedArray<exAnimInfo,enum_count(ex_combat_anim)> ExAnimArray;
	typedef EGArray<exPlayingAnimInfo*> ExPlayingAnimArray;

protected:

	ExCombat*          m_Combat;
	ExCombatantList    m_CombatantList;
	ExAnimArray        m_AnimTemplates;
	ExPlayingAnimArray m_PlayingAnims;

public:

	void InitCombatTeamWidgetBase();
	void DeintCombatTeamWidgetBase();
	void PlayCombatTeamWidgetBaseAnimation( ex_combat_anim Anim , const eg_transform& Pose , eg_real Scale , const eg_color32& PaletteColor );
	void PlayDamageNumber( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale , eg_bool bIsHeal );
	void PlayBurgled( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale );
	void PlayCriticalHit( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale );
	void PlayResisted( ex_attr_t ResistType , const eg_transform& Pose , eg_real Scale );
	void UpdateCombatTeamWidgetBase( eg_real DeltaTime );
	void DrawCombatTeamWidgetBase( const eg_transform& BasePose );

	virtual void InitCombat( class ExCombat* Combat );
	virtual void SetCombatantList( const ExCombatTeam& Team , const ExCombatantList& List );
};
