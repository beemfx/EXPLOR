// (c) 2017 Beem Media

#include "ExFighter.h"
#include "ExCore.h"
#include "ExPortraits.h"
#include "ExBeastiary.h"
#include "EGLocalize.h"
#include "ExSpellBook.h"
#include "ExArmory.h"
#include "EGPlatform.h"
#include "ExGlobalData.h"

ExFighter::ExFighter( eg_ctor_t Ct ) : PortraitId( Ct )
, m_CombatBoosts( nullptr )
, m_EquippedItems( Ct )
, m_AttunedSkills( Ct )
{
	m_EquippedItems.Resize( m_EquippedItems.ARRAY_SIZE );

	if( CT_Clear == Ct || CT_Default == Ct )
	{
		m_Class = ex_class_t::Unknown;
		m_BeastId = CT_Clear;
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) m_##_var_ = 0;
		#include "ExAttrs.items"

		m_bIsDead = false;
		m_bIsUnconscious = false;
		m_bIsCursed = false;
		m_bIsPetrified = false;

		for( exInventoryItem& Item : m_EquippedItems )
		{
			Item = CT_Clear;
		}

		LocName[0] = '\0';

		if( CT_Default == Ct )
		{
			SetAttrValue( ex_attr_t::LVL , 1 );
		}
	}

	ResolveReplicatedData();
}

eg_bool ExFighter::HasCombatBoost() const
{
	if( m_CombatBoosts )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) if( ex_attr_t::AGR != ex_attr_t::_var_ && m_CombatBoosts->_var_ != 0 ) return true;
		#include "ExAttrs.items"
	}

	return false;
}

void ExFighter::InitAs( ex_class_t Class , eg_bool bForCharacterCreate )
{
	m_Class = Class;
	m_BeastId = CT_Clear;
	m_LVL = 1;
	m_XP = 0;


	if( bForCharacterCreate )
	{

	}
	else
	{
		exClassDefaults Defaults( m_Class );

		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) m_##_var_ =  Defaults.Def##_var_;
		#include "ExAttrs.items"
	}

	ResolveReplicatedData();

	RestoreAllHP();
	RestoreAllMP();
}

void ExFighter::InitAs( const eg_string_crc& ClassId , eg_bool bForCharacterCreate )
{
	const exCharacterClassGlobals& CharacterGlobals = ExGlobalData::Get().GetCharacterClassGlobals();
	const exCharacterClass& ClassData = CharacterGlobals.GetClass( ClassId );

	m_Class = ClassData.ClassEnum;
	m_BeastId = CT_Clear;
	m_LVL = 1;
	m_XP = 0;


	if( bForCharacterCreate )
	{

	}
	else
	{
		exClassDefaults Defaults( ClassId );

		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) m_##_var_ =  Defaults.Def##_var_;
		#include "ExAttrs.items"
	}

	ResolveReplicatedData();

	RestoreAllHP();
	RestoreAllMP();
}

void ExFighter::InitMonster( const struct exBeastInfo& BeastInfo , ex_attr_value Level )
{
	m_Class = BeastInfo.Class;
	m_BeastId = BeastInfo.Id;

	#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) m_##_var_ = BeastInfo.BehaviorData.Attributes.##_var_;
	#include "ExAttrs.items"

	m_XP = 0;
	m_LVL = Level;

	ResolveReplicatedData();

	m_HitPoints = GetAttrValue( ex_attr_t::HP );
	m_MagicPoints = GetAttrValue( ex_attr_t::MP );

	eg_loc_text LocTextLocName;
	EGLocalize_Localize( BeastInfo.Name , LocTextLocName );
	EGString_Copy( LocName , LocTextLocName.GetString() , countof( LocName ) );
}

