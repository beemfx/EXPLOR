//
// ExCore - Core game items
// (c) 2016 Beem Media
//
#include "ExCore.h"
#include "ExBeastiary.h"
#include "EGRandom.h"
#include "EGLocalize.h"
#include "ExSpellBook.h"
#include "ExPortraits.h"
#include "ExCharacterClass.h"
#include "ExGlobalData.h"
#include "ExGameSettings.h"

ex_class_t ExCore_StringToClass( eg_cpstr Str )
{
	eg_string AsString( Str );
	AsString.ConvertToUpper();
	eg_string_crc AsCrc( Str );

	return ExCore_CrcToClass( AsCrc );
}

ex_class_t ExCore_CrcToClass( eg_string_crc Crc )
{
	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

	for( const exCharacterClass& ChClass : CharacterClasses.Classes )
	{
		if( ChClass.ClassCrcId == Crc )
		{
			return ChClass.ClassEnum;
		}
	}

	return ex_class_t::Unknown;
}

eg_string_crc ExCore_GetClassName( ex_class_t Class )
{
	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

	for( const exCharacterClass& ChClass : CharacterClasses.Classes )
	{
		if( ChClass.ClassEnum == Class )
		{
			return ChClass.Name;
		}
	}

	return eg_loc("UnkClassOut","-Unknown Class-");
}

eg_string_crc ExCore_GetClassDesc( ex_class_t Class )
{
	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

	for( const exCharacterClass& ChClass : CharacterClasses.Classes )
	{
		if( ChClass.ClassEnum == Class )
		{
			return ChClass.Description;
		}
	}

	return eg_loc("UnkClassDescOut","-Unknown Class Description-");
}

eg_string_crc ExCore_ClassEnumToClassId( ex_class_t Class )
{
	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

	for( const exCharacterClass& ChClass : CharacterClasses.Classes )
	{
		if( ChClass.ClassEnum == Class )
		{
			return ChClass.ClassCrcId;
		}
	}

	return eg_crc("Unknown");
}

eg_string_crc ExCore_GetGenderName( ex_gender_t Gender )
{
	eg_string_crc Out = eg_loc("GenderNameUnk","Unknown");
	switch( Gender )
	{
	case ex_gender_t::Unknown:
		break;
	case ex_gender_t::Male:
		Out = eg_loc("GenderNameMale","Male");
		break;
	case ex_gender_t::Female:
		Out = eg_loc("GenderNameFemale","Female");
		break;
	}
	return Out;
}

eg_string_crc ExCore_GetAttributeName( ex_attr_t Type )
{
	switch( Type )
	{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _aname_;
		#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _aname_;
		#include "ExAttrs.items"
	}

	return eg_loc("AttrNameUnknown","Unknown");
}

eg_string_crc ExCore_GetAttributeLongName( ex_attr_t Type )
{
	switch( Type )
	{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _lname_;
		#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _lname_;
		#include "ExAttrs.items"
	}

	return eg_loc("AttrLongNameUnknown","Unknown");
}

eg_string_crc ExCore_GetAttributeDesc( ex_attr_t Type )
{
	switch( Type )
	{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _desc_;
		#define COMP_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _desc_;
		#include "ExAttrs.items"
	}

	return eg_loc("AttrDescUnknown","Unknown");
}

eg_cpstr16 ExCore_GetAttributeNameFormatColor( ex_attr_t Type )
{
	switch( Type )
	{
		case ex_attr_t::PDEF:
		case ex_attr_t::PDMG: return L"EX_ATTR";
		case ex_attr_t::FDEF:
		case ex_attr_t::FDMG: return L"EX_FDMG";
		case ex_attr_t::WDEF:
		case ex_attr_t::WDMG: return L"EX_WDMG";
		case ex_attr_t::EDEF:
		case ex_attr_t::EDMG: return L"EX_EDMG";
		case ex_attr_t::ADEF:
		case ex_attr_t::ADMG: return L"EX_ADMG";
		default:
			break;
	}
	return L"EX_ATTR";
}

