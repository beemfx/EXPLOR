// (c) 2018 Beem Media

#pragma once

#include "EGUiMeshWidget.h"
#include "ExInventoryBag.h"
#include "ExFighter.h"
#include "ExCombatTypes.h"

struct exInventoryBagItem;
class ExFighter;
struct exSpellInfo;

class ExItemCardWidget : public EGUiMeshWidget
{
	EG_CLASS_BODY( ExItemCardWidget , EGUiMeshWidget )

public:

	enum class ex_mode
	{
		Default,
		VendorSell,
		VendorBuy,
		VendorBuyback,
	};

private:

	ex_mode m_Mode = ex_mode::Default;
	exInventoryBagItem m_ItemBeingViewed;
	ExFighter m_TargetFighter = CT_Default;

public:

	void SetMode( ex_mode InMode );
	void SetItem( const exInventoryBagItem* BagItem , const ExFighter* TargetFighter , eg_bool bIncludeEquipment = false );
	void SetSpell( const exSpellInfo* SpellInfo , const ExFighter* TargetFighter , eg_bool bIncludeEquipment );
	void SetAttack( const ExFighter* TargetFighter , eg_bool bIncludeEquipment , ex_combat_actions ActionType , eg_bool bCanMelee , eg_bool bCanShoot );
	void SetSpellTargeting( const ExFighter* TargetFighter , eg_bool bIncludeEquipment , ex_combat_actions ActionType , const exSpellInfo* Spell );
	const exInventoryBagItem& GetItemBeingViewed() const { return m_ItemBeingViewed; }
};