void ExFighter::InitDefaultItemsAndSkills()
{
	m_AttunedSkills.Clear();
	DeleteEquipment();

	exInventoryItem InitialWeapon1 = CT_Clear;
	exInventoryItem InitialWeapon2 = CT_Clear;

	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();
	const exCharacterClass& ClassData = CharacterClasses.GetClass( ExCore_ClassEnumToClassId( GetClass() ) );

	InitialWeapon1.RawItem.ItemId = ClassData.DefaultRhWeapon;
	InitialWeapon1.RawItem.Level = ClassData.DefaultLhWeaponLevel;
	InitialWeapon2.RawItem.ItemId = ClassData.DefaultLhWeapon;
	InitialWeapon2.RawItem.Level = ClassData.DefaultLhWeaponLevel;

	for( const eg_string_crc& SkillId : ClassData.DefaultAttunedSkills )
	{
		SetSkillAttuned( SkillId , true );
	}

	if( InitialWeapon1.RawItem.ItemId.IsNotNull() )
	{
		InitialWeapon1.ResolvePointers();
		EGArray<exInventoryItem> ItemsRemoved;
		EquipInventoryItem( ex_item_slot::WEAPON1 , InitialWeapon1 , ItemsRemoved );	
	}

	if( InitialWeapon2.RawItem.ItemId.IsNotNull() )
	{
		InitialWeapon2.ResolvePointers();
		EGArray<exInventoryItem> ItemsRemoved;
		EquipInventoryItem( ex_item_slot::WEAPON2 , InitialWeapon2 , ItemsRemoved );	
	}

	ResolveReplicatedData();
}

ex_attr_value ExFighter::GetAttrValue( ex_attr_t Type , ex_fighter_get_attr_t GetType /* = ex_fighter_get_attr_t::FinalValue */ ) const
{
	if( Type == ex_attr_t::XP_ )
	{
		assert( false ); // Must use Get/Set Experience
		return 0;
	}

	ex_attr_value Out = 0;

	eg_bool bWantBase = false;
	eg_bool bWantEquipment = false;
	eg_bool bWantCombatBoosts = false;

	switch( GetType )
	{
	case ex_fighter_get_attr_t::FinalValue:
	case ex_fighter_get_attr_t::BaseValueWithEquipmentAndCombatBoosts:
		bWantBase = true;
		bWantEquipment = true;
		bWantCombatBoosts = true;
		break;
	case ex_fighter_get_attr_t::BaseValue:
		bWantBase = true;
		break;
	case ex_fighter_get_attr_t::BaseValueWithEquipmentBoosts:
		bWantBase = true;
		bWantEquipment = true;
		break;
	case ex_fighter_get_attr_t::EquipmentBoosts:
		bWantEquipment = true;
		break;
	case ex_fighter_get_attr_t::CombatBoosts:
		bWantCombatBoosts = true;
		break;
	}

	if( bWantBase )
	{
		switch( Type )
		{
			#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: Out += m_##_var_; break;
			#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: Out += GetCompAttr_##_var_(); break;
			#include "ExAttrs.items"
		}
	}

	if( bWantEquipment )
	{
		switch( Type )
		{
			#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: Out += m_EquipmentBoosts._var_; break;
			#include "ExAttrs.items"
		}
	}

	if( bWantCombatBoosts )
	{
		if( m_CombatBoosts )
		{
			switch( Type )
			{
				#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: Out += m_CombatBoosts->_var_; break;
				#include "ExAttrs.items"
			}
		}
	}

	Out = EG_Max<ex_attr_value>( Out , 0 );

	if( bWantBase && 0 == Out )
	{
		if( ex_attr_t::SPD == Type || ex_attr_t::STR == Type || ex_attr_t::END == Type )
		{
			Out = 1;
		}
	}

	if( ex_attr_t::MP == Type )
	{
		if( m_Class == ex_class_t::Warrior || m_Class == ex_class_t::Thief )
		{
			return 0;
		}
	}

	return Out;
}

ex_attr_value ExFighter::GetMeleeAttrValue( ex_attr_t Type ) const
{
	ExFighter Temp = *this;
	Temp.ResolveReplicatedData();
	Temp.SetAttackType( ex_attack_t::MELEE );
	ex_attr_value Out = Temp.GetAttrValue( Type );
	return Out;
}

