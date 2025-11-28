// (c) 2018 Beem Media

#include "EGDataAssetLoader.h"
#include "EGDataAssetBase.h"
#include "EGReflection.h"
#include "EGFileData.h"
#include "EGLibFile.h"
#include "EGReflectionDeserializer.h"

EGDataAssetLoader::EGDataAssetLoader( eg_cpstr16 InFilename , eg_mem_pool InMemPool , eg_bool bForEditor , egRflEditor& RflEditor ) 
: m_MemPool( InMemPool )
, m_RflEditor( RflEditor )
{
	EGFileData FileData( eg_file_data_init_t::HasOwnMemory );
	eg_bool bLoadedFile = EGLibFile_OpenFile( InFilename, bForEditor ? eg_lib_file_t::OS : eg_lib_file_t::GameData, FileData );
	if( bLoadedFile )
	{
		XMLLoad( FileData.GetData(), FileData.GetSize(), EGString_ToMultibyte( InFilename ) );
	}
}

EGDataAssetLoader::EGDataAssetLoader( const EGFileData& MemFile, eg_cpstr16 InRefFilename, eg_mem_pool InMemPool, eg_bool bForEditor, egRflEditor& RflEditor )
: m_MemPool( InMemPool )
, m_RflEditor( RflEditor )
{
	unused( bForEditor );

	XMLLoad( MemFile.GetData(), MemFile.GetSize(), EGString_ToMultibyte( InRefFilename ) );
}

EGDataAssetLoader::~EGDataAssetLoader()
{
	if( m_RflDeserializer )
	{
		m_RflDeserializer->Deinit();
		EG_SafeDelete( m_RflDeserializer );
	}
	m_DataAsset = nullptr;
	m_DataAssetClass = nullptr;
}

void EGDataAssetLoader::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	if( Tag.Equals( "egasset" ) )
	{
		assert( nullptr == m_DataAssetClass );
		assert( nullptr == m_DataAsset );
		assert( nullptr == m_RflDeserializer );

		m_DataAssetClass = EGClass::FindClassSafe( AttGet.GetString( "class", "EGDataAsset" ), EGDataAsset::GetStaticClass() );
		if( m_DataAssetClass )
		{
			m_DataAsset = EGNewObject<EGDataAsset>( m_DataAssetClass, m_MemPool );
			if( m_DataAsset )
			{
				m_RflEditor = EGReflection_GetEditorForClass( m_DataAssetClass->GetName() , m_DataAsset , "DataAsset" );
				m_RflDeserializer = new ( eg_mem_pool::DefaultHi ) EGReflectionDeserializer();
				if( m_RflDeserializer )
				{
					m_RflDeserializer->Init( m_RflEditor , GetXmlFilename() );
				}
			}
		}
		else
		{
			EGLogf( eg_log_t::Error , "Tried to deserialize a data asset with no class. (Was this for the correct game?)" );
		}
	}
	else
	{
		if( m_RflDeserializer )
		{
			m_RflDeserializer->OnXmlTagBegin( Tag , AttGet );
		}
	}
}

void EGDataAssetLoader::OnTagEnd( const eg_string_base& Tag )
{
	if( Tag.EqualsI( "egasset" ) )
	{

	}
	else
	{
		if( m_RflDeserializer )
		{
			m_RflDeserializer->OnXmlTagEnd( Tag );
		}
	}
}
