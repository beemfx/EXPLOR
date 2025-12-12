// (c) 2017 Beem Media

#pragma once

#include "EGStringMap.h"
#include "EGBoolList.h"
#include "EGRemoteEvent.h"
#include "EGSmTypes.h"
#include "EGTween.h"
#include "ExGameTypes.reflection.h"

typedef eg_int32 ex_attr_value;
typedef eg_int64 ex_xp_value;

#if defined(__EGEDITOR__) || defined(__DEBUG__)
static const eg_bool EX_CHEATS_ENABLED = true;
#else
static const eg_bool EX_CHEATS_ENABLED = false;
#endif

static const eg_real EX_CLIP_DISTANCE = 1000.f;

static const eg_real EX_LEVEL_GROWTH = 1.25f;
static const eg_real EX_ITEM_COST_GROWTH_MULTIPLIER = 2.5f;
static const eg_int EX_ITEM_GROWTH_MULTIPLIER = 4;
static const eg_real EX_ITEM_LEVEL_GROWTH = 2.44140625f;// Items grow EX_ITEM_GROWTH_MULTIPLIER as fast as levels EGMath_pow(EX_LEVEL_GROWTH,EX_ITEM_GROWTH_MULTIPLIER);
static const ex_attr_value EX_MAX_LEVEL = 30;
static const ex_xp_value FIRST_LEVEL_UP_XP = 100;
static const ex_attr_value EX_BASE_LEVEL_UP_COST = 50;
static const eg_real EX_XP_GROWTH = 1.65f;
static const eg_real EX_GOLD_GROWTH = 1.45f;
static const eg_uint EX_MAX_ATTUNED_SKILLS = 6;
static const eg_real EX_SELL_PERCENT = .3f;
static const eg_uint MAX_BUYBACK_ITEMS = 9;

static const eg_real EX_TURN_TIME = .4f;

static const ex_attr_value EX_COMBAT_ROUND_DURATION = 100;
static const ex_attr_value EX_COMBAT_MAX_SPEED = 20;

static const eg_uint MAX_MAP_MONSTERS = 255;
static const eg_uint MAX_DEFATED_BOSSES = 1024;
static const eg_uint MAX_UNIQUE_TREASURE_CHESTS = 1024;
static const eg_uint MAX_GAME_VARS = 1024;

static const eg_real EX_BLOCKING_MULTIPLIER = 1.5f;

// UI Variables must match what is in content.
static const eg_int EX_CHOICE_WIDGET_VISIBLE_CHOICES = 10;
static const eg_real EX_COMBAT_PORTRAIT_FIRST_ROW_OFFSET = 8.f;
static const eg_real EX_PORTRAIT_SCALE = .5f;
static const eg_real EX_PORTRAIT_SCALE_BIG = .7f;
static const eg_real EX_PORTRAIT_WIDTH = 23.375f*2.f; // Really since it's always scaled the width is this multiplied by EX_PORTRAIT_SCALE

typedef EGFixedArray<eg_string_crc,MAX_UNIQUE_TREASURE_CHESTS> EGTreasureList;

typedef eg_loc_char ex_save_name[16];
static const eg_size_t EX_MAX_SAVE_NAME = (sizeof(ex_save_name)/sizeof(eg_loc_char))-1;
static const eg_size_t EX_SAVE_SLOTS = 20;