void ExFighter::SetAttrValue( ex_attr_t Type , ex_attr_value NewValue )
{
	if( Type == ex_attr_t::XP_ )
	{
		assert( false ); // Must use Get/Set Experience
		return;
	}

	switch( Type )
	{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: m_##_var_ = NewValue; break;
		#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: assert( false ); break; // Cannot set a computed attribute value
		#include "ExAttrs.items"
	}
}

void ExFighter::SetAttackType( ex_attack_t NewAttackType )
{
	if( m_CurrentAttackType != NewAttackType )
	{
		m_CurrentAttackType = NewAttackType;
		ResolveEquipmentBoosts();
	}
}

exDamage ExFighter::GetDamage() const
{
	assert( m_CurrentAttackType != ex_attack_t::NONE ); // Should set attack type before doing damage.
	
	exDamage Out( CT_Clear );

	Out.Physical = GetAttrValue( ex_attr_t::PDMG );
	Out.Fire = GetAttrValue( ex_attr_t::FDMG );
	Out.Water = GetAttrValue( ex_attr_t::WDMG );
	Out.Earth = GetAttrValue( ex_attr_t::EDMG );
	Out.Air = GetAttrValue( ex_attr_t::ADMG );

	return Out;
}

exDamage ExFighter::GetDefense() const
{
	exDamage Out( CT_Clear );

	Out.Physical = GetAttrValue( ex_attr_t::PDEF );
	Out.Fire = GetAttrValue( ex_attr_t::FDEF );
	Out.Water = GetAttrValue( ex_attr_t::WDEF );
	Out.Earth = GetAttrValue( ex_attr_t::EDEF );
	Out.Air = GetAttrValue( ex_attr_t::ADEF );

	return Out;
}

ex_gender_t ExFighter::GetGender() const
{
	return ExPortraits::Get().FindInfo( PortraitId ).Gender;
}

const exInventoryItem ExFighter::GetItemInSlot( ex_item_slot Slot , eg_bool bActualSlot ) const
{
	exInventoryItem Out = CT_Clear;

	Out = m_EquippedItems.IsValidIndex( static_cast<eg_uint>(Slot) ) ? m_EquippedItems[static_cast<eg_uint>(Slot)] : exInventoryItem( CT_Clear );
	
	if( !bActualSlot && Out.RawItem.ItemId.IsNull() )
	{
		// Search for any item occupying this slot
		for( const exInventoryItem& Item : m_EquippedItems )
		{
			if( Item.ArmoryItem )
			{
				EGArray<ex_item_slot> AllowedSlots;
				Item.ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
				for( ex_item_slot OccupiedSlot : AllowedSlots )
				{
					if( OccupiedSlot == Slot && Item.ArmoryItem->EquipMode == ex_equip_m::ALL_SLOTS )
					{
						Out = Item;
						break;
					}
				}
			}
		}
	}

	return Out;
}