eg_cpstr16 ExCore_GetAttributeValueFormatColor( ex_attr_t Type )
{
	switch( Type )
	{
	case ex_attr_t::PDEF:
	case ex_attr_t::PDMG: return L"EX_PDMG";
	case ex_attr_t::FDEF:
	case ex_attr_t::FDMG: return L"EX_FDMG";
	case ex_attr_t::WDEF:
	case ex_attr_t::WDMG: return L"EX_WDMG";
	case ex_attr_t::EDEF:
	case ex_attr_t::EDMG: return L"EX_EDMG";
	case ex_attr_t::ADEF:
	case ex_attr_t::ADMG: return L"EX_ADMG";
	default:
		break;
	}
	return L"EX_ATTRV";
}

ex_xp_value ExCore_GetXpNeededForLevel( ex_attr_value Level )
{
	// TODO: Put these in a table:
	ex_xp_value XpNeeded = 0;
	for( ex_attr_value i = 1; i < Level; i++ )
	{
		XpNeeded += static_cast<ex_xp_value>( EGMath_round( FIRST_LEVEL_UP_XP*EGMath_pow( static_cast<eg_real64>(EX_XP_GROWTH) , static_cast<eg_real64>(EG_Max<ex_attr_value>( 0 , i-1)) ) ) );
	}
	return XpNeeded;
}

ex_attr_value ExCore_GetMaxLevelToTrainTo( ex_xp_value XpValue )
{
	ex_attr_value Out = 0;

	for( ex_attr_value CompareLvl = 1; CompareLvl <= EX_MAX_LEVEL; CompareLvl++ )
	{
		const ex_xp_value CompareXpNeeded = ExCore_GetXpNeededForLevel( CompareLvl );
		if( XpValue >= CompareXpNeeded )
		{
			Out = CompareLvl;
		}
	}

	return Out;
}

ex_attr_value ExCore_GetCostToTrainToLevel( ex_attr_value Level )
{
	if( Level <= 1 )
	{
		return 0;
	}
	return ExCore_GetRampedValue_Level( EG_To<ex_attr_value>(EX_BASE_LEVEL_UP_COST) , Level , EX_GOLD_GROWTH );
}

ex_attr_t ExCore_CrcToAttrType( eg_string_crc Crc )
{
	switch_crc( Crc )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case_crc(#_var_): return ex_attr_t::_var_;
		#include "ExAttrs.items"
	}

	assert( false ); // Invalid CRC
	return ex_attr_t::XP_;
}

ex_attack_t ExCore_StrToAttackType( eg_cpstr Str )
{
	eg_string Convert( Str );
	Convert.ConvertToUpper();
	eg_string_crc AsCrc( Convert );

	switch_crc( AsCrc )
	{
		case_crc(""):
		case_crc("NONE"): return ex_attack_t::NONE;
		case_crc("MELEE"): return ex_attack_t::MELEE;
		case_crc("RANGED"): return ex_attack_t::RANGED;
	}

	assert( false );
	return ex_attack_t::NONE;
}

ex_item_slot ExCore_StrToItemSlot( eg_cpstr Str )
{
	eg_string Convert( Str );
	Convert.ConvertToUpper();
	eg_string_crc AsCrc( Convert );

	switch_crc( AsCrc )
	{
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) case_crc( #_var_ ): return ex_item_slot::_var_;
		#include "ExItemSlots.items"
	}

	assert( false );
	return ex_item_slot::NONE;
}

eg_string_crc ExCore_GetItemSlotName( ex_item_slot Slot )
{
	switch( Slot )
	{
		#define ITEM_SLOT( _var_ , _name_ , _desc_ ) case ex_item_slot::_var_: return _name_;
		#include "ExItemSlots.items"
		case ex_item_slot::COUNT:
			break;
	}
	return CT_Clear;
}