static const eg_loc_char EX_CHARACTER_NAME_ALLOWED_CHARS[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz 0123456789_,'\":;()$!`-";

egreflect enum class ex_attr_t
{
	//
	// Base attributes:
	//
	// Fixed
	XP_ ,
	LVL ,
	SPD ,
	STR ,
	END ,
	ACC ,
	MAG ,
	AGR ,
	BHP ,
	BMP ,

	//
	// Computed attributes:
	//
	DMG_ ,
	PDMG ,
	FDMG ,
	WDMG ,
	EDMG ,
	ADMG ,

	DEF_ ,
	PDEF ,
	FDEF ,
	WDEF ,
	EDEF ,
	ADEF ,

	HP  ,
	MP  ,
	HIT ,
	DG  ,
	ATK ,

};

egreflect enum class ex_class_t
{
	Unknown,    
	Warrior,
	Thief,
	Mage,        
	Cleric,
	EnemyMonster,
};

enum class ex_attr_set_t
{
	All,
	CharacterCreation,
	Boosts,
	BeastAttributes,
	WeaponDamage,
};

egreflect struct exAttrSet
{
	ex_attr_set_t SetTypeForEditor = ex_attr_set_t::All;

	egprop eg_int XP_;
	egprop eg_int LVL; 
	egprop eg_int SPD;
	egprop eg_int STR;
	egprop eg_int END;
	egprop eg_int ACC;
	egprop eg_int MAG;
	egprop eg_int AGR;
	egprop eg_int BHP;
	egprop eg_int BMP;
	egprop eg_int DMG_;
	egprop eg_int PDMG;
	egprop eg_int FDMG;
	egprop eg_int WDMG;
	egprop eg_int EDMG;
	egprop eg_int ADMG;
	egprop eg_int DEF_;
	egprop eg_int PDEF;
	egprop eg_int FDEF;
	egprop eg_int WDEF;
	egprop eg_int EDEF;
	egprop eg_int ADEF;
	egprop eg_int HP;
	egprop eg_int MP;
	egprop eg_int HIT;
	egprop eg_int DG;
	egprop eg_int ATK;

	exAttrSet() = default;
	exAttrSet( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			ZeroAll();
		}
	}
	exAttrSet( ex_attr_set_t InSetTypeForEditor )
	: SetTypeForEditor( InSetTypeForEditor )
	{
		ZeroAll();
	}

	void ZeroAll();

	const exAttrSet& operator += ( const exAttrSet& rhs )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) _var_ += rhs._var_;
		#include "ExAttrs.items"

		return *this;
	}

	ex_attr_value GetAttrValue( ex_attr_t Type ) const;
	void RefreshVisibleAttributes( struct egRflEditor* Editor ) const;

	friend inline eg_bool operator != ( const exAttrSet& Left , const exAttrSet& Right )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) if( Left._var_ != Right._var_ ) return true;
		#include "ExAttrs.items"

		return false;
	}

	friend inline eg_bool operator == ( const exAttrSet& Left , const exAttrSet& Right )
	{
		return !( Left != Right );
	}
};

egreflect struct exAttrBaseSet
{
	egprop eg_int SPD;
	egprop eg_int STR;
	egprop eg_int END;
	egprop eg_int ACC;
	egprop eg_int MAG;
	egprop eg_int AGR;
	egprop eg_int BHP;
	egprop eg_int BMP;

	// These just make it place nice with including ExAttrs.items
	eg_int XP_;
	eg_int LVL;

	exAttrBaseSet() = default;
	exAttrBaseSet( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			ZeroAll();

			if( CT_Default == Ct )
			{
				SPD = 1;
				STR = 1;
				END = 1;
				ACC = 1;
				MAG = 0;
				AGR = 0;
				BHP = 0;
				BMP = 0;
			}
		}
	}

	void ZeroAll()
	{
		SPD = 0;
		STR = 0;
		END = 0;
		ACC = 0;
		MAG = 0;
		AGR = 0;
		BHP = 0;
		BMP = 0;

		XP_ = 0;
		LVL = 0;
	}
};

egreflect enum class ex_race_t
{
	Unknown,
	Human,
};

egreflect enum class ex_gender_t
{
	Unknown,
	Male,
	Female,
};

egreflect enum class ex_spell_target
{
	None,
	EnemyActive,
	EnemyAny,
	AllyActive,
	AllyAny,
	AllActive,
	AllAny,
	Self,
};

egreflect enum class ex_spell_t
{
	Unknown,
	CombatDamage,
	AttributeBoost,
	Resurrect,
	TownPortalMarkAndGo,
	TownPortalRecall,
	FortifyRanks,
	ExpandRanks,
};

egreflect enum class ex_spell_func
{
	Fixed,
	MagicMul,
	MagicMulLvlGrowth,
};

egreflect enum class ex_target_strategy
{
	Default,
	Agression,
	PartyOrder,
	HighestHP,
	LowestHP,
	Kill,
	AgressionThenKill,
};

egreflect enum class ex_rarity_t
{
	Normal,
	Rare,
	VeryRare,
	Elite,
	Artifact,
};

enum class ex_map_reveal_result
{
	None,
	NewlyRevealed,
	AlreadyRevealed,
};

static inline ex_attr_value ExCore_GetRampedValue_Level( ex_attr_value BaseMultiplier , ex_attr_value Level , eg_real Growth = EX_LEVEL_GROWTH )
{
	return EGMath_floor( BaseMultiplier * EGMath_pow( Growth , static_cast<eg_real>(EG_Max<ex_attr_value>(0,Level-1)) ));
}

static inline ex_attr_value ExCore_GetRampedValue_Level( eg_real BaseMultiplier , ex_attr_value Level , eg_real Growth = EX_LEVEL_GROWTH )
{
	return EGMath_round( BaseMultiplier * EGMath_pow( Growth , static_cast<eg_real>(EG_Max<ex_attr_value>(0,Level-1)) ));
}