eg_bool ExFighter::IsInventorySlotFull( ex_item_slot Slot ) const
{
	// First check the slot in question:
	eg_size_t SlotAsIndex = static_cast<eg_size_t>(Slot);
	if( m_EquippedItems.IsValidIndex( SlotAsIndex ) )
	{
		if( !m_EquippedItems[SlotAsIndex].RawItem.ItemId.IsNull() )
		{
			return true;
		}
	}

	// Now check every slot and see if the item fills that slot
	for( const exInventoryItem& Item : m_EquippedItems )
	{
		assert(  Item.ArmoryItem ); // Can only be called after resolution of data. (Client only function.)
		if( Item.ArmoryItem )
		{
			if( Item.ArmoryItem->EquipMode == ex_equip_m::ALL_SLOTS )
			{
				EGArray<ex_item_slot> AllowedSlots;
				Item.ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
				for( ex_item_slot EquipToSlot :  AllowedSlots )
				{
					if( EquipToSlot == Slot )
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

eg_bool ExFighter::MeetsRequirementsToEquip( const exInventoryItem& Item ) const
{
	assert( Item.ArmoryItem ); // Must resolve replicated data before calling. (Client only function.)
	if( Item.ArmoryItem )
	{
		EGArray<ex_item_slot> AllowedSlots;
		Item.ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
		for( ex_item_slot Slot : AllowedSlots )
		{
			if( Slot != ex_item_slot::COUNT && Slot != ex_item_slot::NONE && Item.ArmoryItem->CanBeUsedByClass( GetClass() ) )
			{
				// TODOs character level, etc?	
				return true;
			}
		}
	}

	return false;
}

void ExFighter::EquipInventoryItem( ex_item_slot Slot, const exInventoryItem& Item, EGArray<exInventoryItem>& ItemsRemoved )
{
	const eg_bool bWasHPFullBefore = GetHP() >= GetAttrValue( ex_attr_t::HP );
	const eg_bool bWasMPFullBefore = GetMP() >= GetAttrValue( ex_attr_t::MP );

	eg_uint SlotAsIndex = static_cast<eg_uint>(Slot);

	auto RemoveItemInSlot = [this,&ItemsRemoved]( ex_item_slot RemoveSlot ) -> void
	{
		auto RemoveActualItemInSlot = [this,&ItemsRemoved]( ex_item_slot ActualSlot ) -> void
		{
			eg_size_t RemoveSlotAsIndex = static_cast<eg_size_t>(ActualSlot);
			if( m_EquippedItems.IsValidIndex( RemoveSlotAsIndex ) )
			{
				if( !m_EquippedItems[RemoveSlotAsIndex].RawItem.ItemId.IsNull() )
				{
					ItemsRemoved.Append( m_EquippedItems[RemoveSlotAsIndex] );
				}
				m_EquippedItems[RemoveSlotAsIndex] = CT_Clear;
			}
		};

		// First check the slot in question:
		RemoveActualItemInSlot( RemoveSlot );

		// Now check every slot and see if the item fills that slot
		for( const exInventoryItem& EquippedItem : m_EquippedItems )
		{
			if( EquippedItem.RawItem.ItemId.IsNotNull() )
			{
				assert( EquippedItem.ArmoryItem );
				if( EquippedItem.ArmoryItem && EquippedItem.ArmoryItem->EquipMode == ex_equip_m::ALL_SLOTS )
				{
					if( EquippedItem.ArmoryItem->AlwaysOccupiesSlot( RemoveSlot ) )
					{
						EGArray<ex_item_slot> AllowedSlots;
						EquippedItem.ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
						for( ex_item_slot SlotOccupied : AllowedSlots )
						{
							RemoveActualItemInSlot( SlotOccupied );
						}
					}
				}
			}
		}
	};

	ResolveReplicatedData();

	assert( Item.RawItem.ItemId.IsNull() || Item.ArmoryItem );
	if( Item.RawItem.ItemId.IsNotNull() && Item.ArmoryItem && Item.ArmoryItem->EquipMode == ex_equip_m::ALL_SLOTS )
	{
		RemoveItemInSlot( Slot );

		EGArray<ex_item_slot> AllowedSlots;
		Item.ArmoryItem->EquipSlots.GetAllowedSlots( AllowedSlots );
		for( ex_item_slot EquipToSlot : AllowedSlots )
		{
			RemoveItemInSlot( EquipToSlot );
		}
	}
	else
	{
		RemoveItemInSlot( Slot );
	}

	if( Item.RawItem.ItemId.IsNotNull() && m_EquippedItems.IsValidIndex( SlotAsIndex ) )
	{
		m_EquippedItems[SlotAsIndex] = Item;
		m_EquippedItems[SlotAsIndex].ResolvePointers();
	}

	ResolveEquipmentBoosts();

	if( bWasHPFullBefore )
	{
		RestoreAllHP();
	}

	if( bWasMPFullBefore )
	{
		RestoreAllMP();
	}
}

void ExFighter::SetSkillAttuned( eg_string_crc SkillId, eg_bool bAttuned )
{	
	if( bAttuned )
	{
		m_AttunedSkills.AppendUnique( SkillId );
	}
	else
	{
		if( m_AttunedSkills.Contains( SkillId ) )
		{
			m_AttunedSkills.DeleteByItem( SkillId );
		}
	}
}

eg_bool ExFighter::IsSkillAttuned( eg_string_crc SkillId, eg_size_t* AttunedSlotOut ) const
{
	for( eg_size_t i=0; i<m_AttunedSkills.Len(); i++ )
	{
		if( m_AttunedSkills[i] == SkillId )
		{
			if( AttunedSlotOut )
			{
				*AttunedSlotOut = i;
			}
			return true;
		}
	}
	return false;
}

eg_bool ExFighter::IsAttunedSkillsFull() const
{
	return m_AttunedSkills.Len() >= EX_MAX_ATTUNED_SKILLS;
}

void ExFighter::ResolveReplicatedData()
{
	for( exInventoryItem& EquippedItem : m_EquippedItems )
	{
		EquippedItem.ResolvePointers();
	}

	m_BeastInfo = nullptr;
	if( m_BeastId.IsNotNull() )
	{
		m_BeastInfo = &ExBeastiary::Get().FindInfo( m_BeastId );
	}

	ResolveEquipmentBoosts();
}

void ExFighter::ResolveEquipmentBoosts()
{
	m_EquipmentBoosts.ZeroAll();

	static const ex_item_slot NonStackingSlots[] =
	{
		ex_item_slot::RING1 ,
		ex_item_slot::RING2 ,
		ex_item_slot::AMULET ,
	};

	static const ex_item_slot StackingSlots[] =
	{
		ex_item_slot::ARMOR ,
		ex_item_slot::HELM ,
		ex_item_slot::WEAPON1 ,
		ex_item_slot::WEAPON2 ,
	};

	static_assert( (countof(NonStackingSlots) + countof(StackingSlots)) == (static_cast<eg_size_t>(ex_item_slot::COUNT)-1) , "Need to account for all item slots" );

	auto ApplyBoostsTo = [this]( exAttrSet& Set , ex_attack_t AttackType ) -> void
	{
		exScaleFuncAttrs ScaleAttrs;
		ScaleAttrs.LVL = GetAttrValue( ex_attr_t::LVL );
		ScaleAttrs.STR = GetAttrValue( ex_attr_t::STR );
		ScaleAttrs.END = GetAttrValue( ex_attr_t::END );
		ScaleAttrs.MAG = GetAttrValue( ex_attr_t::MAG );
		ScaleAttrs.SPD = GetAttrValue( ex_attr_t::SPD );

		// Apply non-stacking boosts...
		for( const ex_item_slot Slot : NonStackingSlots )
		{
			const eg_size_t ItemIndex = static_cast<eg_size_t>(Slot);
			if( m_EquippedItems.IsValidIndex( ItemIndex ) )
			{
				const exInventoryItem& EquippedItem = m_EquippedItems[ItemIndex];
				if( EquippedItem.RawItem.ItemId.IsNotNull() )
				{
					const eg_bool bCanApply = EquippedItem.ArmoryItem && (EquippedItem.ArmoryItem->AttackType == ex_attack_t::NONE || EquippedItem.ArmoryItem->AttackType == AttackType);
					if( bCanApply )
					{
						Set += EquippedItem.GetAttrBoost( ScaleAttrs );
					}
				}
			}
		}

		// Apply stacking boosts (need to get attributes for every item)
		for( const ex_item_slot Slot : StackingSlots )
		{
			const eg_size_t ItemIndex = static_cast<eg_size_t>(Slot);
			if( m_EquippedItems.IsValidIndex( ItemIndex ) )
			{
				const exInventoryItem& EquippedItem = m_EquippedItems[ItemIndex];
				if( EquippedItem.RawItem.ItemId.IsNotNull() )
				{
					const eg_bool bCanApply = EquippedItem.ArmoryItem && (EquippedItem.ArmoryItem->AttackType == ex_attack_t::NONE || EquippedItem.ArmoryItem->AttackType == AttackType);
					if( bCanApply )
					{
						ScaleAttrs.LVL = GetAttrValue( ex_attr_t::LVL );
						ScaleAttrs.STR = GetAttrValue( ex_attr_t::STR );
						ScaleAttrs.END = GetAttrValue( ex_attr_t::END );
						ScaleAttrs.MAG = GetAttrValue( ex_attr_t::MAG );
						ScaleAttrs.SPD = GetAttrValue( ex_attr_t::SPD );

						Set += EquippedItem.GetAttrBoost( ScaleAttrs );
					}
				}
			}
		}

		if( m_BeastInfo )
		{
			Set += ExCore_GetModiferBoost( m_BeastInfo->BehaviorData.CombatModifiers , ScaleAttrs , 0 );
		}
	};

	ApplyBoostsTo( m_EquipmentBoosts , m_CurrentAttackType );

	if( m_HitPoints > GetAttrValue( ex_attr_t::HP ) )
	{
		m_HitPoints = GetAttrValue( ex_attr_t::HP );
	}
	if( m_MagicPoints > GetAttrValue( ex_attr_t::MP ) )
	{
		m_MagicPoints = GetAttrValue( ex_attr_t::MP );
	}
}

void ExFighter::ClearPointers()
{
	for( exInventoryItem& EquippedItem : m_EquippedItems )
	{
		EquippedItem.ArmoryItem = nullptr;
	}

	m_CombatBoosts = nullptr;
}

void ExFighter::DeleteEquipment()
{
	for( exInventoryItem& EquippedItem : m_EquippedItems )
	{
		EquippedItem = CT_Clear;
	}
}

eg_string_crc ExFighter::GetStatusText() const
{
	if( IsDead() )
	{
		return eg_loc("DeadText","Dead");
	}
	else if( IsPetrified() )
	{
		return eg_loc("PetrifiedText","Petrified");
	}
	else if( IsUnconscious() )
	{
		return eg_loc("UnconsciousText","Unconscious");
	}
	else if( IsCursed() )
	{
		return eg_loc("CursedText","Cursed");
	}
	return CT_Clear;
}

ex_target_strategy ExFighter::GetTargetStrategy() const
{
	if( m_BeastId.IsNotNull() )
	{
		assert( nullptr != m_BeastInfo ); // Should have resolved...
		if( m_BeastInfo )
		{
			return m_BeastInfo->BehaviorData.TargetStrategy;
		}
	}

	return ex_target_strategy::Default;
}

eg_bool ExFighter::HasRangedAttack() const
{
	if( m_BeastId.IsNotNull() )
	{
		assert( nullptr != m_BeastInfo ); // Should have resolved...
		if( m_BeastInfo && m_BeastInfo->BehaviorData.bHasRangedAttack )
		{
			return true;
		}
	}
	
	for( const exInventoryItem& EquippedItem : m_EquippedItems )
	{
		if( EquippedItem.RawItem.ItemId.IsNotNull() )
		{
			assert( nullptr != EquippedItem.ArmoryItem );
			if( EquippedItem.ArmoryItem && EquippedItem.ArmoryItem->AttackType == ex_attack_t::RANGED )
			{
				return true;
			}
		}
	}

	return false;
}

ex_attr_value ExFighter::GetCompAttr_DMG_( void )const
{
	return GetAttrValue( ex_attr_t::PDMG ) + GetAttrValue( ex_attr_t::FDMG ) + GetAttrValue( ex_attr_t::WDMG ) + GetAttrValue( ex_attr_t::EDMG ) + GetAttrValue( ex_attr_t::ADMG );
}

ex_attr_value ExFighter::GetCompAttr_DEF_( void )const
{
	return GetAttrValue( ex_attr_t::PDEF ) + GetAttrValue( ex_attr_t::FDEF ) + GetAttrValue( ex_attr_t::WDEF ) + GetAttrValue( ex_attr_t::EDEF ) + GetAttrValue( ex_attr_t::ADEF );
}

ex_attr_value ExFighter::GetCompAttr_PDMG( void )const
{
	return ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::STR ) , GetAttrValue( ex_attr_t::LVL ) );
}

ex_attr_value ExFighter::GetCompAttr_FDMG( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_WDMG( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_EDMG( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_ADMG( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_PDEF( void )const
{
	return ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::END ) , GetAttrValue( ex_attr_t::LVL ) );
}

ex_attr_value ExFighter::GetCompAttr_FDEF( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_WDEF( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_EDEF( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_ADEF( void )const
{
	return 0;
}

ex_attr_value ExFighter::GetCompAttr_HP( void )const
{
	const ex_attr_value Lvl = GetAttrValue( ex_attr_t::LVL );
	return EG_Max<ex_attr_value>( 1 , ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::BHP ) , Lvl ) +  ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::END ) , Lvl ) );
}

ex_attr_value ExFighter::GetCompAttr_MP( void )const
{
	const ex_attr_value Lvl = GetAttrValue( ex_attr_t::LVL );
	return ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::BMP ) , Lvl ) + ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::MAG ) , Lvl );
}