ex_equip_m ExCore_StrToEquipMode( eg_cpstr Str )
{
	eg_string Convert( Str );
	Convert.ConvertToUpper();
	eg_string_crc AsCrc( Convert );

	switch_crc( AsCrc )
	{
		case_crc(""):
		case_crc("NONE"): return ex_equip_m::NONE;
		case_crc("SINGLE_SLOT"): return ex_equip_m::SINGLE_SLOT;
		case_crc("ALL_SLOTS"): return ex_equip_m::ALL_SLOTS;
	}

	assert( false );
	return ex_equip_m::NONE;
}

void ExCore_SlotStrToArray( eg_cpstr Str, EGArray<ex_item_slot>& Out )
{
	eg_string FindingString;
	eg_string SlotString = Str;
	for( eg_size_t i=0; i<SlotString.Len(); i++ )
	{
		eg_char c = SlotString[i];
		if( c == ';' )
		{
			ex_item_slot Slot = ExCore_StrToItemSlot( FindingString );
			Out.Append( Slot );
			FindingString.Clear();
		}
		else
		{
			FindingString.Append( c );
		}
	}

	if( FindingString.Len() > 0 )
	{
		ex_item_slot Slot = ExCore_StrToItemSlot( FindingString );
		Out.Append( Slot );
	}
}

exAttrSet ExCore_GetModiferBoost( const EGArray<exAttrModifier>& InModifiers , const exScaleFuncAttrs& Attrs , eg_int ItemLevel )
{
	exAttrSet Out( CT_Clear );

	auto HandleAttributeType = [&InModifiers,&Attrs,&ItemLevel]( eg_int& OutValue , ex_attr_t AttrType ) -> void
	{
		for( const exAttrModifier& Modifier : InModifiers )
		{
			if( Modifier.Attribute == AttrType )
			{
				OutValue += ExCore_GetScaledValue( Modifier.Value , Modifier.Function , ItemLevel , Attrs );
			}
		}
	};

	#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) HandleAttributeType( Out._var_ , ex_attr_t::_var_ );
	#include "ExAttrs.items"

	return Out;
}

ex_attr_value ExCore_GetScaledValue( eg_real BaseValue , ex_scale_func Function , ex_attr_value ItemLevel , const exScaleFuncAttrs& CharacterAttrs )
{
	assert( CharacterAttrs.LVL > 0 ); // No level 0 character should exist.

	switch( Function )
	{
		case ex_scale_func::Fixed: return EGMath_round( BaseValue );
		case ex_scale_func::ScaledByItemLevel:
			assert( ItemLevel > 0 ); // Scaling by item level on a monster modifier?
			return ExCore_GetRampedValue_Level( BaseValue , ItemLevel , EX_ITEM_LEVEL_GROWTH );
		case ex_scale_func::MultipliedByItemLevel:
			assert( ItemLevel > 0 );
			return EGMath_round( BaseValue * ItemLevel );
			break;
		case ex_scale_func::ScaledByMonsterLevel:
		case ex_scale_func::ScaledByCharacterLevel: return ExCore_GetRampedValue_Level( BaseValue , CharacterAttrs.LVL );
		case ex_scale_func::ScaledByCharacterSTR: return ExCore_GetRampedValue_Attr( BaseValue , CharacterAttrs.STR );
		case ex_scale_func::ScaledByCharacterMAG: return ExCore_GetRampedValue_Attr( BaseValue , CharacterAttrs.MAG );
		case ex_scale_func::ScaledByCharacterEND: return ExCore_GetRampedValue_Attr( BaseValue , CharacterAttrs.END );
		case ex_scale_func::ScaledByCharacterSPD: return ExCore_GetRampedValue_Attr( BaseValue , CharacterAttrs.SPD );
		default:
			assert( false); // Old function, should not be used (or new one that wasn't implemented?)
			break;
	}

	return 0;
}

eg_string_crc ExCore_GetHelpText( ex_character_help_text_t Type )
{	
	switch( Type )
	{
		case ex_character_help_text_t::Portrait: return eg_loc("CoreHelpPortrait","Choose the appearance of this character. Appearance has no effect on the abilities of the character.");
		case ex_character_help_text_t::Name: return eg_loc("CoreHelpTextName","Name this character. A name speaks to who one is, what their family thought of them, and perhaps what was hoped or expected of them in this life.");
		case ex_character_help_text_t::CreateCharacter: return eg_loc("CoreCreateCharacterHelpText","Create this character.");
	}

	return CT_Clear;
}

