// (c) 2018 Beem Media

#include "EGEngineDataAssetLib.h"
#include "EGAlwaysLoaded.h"
#include "EGAudio.h"
#include "EGFont.h"
#include "EGPath2.h"
#include "EGFileData.h"
#include <fs_sys2/fs_sys2.h>

void EGEngineDataAssetLib_QuaryFilenameValues( const eg_string_base& Ext , eg_bool bUseRelativePath , eg_cpstr RelativePath , EGArray<eg_string_big>& ValuesOut )
{
	ValuesOut.Append( "" );

	if( Ext.EqualsI("-efont-") )
	{
		EGArray<eg_string_crc> AllFonts;
		EGFontMgr::Get()->QueryFonts( AllFonts );
		for( const eg_string_crc& Font : AllFonts )
		{
			ValuesOut.Append( EGCrcDb::CrcToString( Font ) );
		}
	}
	else if( Ext.EqualsI("-esound-") )
	{
		EGArray<eg_string_small> AllSounds;
		EGAudio_QueryAllSounds( AllSounds );
		for( const eg_string_small& Sound : AllSounds )
		{
			ValuesOut.Append( Sound );
		}
	}
	else if( Ext.Len() > 0 )
	{
		EGAlwaysLoadedFilenameList List;
		AlwaysLoaded_GetFileList( &List , Ext );

		for( const egAlwaysLoadedFilename* Filename : List )
		{
			if( Filename->Filename.Len() >= ( Ext.Len() + 1 ) )
			{
				eg_string_big FinalFilename = Filename->Filename;
				FinalFilename.ClampEnd( Ext.Len() + 1 );
				if( bUseRelativePath )
				{
					FinalFilename = *EGPath2_GetRelativePathTo( FinalFilename , RelativePath );
				}
				ValuesOut.Append( FinalFilename );
			}
		}

		ValuesOut.Sort( []( const eg_string_big& Left , const eg_string_big& Right ) -> eg_bool { return EGString_CompareI( Left , Right ) < 0; } );
	}
}

eg_bool EGEngineDataAssetLib_GameDataOpenFn( eg_cpstr16 Filename , EGFileData& FileDataOut )
{
	eg_bool bReadData = false;
	LF_FILE3 File = LF_Open( Filename , LF_ACCESS_READ|LF_ACCESS_MEMORY , LF_OPEN_EXISTING );
	if( File )
	{
		eg_size_t FileSize = LF_GetSize( File );
		eg_size_t PrevFileDataPos = FileDataOut.Tell();
		eg_size_t SizeLoaded = FileDataOut.Write( LF_GetMemPointer( File ) , LF_GetSize( File ) );
		LF_Close( File );
		bReadData = FileSize == SizeLoaded;
		FileDataOut.Seek( eg_file_data_seek_t::Begin , EG_To<eg_int>(PrevFileDataPos) );
	}
	return bReadData;
}

eg_bool EGEngineDataAssetLib_GameDataSaveFn( eg_cpstr16 Filename , const EGFileData& FileData )
{
	eg_bool bWroteData = false;
	LF_FILE3 File = LF_Open( Filename , LF_ACCESS_WRITE , LF_CREATE_ALWAYS );
	if( File )
	{
		eg_size_t SizeWritten = LF_Write( File , FileData.GetData() , EG_To<fs_dword>(FileData.GetSize()) );
		LF_Close( File );
		bWroteData = SizeWritten == FileData.GetSize();
	} 

	return bWroteData;
}