static inline ex_attr_value ExCore_GetRampedValue_Attr( ex_attr_value BaseMultiplier , ex_attr_value AttrValue , eg_real Growth = EX_LEVEL_GROWTH )
{
	return EGMath_floor( BaseMultiplier * EGMath_pow( Growth , static_cast<eg_real>(EG_Max<ex_attr_value>(0,AttrValue)) ));
}

static inline ex_attr_value ExCore_GetRampedValue_Attr( eg_real BaseMultiplier , ex_attr_value AttrValue , eg_real Growth = EX_LEVEL_GROWTH )
{
	return EGMath_round( BaseMultiplier * EGMath_pow( Growth , static_cast<eg_real>(EG_Max<ex_attr_value>(0,AttrValue)) ));
}

egreflect enum class ex_reveal_speed
{
	Slow,
	Normal,
	Fast,
	SuperFast,
};

egreflect enum class ex_move_animation
{
	Smooth ,
	HalfStep ,
	None ,
};

egreflect enum class ex_move_animation_context
{
	Walking ,
	Turning ,
	Looking ,
	Door ,
};

enum class ex_combat_result : eg_uint8
{
	Defeat,
	Victory,
	Fled,
};

enum class ex_combat_log_verbosity : eg_uint8
{
	Minimal,
	Verbose,
};

enum class ex_rest_t : eg_uint8
{
	UntilHealed,
	UntilMorning,
};

egreflect enum class ex_map_info_map_t
{
	Dungeon,
	Town,
};

enum class ex_encounter_disposition
{
	Default,
	FirstProbabilitySet,
};

struct exPackedCreateGameInfo
{
	union
	{
		eg_int64 AsEventParm;
		struct 
		{
			eg_uint Slot:8;
		};
	};
};

enum ex_notify_t : eg_uint
{
	NOTIFY_UNK,
	NOTIFY_ACTIVATE,
	NOTIFY_TURN_START,
	NOTIFY_TURN_END,
};

enum class ex_face: eg_uint
{
	UNK,
	NORTH,
	EAST,
	SOUTH,
	WEST,
	ALL,
	NONE,
};

class ExGameVarSearchMap : public EGStringCrcMapFixedSize<egsm_var,MAX_GAME_VARS>
{
public:
	ExGameVarSearchMap( eg_ctor_t Ct ): EGStringCrcMapFixedSize( Ct , egsm_var(CT_Clear) ) { }
};

egreflect enum class ex_attack_t
{
	NONE ,
	MELEE ,
	RANGED ,
};

enum class ex_item_slot
{
	#define ITEM_SLOT( _var_ , _name_ , _desc_ ) _var_,
	#include "ExItemSlots.items"
	COUNT,
};

egreflect enum class ex_equip_m
{
	NONE ,
	SINGLE_SLOT , // For items with more than one slot, the item only uses one of them when equipped.
	ALL_SLOTS ,   // For items with more than one slot, the item occupies all slots when equipped (two-handed swords).
};

egreflect enum class ex_scale_func
{
	// New, good values
	Fixed,
	ScaledByItemLevel,
	MultipliedByItemLevel,
	ScaledByMonsterLevel,
	ScaledByCharacterLevel,
	ScaledByCharacterSTR,
	ScaledByCharacterMAG,
	ScaledByCharacterEND,
	ScaledByCharacterSPD,
};

enum class ex_can_buy_t
{
	CanBuy,
	TooExpensive,
	BagsFull,
};

enum class ex_character_help_text_t
{
	Portrait,
	Name,
	CreateCharacter,
};

struct exScaleFuncAttrs
{
	ex_attr_value LVL = 1;
	ex_attr_value STR = 0;
	ex_attr_value END = 0;
	ex_attr_value MAG = 0;
	ex_attr_value SPD = 0;
};

egreflect struct exRawItem
{
	egprop eg_string_crc ItemId;
	egprop eg_int Level;

	exRawItem() = default;
	exRawItem( eg_ctor_t Ct )
	: ItemId( Ct )
	{
		if( Ct == CT_Clear )
		{
			Level = 0;
		}
		else if( Ct == CT_Default )
		{
			Level = 1;
		}
	}

	eg_bool operator == ( const exRawItem& rhs ) const
	{
		return ItemId == rhs.ItemId && Level == rhs.Level;
	}
};

struct exTreasureChestResults
{
	eg_bool bWasAbleToOpen;
	eg_int GoldAmount;
	EGFixedArray<exRawItem,6> InventoryItems;
	EGFixedArray<eg_string_crc,4> QuestItems;

	exTreasureChestResults() = default;

	exTreasureChestResults( eg_ctor_t Ct )
	: InventoryItems( Ct )
	, QuestItems( Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			Clear();
		}
	}

	void Clear()
	{
		bWasAbleToOpen = false;
		GoldAmount = 0;
		InventoryItems.Clear();
		QuestItems.Clear();
	}

	eg_bool HasContents() const
	{
		return GoldAmount > 0 || InventoryItems.HasItems() || QuestItems.HasItems();
	}
};

