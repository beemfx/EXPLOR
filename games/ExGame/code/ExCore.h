//
// ExCore - Core game items
// (c) 2016 Beem Media
//
#pragma once

#include "ExGameTypes.h"

ex_class_t ExCore_StringToClass( eg_cpstr Str );
ex_class_t ExCore_CrcToClass( eg_string_crc Crc );
eg_string_crc ExCore_GetClassName( ex_class_t Class );
eg_string_crc ExCore_GetClassDesc( ex_class_t Class );
eg_string_crc ExCore_ClassEnumToClassId( ex_class_t Class );

eg_string_crc ExCore_GetGenderName( ex_gender_t Gender );

eg_string_crc ExCore_GetAttributeName( ex_attr_t Type );
eg_string_crc ExCore_GetAttributeLongName( ex_attr_t Type );
eg_string_crc ExCore_GetAttributeDesc( ex_attr_t Type );
eg_cpstr16 ExCore_GetAttributeNameFormatColor( ex_attr_t Type );
eg_cpstr16 ExCore_GetAttributeValueFormatColor( ex_attr_t Type );

ex_xp_value ExCore_GetXpNeededForLevel( ex_attr_value Level );
ex_attr_value ExCore_GetMaxLevelToTrainTo( ex_xp_value XpValue );
ex_attr_value ExCore_GetCostToTrainToLevel( ex_attr_value Level );
ex_attr_t ExCore_CrcToAttrType( eg_string_crc Crc );

ex_attack_t ExCore_StrToAttackType( eg_cpstr Str );
ex_item_slot ExCore_StrToItemSlot( eg_cpstr Str );
eg_string_crc ExCore_GetItemSlotName( ex_item_slot Slot );
ex_equip_m ExCore_StrToEquipMode( eg_cpstr Str );
void ExCore_SlotStrToArray( eg_cpstr Str , EGArray<ex_item_slot>& Out );
exAttrSet ExCore_GetModiferBoost( const EGArray<exAttrModifier>& InModifiers , const exScaleFuncAttrs& Attrs , eg_int ItemLevel );

ex_attr_value ExCore_GetScaledValue( eg_real BaseValue , ex_scale_func Function , ex_attr_value ItemLevel , const exScaleFuncAttrs& CharacterAttrs );

eg_string_crc ExCore_GetHelpText( ex_character_help_text_t Type );

eg_real ExCore_GetModifiedAnimationProgress( eg_real ValueIn , ex_move_animation_context Context );

struct exClassDefaults
{
	#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) ex_attr_value Def##_var_; ex_attr_value Min##_var_; ex_attr_value Max##_var_;
	#include "ExAttrs.items"

	exClassDefaults( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			zero( this );
			if( CT_Default == Ct )
			{
				DefLVL = 1; MinLVL = 1; MaxLVL = 1;
			}
		}
	}

	exClassDefaults( ex_class_t Class );
	exClassDefaults( const eg_string_crc& ClassId );

	ex_attr_value GetMin( ex_attr_t Type ) const
	{
		switch( Type )
		{
			#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return Min##_var_;
			#include "ExAttrs.items"
			default: assert( false ); // This is a computed attribute
		}
		return 0;
	}
	ex_attr_value GetMax( ex_attr_t Type ) const
	{
		switch( Type )
		{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return Max##_var_;
		#include "ExAttrs.items"
		default: assert( false ); // This is a computed attribute
		}
		return 0;
	}
	ex_attr_value GetDef( ex_attr_t Type ) const
	{
		switch( Type )
		{
		#define BASE_ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return Def##_var_;
		#include "ExAttrs.items"
		default: assert( false ); // This is a computed attribute
		}
		return 0;
	}
};
