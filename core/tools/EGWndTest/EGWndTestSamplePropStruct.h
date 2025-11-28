// (c) 2018 Beem Media

#pragma once

#include "EGReflection.h"
#include "EGRflPrimitiveTest.h"
#include "EGWndTestSamplePropStruct.reflection.h"

///////////////////////////////////////////////////////////////////////////////

struct egRegStruct
{
	virtual ~egRegStruct() { }

	eg_real Default = 7.5f;
	eg_real Part2 = 3.7f;
};

egreflect enum class eg_test_raw_enum
{
	Value1,
	Value2,
	Value3,
};

egreflect struct egTestRawItem
{
	egprop eg_string_crc    ItemId = CT_Clear;
	egprop eg_test_raw_enum EnumTest = eg_test_raw_enum::Value2;
	egprop eg_int           ItemLevel = 1;
};

egreflect struct egTestRawItemEx : public egRegStruct , public egprop egTestRawItem
{
	egprop eg_real ExtendedReal = 2.f;
};

egreflect struct egTestItem
{
	egprop eg_d_string          Id;
	egprop egTestRawItem        RawItem;
	egprop eg_vec3              Pos = eg_vec3(0.f,0.f,0.f);
	egprop eg_transform         Transform = CT_Default;
	egprop eg_string_crc        ComboCrc = CT_Clear;
	egprop EGArray<eg_int>      Ints;
	egprop EGArray<eg_d_string> Strings;
};

egreflect class EGWndTestPropStruct
{
friend struct __EGRfl_EGWndTestPropStruct;

private:

	egRflEditor                        m_SelfEditor;

	egprop eg_string_crc               m_Id = CT_Clear;
	egprop EGArray<egRflAllPrimitives> m_MainList;
	egprop eg_bool                     m_SomeBool = true;
	egprop egTestItem                  m_SingleItem;
	egprop EGArray<egTestItem>         m_Items;
	egprop eg_real                     m_LastReal = 0.f;
	egprop eg_d_string                 m_SomeString = "";
	egprop egTestRawItemEx             m_ExItem;

public:

	EGWndTestPropStruct();
	virtual ~EGWndTestPropStruct();

	egRflEditor* GetEditor() { return &m_SelfEditor; }

	void Load();
	void Save();

private:

	eg_d_string GetFilename();

	static void GetFilenames( eg_asset_path_special_t SpecialType , eg_cpstr Ext , EGArray<eg_d_string>& Out );
};
