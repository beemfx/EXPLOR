// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"
#include "ExCharacterClass.reflection.h"

egreflect struct exCharacterClassBalanceData
{
	egprop eg_int Level = 1;
	egprop exRawItem RhWeapon = CT_Clear;
	egprop exRawItem LhWeapon = CT_Clear;
	egprop exRawItem Armor = CT_Clear;
	egprop exRawItem Helm = CT_Clear;
};

egreflect struct exCharacterClass
{
	egprop eg_string_crc ClassCrcId = CT_Clear;
	egprop ex_class_t ClassEnum = ex_class_t::Unknown;
	egprop eg_bool bIsCharacterClass = false;
	egprop eg_string_crc Name = CT_Clear;
	egprop eg_d_string Name_enus = CT_Clear;
	egprop eg_string_crc Description = CT_Clear;
	egprop eg_d_string_ml Description_enus = CT_Clear;

	egprop eg_string_crc DefaultRhWeapon = CT_Clear;
	egprop eg_int        DefaultRhWeaponLevel = 1;
	egprop eg_string_crc DefaultLhWeapon = CT_Clear;
	egprop eg_int        DefaultLhWeaponLevel = 1;
	egprop EGArray<eg_string_crc> DefaultAttunedSkills;

	egprop EGArray<exCharacterClassBalanceData> BalanceData;

	egprop exAttrBaseSet DefaultAttributes = CT_Default;
	egprop exAttrBaseSet CreationMinAttributes = CT_Default;
	egprop exAttrBaseSet CreationMaxAttributes = CT_Default;

	void SortBalanceData();
};

egreflect struct exDefaultCharacter
{
	egprop eg_d_string CharacterName = CT_Clear;
	egprop eg_string_crc ClassId = CT_Clear;
	egprop eg_string_crc PortraitId = CT_Clear;
};

egreflect struct exCharacterClassGlobals
{
	egprop EGArray<exCharacterClass> Classes;
	egprop EGArray<exDefaultCharacter> DefaultCharacters;
	exCharacterClass DefaultClass;

	const exCharacterClass& GetClass( const eg_string_crc& ClassId ) const;
	void PostLoad();
};