ex_attr_value ExFighter::GetCompAttr_HIT( void )const
{
	ex_attr_value Chance = ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::ACC ) , GetAttrValue( ex_attr_t::LVL ) );
	Chance = EG_Clamp<ex_attr_value>( Chance , 0 , 100 );
	return Chance;
}

ex_attr_value ExFighter::GetCompAttr_DG( void )const
{
	ex_attr_value Chance = ExCore_GetRampedValue_Level( GetAttrValue( ex_attr_t::SPD ) , GetAttrValue( ex_attr_t::LVL ) );
	Chance = EG_Clamp<ex_attr_value>( Chance , 0 , 100 );
	return Chance;
}

ex_attr_value ExFighter::GetCompAttr_ATK( void )const
{
	return 1 + (m_LVL/4);
}


eg_bool ExFighter::CanTrainToNextLevel() const
{
	ex_attr_value Level = GetAttrValue( ex_attr_t::LVL );
	return Level < EX_MAX_LEVEL && GetXP() >= ExCore_GetXpNeededForLevel( Level + 1 );
}

eg_bool ExFighter::IsInConditionToTrainToNextLevel() const
{
	if( IsDead() || IsUnconscious() || IsPetrified() || IsCursed() )
	{
		return false;
	}

	return true;
}

void ExFighter::TrainToNextLevel()
{
	if( CanTrainToNextLevel() ) { m_LVL++; RestoreAllHP(); RestoreAllMP(); }
	else { assert( false ); /* Can't train yet.*/ }

	ResolveEquipmentBoosts();
}