eg_real ExCore_GetModifiedAnimationProgress( eg_real ValueIn , ex_move_animation_context Context )
{
	auto GetHalfStepValue = [&ValueIn]() -> eg_real
	{
		eg_real Out = ValueIn;

		if( ValueIn < EG_SMALL_NUMBER )
		{
			Out = 0.f;
		}
		else if( ValueIn < (1.f-EG_SMALL_NUMBER ) )
		{
			if( ValueIn < .5f )
			{
				Out = .5f;
			}
			else
			{
				Out = 1.f;
			}
		}
		else
		{
			Out = 1.f;
		}

		return Out;
	};

	auto GetFullStepValue = [&ValueIn]() -> eg_real
	{
		return ValueIn < EG_SMALL_NUMBER ? 0.f : 1.f;
	};

	eg_real ValueOut = ValueIn;

	ex_move_animation AnimType = ex_move_animation::Smooth;

	switch( Context )
	{
	case ex_move_animation_context::Walking:
	case ex_move_animation_context::Door:
		AnimType = ExGameSettings_WalkAnimation.GetValue();
		if( AnimType != ex_move_animation::Smooth && Context == ex_move_animation_context::Door )
		{
			AnimType = ex_move_animation::HalfStep;
		}
		break;
	case ex_move_animation_context::Turning:
	case ex_move_animation_context::Looking:
		AnimType = ExGameSettings_TurnAnimation.GetValue();
		break;
	}

	switch( AnimType )
	{
	case ex_move_animation::Smooth:
		// Nothing to do for smooth
		break;
	case ex_move_animation::HalfStep:
	{
		ValueOut = GetHalfStepValue();
	} break;
	case ex_move_animation::None:
		ValueOut = GetFullStepValue();
		break;

	}

	return ValueOut;
}

//
// Class Defaults:
//

exClassDefaults::exClassDefaults( ex_class_t Class )
: exClassDefaults( CT_Default )
{
	if( ExGlobalData::IsInitialized() )
	{
		const eg_string_crc ClassCrcId = ExCore_ClassEnumToClassId( Class );
		const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

		for( const exCharacterClass& ChClass : CharacterClasses.Classes )
		{
			if( ChClass.ClassCrcId == ClassCrcId )
			{
				#define COPY_ATTR( _var_ ) Def##_var_ = ChClass.DefaultAttributes._var_; Min##_var_ = ChClass.CreationMinAttributes._var_; Max##_var_ = ChClass.CreationMaxAttributes._var_
				COPY_ATTR( SPD );
				COPY_ATTR( STR );
				COPY_ATTR( END );
				COPY_ATTR( ACC );
				COPY_ATTR( MAG );
				COPY_ATTR( AGR );
				COPY_ATTR( BHP );
				COPY_ATTR( BMP );
				#undef COPY_ATTR
			}
		}
	}
}

exClassDefaults::exClassDefaults( const eg_string_crc& ClassId )
: exClassDefaults( CT_Default )
{
	const exCharacterClassGlobals& CharacterClasses = ExGlobalData::Get().GetCharacterClassGlobals();

	for( const exCharacterClass& ChClass : CharacterClasses.Classes )
	{
		if( ChClass.ClassCrcId == ClassId )
		{
			#define COPY_ATTR( _var_ ) Def##_var_ = ChClass.DefaultAttributes._var_; Min##_var_ = ChClass.CreationMinAttributes._var_; Max##_var_ = ChClass.CreationMaxAttributes._var_
			COPY_ATTR( SPD );
			COPY_ATTR( STR );
			COPY_ATTR( END );
			COPY_ATTR( ACC );
			COPY_ATTR( MAG );
			COPY_ATTR( AGR );
			COPY_ATTR( BHP );
			COPY_ATTR( BMP );
			#undef COPY_ATTR
		}
	}
}
