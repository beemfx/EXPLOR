// (c) 2018 Beem Media

#include "EGDataAssetBase.h"
#include "EGReflection.h"
#include "EGFileData.h"
#include "EGEdUtility.h"
#include "EGDataAssetLoader.h"

EG_CLASS_DECL( EGDataAsset )

void EGDataAsset::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	RflEditor.ExecutePostLoad( EGString_ToMultibyte(Filename) , bForEditor );
}

void EGDataAsset::SaveGameDataTo( EGFileData& DataOut )
{
	if( GetObjectClass() )
	{
		const egRflEditor RflEditor = EGReflection_GetEditorForClass( GetObjectClass()->GetName() , this , "DataAsset" );

		EGDataAsset::SaveDataAsset( DataOut , this , RflEditor );
	}
}

EGDataAsset* EGDataAsset::CreateDataAsset( EGClass* Class, eg_mem_pool MemPool, egRflEditor& RflEditor )
{
	EGDataAsset* Out = nullptr;
	RflEditor = CT_Clear;
	if( Class )
	{
		Out = EGNewObject<EGDataAsset>( Class , MemPool );
		RflEditor = EGReflection_GetEditorForClass( Class->GetName() , Out , "DataAsset" );
	}
	return Out;
}

EGDataAsset* EGDataAsset::LoadDataAsset( eg_cpstr16 Filename, eg_mem_pool MemPool, eg_bool bForEditor, egRflEditor& RflEditor )
{
	EGDataAsset* NewDataAsset = EGDataAssetLoader( Filename , MemPool , bForEditor , RflEditor ).GetDataAsset();
	if( NewDataAsset )
	{
		NewDataAsset->PostLoad( Filename , bForEditor , RflEditor );
	}
	return NewDataAsset;
}

EGDataAsset* EGDataAsset::LoadDataAsset( const EGFileData& MemFile , eg_cpstr16 RefFilename , eg_mem_pool MemPool , eg_bool bForEditor , egRflEditor& RflEditor )
{
	EGDataAsset* NewDataAsset = EGDataAssetLoader( MemFile , RefFilename , MemPool , bForEditor , RflEditor ).GetDataAsset();
	if( NewDataAsset )
	{
		NewDataAsset->PostLoad( RefFilename , bForEditor , RflEditor );
	}
	return NewDataAsset;
}

eg_bool EGDataAsset::SaveDataAsset( EGFileData& FileDataOut , EGDataAsset* DataAsset , const egRflEditor& RflEditor )
{
	FileDataOut.WriteStr8( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	FileDataOut.WriteStr8( EGString_Format( "<egasset class=\"%s\">\r\n" , DataAsset->GetObjectClass()->GetName() ) );

	if( DataAsset == RflEditor.GetData() )
	{
		RflEditor.Serialize( eg_rfl_serialize_fmt::XML , 1 , FileDataOut );
	}
	else
	{
		EGLogf( eg_log_t::Error , "Data asset editor contained the wrong data." );
	}

	FileDataOut.WriteStr8( "</egasset>\r\n" );
	return true;
}


eg_bool EGDataAsset::SaveDataAsset( eg_cpstr16 Filename, EGDataAsset* DataAsset, const egRflEditor& RflEditor )
{
	eg_bool bWritten = false;

	if( DataAsset && DataAsset->GetObjectClass() )
	{
		EGFileData Out( eg_file_data_init_t::HasOwnMemory );
		SaveDataAsset( Out , DataAsset , RflEditor );
		bWritten = EGEdUtility_SaveFile( Filename , Out );
	}

	return bWritten;
}