egreflect struct exMapTravelTarget
{
	egprop eg_string_crc MapId = CT_Clear;
	egprop eg_string_crc EntranceId = CT_Clear;
};

egreflect struct exAttrModifier
{
	egprop ex_attr_t Attribute;
	egprop eg_real Value;
	egprop ex_scale_func Function = ex_scale_func::Fixed;

	eg_bool operator == ( const exAttrModifier& rhs ) const
	{
		return Attribute == rhs.Attribute && Function == rhs.Function && EG_IsEqualEps( Value , rhs.Value );
	}

	eg_bool operator != ( const exAttrModifier& rhs ) const
	{
		return !(*this == rhs);
	}
};

egreflect struct exLocText
{
	egprop eg_string_crc Text = CT_Clear;
	egprop eg_d_string Text_enus = CT_Clear;
};

egreflect struct exLocTextMultiline
{
	egprop eg_string_crc Text = CT_Clear;
	egprop eg_d_string_ml Text_enus = CT_Clear;
};

egreflect struct exPlayerClassSet
{
	egprop eg_bool bWarrior;
	egprop eg_bool bThief;
	egprop eg_bool bMage;
	egprop eg_bool bCleric;

	exPlayerClassSet() = default;
	exPlayerClassSet( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			bWarrior = false;
			bThief = false;
			bMage = false;
			bCleric = false;
		}
	}

	void GetAllowedClasses( EGArray<eg_string_crc>& Out ) const;
	void GetAllowedClasses( EGArray<ex_class_t>& Out ) const;
	eg_bool IsClassSet( ex_class_t ClassType ) const;
};

egreflect struct exSlotSet
{
	egprop eg_bool bHelm;
	egprop eg_bool bArmor;
	egprop eg_bool bWeapon1;
	egprop eg_bool bWeapon2;
	egprop eg_bool bAmulet;
	egprop eg_bool bRing1;
	egprop eg_bool bRing2;

	exSlotSet() = default;
	exSlotSet( eg_ctor_t Ct )
	{
		if( Ct == CT_Default || Ct == CT_Clear )
		{
			bHelm = false;
			bArmor = false;
			bWeapon1 = false;
			bWeapon2 = false;
			bAmulet = false;
			bRing1 = false;
			bRing2 = false;
		}
	}

	void GetAllowedSlots( EGArray<ex_item_slot>& Out ) const;
	eg_bool IsSlotSet( ex_item_slot SlotType ) const;
	ex_item_slot GetDefaultSlot() const;
	eg_int GetNumSlots() const;
};

struct exMovingPose
{
public:

	static const eg_real DEFAULT_MOVE_DURATION;

private:

	eg_transform Pose = CT_Default;
	eg_transform NextPose = CT_Default;
	eg_transform PrevPose = CT_Default;
	eg_real      MoveTime = 0.f;
	eg_real      MoveSpeed = 1.f/DEFAULT_MOVE_DURATION;
	eg_bool      bIsMoving = false;
	eg_transform RestorePose = CT_Default;
	eg_bool      bWantsRestore = false;
	eg_real      RestoreTimer = 0.f;
	eg_real      RestoreWaitDuration = 1.f/DEFAULT_MOVE_DURATION;

public:

	void Update( eg_real DeltaTime );
	void SetPose( const eg_transform& NewPose );
	void MoveTo( const eg_transform& NewPose , eg_bool bResetTimer );
	void MoveToThenRestore( const eg_transform& InNewPose , const eg_transform& InRestorePose , eg_bool bResetMoveTime );
	const eg_transform& GetPose() const { return Pose; }
	const eg_transform& GetTargetPose() const;
	void SetMoveSpeed( eg_real InMoveTime ) { MoveSpeed = InMoveTime; }
	void SetMoveDuration( eg_real InMoveDuration ) { MoveSpeed = 1.f/InMoveDuration; }
	void SetRestoreWaitDuration( eg_real InRestoreWaitDuration ) { RestoreWaitDuration = InRestoreWaitDuration; }
};

struct exResourceBarTween
{
private:

	EGTween<eg_real> Tween;
	ex_attr_value    LastKnownValue;
	eg_real          SecondsForFullBarAnim = .5f;

public:

	void Update( eg_real DeltaTime , ex_attr_value CurValue , ex_attr_value MaxValue );
	void SetValue( ex_attr_value NewValue );
	ex_attr_value GetValue() const { return LastKnownValue; }
	eg_real GetCurrentValue() const { return Tween.GetCurrentValue(); }
};