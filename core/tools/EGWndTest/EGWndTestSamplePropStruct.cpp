// (c) 2018 Beem Media

#include "EGWndTestSamplePropStruct.h"
#include "EGCrcDb.h"
#include "EGFileData.h"
#include "EGToolsHelper.h"
#include "EGPath2.h"
#include "EGLibFile.h"
#include "EGXMLBase.h"
#include "EGReflectionDeserializer.h"
#include "EGAssetPath.h"
#include "EGOsFile.h"

class EGTestStructDeserializer : public IXmlBase
{
private:

	EGReflectionDeserializer m_Deserializer;
	eg_bool                  m_bInTestSection = false;
	
public:

	EGTestStructDeserializer( EGFileData& FileData , egRflEditor& TargetIn )
	{
		m_Deserializer.Init( TargetIn , "TestFile" );

		XMLLoad( FileData.GetData() , FileData.GetSize() , "TestFile" );

		TargetIn.HandlePostLoad();

		typedef eg_s_string_base<eg_char8,12> eg_s_string_test;

		eg_s_string_test SmallString = L"My small ";
		SmallString.Append( 'a' );
		SmallString.Append( 'b' );
		assert( SmallString.IsValid() );

		eg_s_string_test TestTwo( std::move( SmallString ) );
		SmallString = TestTwo;
		TestTwo = "Test Two";
		TestTwo = TestTwo;

		TestTwo = std::move( SmallString );

		assert( TestTwo.Equals( *SmallString ) );

		SmallString = "small";
		TestTwo = "SmaLL";

		assert( SmallString.EqualsI( *TestTwo ) );

		assert( TestTwo.IsValid() );

		eg_s_string_test One = "1+2";
		eg_s_string_test Two = "=3";
		eg_s_string_test Three = One + Two;
		assert(Three.IsValid() );
	}

	~EGTestStructDeserializer()
	{
		m_Deserializer.Deinit();
	}

private:

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) override
	{
		if( m_bInTestSection )
		{
			m_Deserializer.OnXmlTagBegin( Tag , AttGet );
		}

		if( Tag.Equals( "test" ) )
		{
			m_bInTestSection = true;
		}
	}

	virtual void OnTagEnd( const eg_string_base& Tag ) override 
	{
		if( Tag.Equals( "test" ) )
		{
			m_bInTestSection = false;
		}

		if( m_bInTestSection )
		{
			m_Deserializer.OnXmlTagEnd( Tag );
		}
	}

	virtual void OnData( eg_cpstr DataStr , eg_uint DataLen ) override
	{
		unused( DataStr , DataLen );
	}

	virtual eg_cpstr XMLObjName() const override { return "EGTestStructDeserializer"; }
};

EGWndTestPropStruct::EGWndTestPropStruct()
{
	eg_asset_path::SetGetAssetPathsFn( GetFilenames );
	/*
	// Set up a test case.
	m_Items.Resize( 2 );
	m_Items[1].Ints.Resize( 3 );
	m_Items[1].Ints[2] = -25;

	m_Items[0].ComboCrc = EGCrcDb::StringToCrc("The First Item");
	m_Items[1].ComboCrc = EGCrcDb::StringToCrc("2) An Item");

	m_Items[0].Id = "Inventory 1";
	m_Items[1].Id = "Second Inventory";

	m_Items[1].Strings.Resize( 1 );
	m_Items[1].Strings[0] = "A third item, but in 2nd inventory.";
	*/

	m_SelfEditor = EGReflection_GetEditor( *this , "Test" );

	Load();
}

EGWndTestPropStruct::~EGWndTestPropStruct()
{
	Save();
}

void EGWndTestPropStruct::Load()
{
	EGFileData InFile( eg_file_data_init_t::HasOwnMemory );
	if( EGLibFile_OpenFile( *GetFilename() , eg_lib_file_t::OS , InFile ) )
	{
		EGLogf( eg_log_t::General , "Loading..." );
		EGTestStructDeserializer( InFile , m_SelfEditor );
		m_SelfEditor.DebugPrint( 0 );
	}
}

void EGWndTestPropStruct::Save()
{
	EGLogf( eg_log_t::General , "Saving..." );
	if( m_SelfEditor.GetData() == this )
	{
		EGFileData SerializedFile( eg_file_data_init_t::HasOwnMemory );
		SerializedFile.WriteStr8( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
		SerializedFile.WriteStr8( "<test>\r\n" );
		m_SelfEditor.Serialize( eg_rfl_serialize_fmt::XML , 1 , SerializedFile );
		SerializedFile.WriteStr8( "</test>\r\n" );

		EGLibFile_SaveFile( *GetFilename() , eg_lib_file_t::OS , SerializedFile );
	}
	else
	{
		EGLogf( eg_log_t::Error , "Self editor got broken." );
	}
}

eg_d_string EGWndTestPropStruct::GetFilename()
{
	eg_d_string SaveToFilename = EGPath2_CleanPath( EGString_Format( "%s/!test_serialize.xml" , EGToolsHelper_GetEnvVar("EGOUT").String() ) , '/' );
	return SaveToFilename;
}

void EGWndTestPropStruct::GetFilenames( eg_asset_path_special_t SpecialType , eg_cpstr Ext , EGArray<eg_d_string>& Out )
{
	unused( SpecialType , Ext );

	EGOsFile_FindAllFiles( "." , nullptr , Out );
}

#define EGRFL_SYSTEM_HEADER "tools.link.reflection.hpp"
#include "EGRflLinkSystem.hpp"
#undef EGRFL_SYSTEM_HEADER

#define EGRFL_SYSTEM_HEADER "foundation.link.reflection.hpp"
#include "EGRflLinkSystem.hpp"
#undef EGRFL_SYSTEM_HEADER