eg_bool ExFighter::CanFight() const
{
	if( GetHP() <= 0 )
	{
		return false;
	}
	else if( IsDead() )
	{
		return false;
	}
	else if( IsUnconscious() )
	{
		return false;
	}
	else if( IsPetrified() )
	{
		return false;
	}

	return true;
}

void ExFighter::FormatTextInternal( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const
{
	eg_string_crc BaseFlag = Formatter->GetNextFlag( &Flags );

	switch_crc( BaseFlag )
	{
		case_crc("NAME"):
		{
			Formatter->SetText( LocName );
		} break;
		case_crc("CLASS"):
		{
			Formatter->SetText( eg_loc_text( ExCore_GetClassName( GetClass() ) ).GetString() );
		} break;
		case_crc("GENDER"):
		{
			Formatter->SetText( eg_loc_text( ExCore_GetGenderName( GetGender() ) ).GetString() );
		} break;
		case_crc("EQUIPMENT"):
		{
			eg_loc_text Text;

			EGFixedArray<eg_int,enum_count(ex_item_slot)> AlreadyCounted;

			for( eg_int ItemIdx = 0; ItemIdx<m_EquippedItems.LenAs<eg_int>(); ItemIdx++ )
			{
				if( !AlreadyCounted.Contains( ItemIdx ) )
				{
					if( m_EquippedItems[ItemIdx].RawItem.ItemId.IsNotNull() )
					{
						eg_int Count = 1;
						for( eg_int CountIdx = ItemIdx+1; CountIdx<m_EquippedItems.LenAs<eg_int>(); CountIdx++ )
						{
							if( m_EquippedItems[CountIdx] == m_EquippedItems[ItemIdx] )
							{
								Count++;
								AlreadyCounted.Append( CountIdx );
							}
						}

						if( Text.GetLen() > 0 )
						{
							Text = EGFormat( L"{0}\n" , Text );
						}

						if( Count > 1 )
						{
							Text = EGFormat( L"{0}{1:NAME} ({2}X)" , Text , &m_EquippedItems[ItemIdx] , Count );
						}
						else
						{
							Text = EGFormat( L"{0}{1:NAME}" , Text , &m_EquippedItems[ItemIdx] );
						}
					}
				}
			}

			Formatter->SetText( Text.GetString() );
		} break;
		case_crc("ATTR"):
		{
			eg_cpstr16 DamageFormatStr = L"|SC(EX_PDMG)|{0}|RC()|/|SC(EX_FDMG)|{1}|RC()|/|SC(EX_WDMG)|{2}|RC()|/|SC(EX_EDMG)|{3}|RC()|/|SC(EX_ADMG)|{4}|RC()|";

			eg_string_crc AttrFlag = Formatter->GetNextFlag( &Flags );
			ex_attr_t AttrType = ExCore_CrcToAttrType( AttrFlag );
			eg_string_crc AttrParm = Formatter->GetNextFlag( &Flags );
			ex_attr_value AttrValue = GetAttrValue( AttrType );
			if( AttrParm == eg_crc("DETAIL") )
			{
				if( AttrType == ex_attr_t::HP )
				{
					Formatter->SetText( EGFormat(L"{0}/{1}" , GetHP() , AttrValue ).GetString() );
				}
				else if( AttrType == ex_attr_t::MP )
				{
					Formatter->SetText( EGFormat(L"{0}/{1}" , GetMP() , AttrValue ).GetString() );
				}
				else if( AttrType == ex_attr_t::DMG_ )
				{
					Formatter->SetText( EGFormat( DamageFormatStr , GetAttrValue( ex_attr_t::PDMG ) , GetAttrValue( ex_attr_t::FDMG )  , GetAttrValue( ex_attr_t::WDMG )  , GetAttrValue( ex_attr_t::EDMG ) , GetAttrValue( ex_attr_t::ADMG ) ).GetString() );
				}
				else if( AttrType == ex_attr_t::DEF_ )
				{
					Formatter->SetText( EGFormat( DamageFormatStr , GetAttrValue( ex_attr_t::PDEF ) , GetAttrValue( ex_attr_t::FDEF )  , GetAttrValue( ex_attr_t::WDEF )  , GetAttrValue( ex_attr_t::EDEF ) , GetAttrValue( ex_attr_t::ADEF ) ).GetString() );
				}
				else
				{
					Formatter->SetNumber( AttrValue );
				}
			}
			else
			{
				Formatter->SetNumber( AttrValue );
			}
		} break;
		case_crc("XP"):
		{
			Formatter->SetBigNumber( GetXP() );
		} break;
		break;
	}
}

ExFighter::operator eg_loc_parm() const
{
	assert( EGPlatform_IsMainThread() );

	static ExFighterTextHander FighterHandler( nullptr ); // Should only ever be called on client thread so this is okay.
	
	FighterHandler = ExFighterTextHander( this );

	return &FighterHandler;
}

void ExFighterTextHander::FormatText( eg_cpstr Flags , EGTextParmFormatter* Formatter ) const
{
	if( m_Fighter )
	{
		m_Fighter->FormatTextInternal( Flags , Formatter );
	}
}