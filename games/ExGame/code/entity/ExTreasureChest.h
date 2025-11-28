#pragma once

#include "ExIoEnt.h"
#include "ExTreasureChest.reflection.h"

egreflect class ExTreasureChest : public egprop ExIoEnt
{
	EG_CLASS_BODY( ExTreasureChest , ExIoEnt )
	EG_FRIEND_RFL( ExTreasureChest )

private:

	eg_bool m_bIsUnique:1;
	eg_bool m_bIsOpen:1;
	eg_real m_FadeOutTime;
	eg_bool m_bIsFadingOut;
	exTreasureChestResults m_Contents = CT_Preserve;

public:

	virtual void OnCreate( const egEntCreateParms& Parms ) override;
	virtual void OnEnterWorld() override;
	virtual void OnEncounter() override;
	virtual void OnUpdate( eg_real DeltaTime ) override;
	void OpenTreasureChest();
	void CloseTreasureChest();
	void RunTreasureScript();
	void AddGold( eg_int GoldAmount );
	void SetUnique( eg_bool bNewValue ) { m_bIsUnique = bNewValue; }
	eg_bool IsUnique() const { return m_bIsUnique; }

	void FadeOut();

protected:

	virtual void ParseInitString( const eg_d_string& InitString ) override;
};
