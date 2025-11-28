// (c) 2018 Beem Media

#include "EGLibFile.h"
#include "EGFileData.h"

#if defined(__WIN32__) || defined(__WIN64__)
#include "EGWindowsAPI.h"
#endif

static eg_bool EGLibFile_DefaultOSOpenFn( eg_cpstr16 Filename , EGFileData& FileDataOut );
static eg_bool EGLibFile_DefaultOSSaveFn( eg_cpstr16 Filename , const EGFileData& FileData );

static EGLibFileOpenCb EGLibFile_Open_OS = nullptr;
static EGLibFileSaveCb EGLibFile_Save_OS = nullptr;

static EGLibFileOpenCb EGLibFile_Open_GameData = nullptr;
static EGLibFileSaveCb EGLibFile_Save_GameData = nullptr;

void EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t FileType )
{
	switch( FileType )
	{
	case eg_lib_file_t::OS:
		EGLibFile_Open_OS = EGLibFile_DefaultOSOpenFn;
		EGLibFile_Save_OS = EGLibFile_DefaultOSSaveFn;
		break;
	case eg_lib_file_t::GameData:
		EGLibFile_Open_GameData = EGLibFile_DefaultOSOpenFn;
		EGLibFile_Save_GameData = EGLibFile_DefaultOSSaveFn;
		break;
	}
}

void EGLibFile_SetFunctions( eg_lib_file_t FileType, EGLibFileOpenCb OpenFn, EGLibFileSaveCb SaveFn )
{
	switch( FileType )
	{
		case eg_lib_file_t::OS:
			EGLibFile_Open_OS = OpenFn;
			EGLibFile_Save_OS = SaveFn;
			break;
		case eg_lib_file_t::GameData:
			EGLibFile_Open_GameData = OpenFn;
			EGLibFile_Save_GameData = SaveFn;
			break;
	}
}

eg_bool EGLibFile_OpenFile( eg_cpstr16 Filename, eg_lib_file_t FileType, EGFileData& FileDataOut )
{
	eg_bool bOpened = false;
	eg_size_t StartPos = FileDataOut.Tell();

	switch( FileType )
	{
		case eg_lib_file_t::OS: 
			bOpened = EGLibFile_Open_OS ? EGLibFile_Open_OS( Filename , FileDataOut ) : false;
			break;
		case eg_lib_file_t::GameData: 
			bOpened = EGLibFile_Open_GameData ? EGLibFile_Open_GameData( Filename , FileDataOut ) : false;
			break;
	}

	FileDataOut.Seek( eg_file_data_seek_t::Begin , EG_To<eg_int>(StartPos) );

	return bOpened;
}

eg_bool EGLibFile_OpenFile( eg_cpstr16 Filename, eg_lib_file_t FileType, EGArray<eg_byte>& FileData )
{
	return EGLibFile_OpenFile( Filename , FileType , EGFileData( FileData ) );
}

eg_bool EGLibFile_SaveFile( eg_cpstr16 Filename, eg_lib_file_t FileType, const EGFileData& FileData )
{
	switch( FileType )
	{
		case eg_lib_file_t::OS: return EGLibFile_Save_OS ? EGLibFile_Save_OS( Filename , FileData ) : false;
		case eg_lib_file_t::GameData: return EGLibFile_Save_GameData ? EGLibFile_Save_GameData( Filename , FileData ) : false;
	}
	return false;
}

eg_bool EGLibFile_SaveFile( eg_cpstr16 Filename, eg_lib_file_t FileType, const EGArray<eg_byte>& FileData )
{
	return EGLibFile_SaveFile( Filename , FileType , EGFileData( FileData ) );
}

//
// Default Functions
//

static eg_bool EGLibFile_DefaultOSOpenFn( eg_cpstr16 Filename , EGFileData& FileDataOut )
{
#if defined(__WIN32__) || defined(__WIN64__)
	eg_bool bSuccess = false;

	HANDLE hFile = CreateFileW( Filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if( INVALID_HANDLE_VALUE == hFile )
	{
		// EGLogf( eg_log_t::Error , ( "OS Open File Error: Couldn't open input file \"%s\"") , EGString_ToMultibyte(Filename) );
		return false;
	}

	const eg_uint Size = GetFileSize( hFile , NULL );

	//Read the whole chunk.
	eg_byte* pData = new eg_byte[Size];
	if( pData )
	{
		DWORD SizeRead = 0;
		if( ReadFile( hFile , pData , Size , &SizeRead , NULL ) )
		{
			if( Size == SizeRead )
			{
				eg_size_t FileDataOrigSize = FileDataOut.GetSize();
				FileDataOut.Write( pData , Size );
				if( (FileDataOut.GetSize()-FileDataOrigSize) == Size )
				{
					bSuccess = true;
				}
			}
		}
		delete [] pData;
	}

	CloseHandle( hFile );

	if( !bSuccess )
	{
		// EGLogf( eg_log_t::Error , ("OS Open File Error: Could not read input file \"%s\""), EGString_ToMultibyte(Filename));
	}

	return bSuccess;
#else
	return false;
#endif
}

static eg_bool EGLibFile_DefaultOSSaveFn( eg_cpstr16 Filename , const EGFileData& FileData )
{
#if defined(__WIN32__) || defined(__WIN64__)
	//Open output file.
	HANDLE hFile = CreateFileW( Filename , GENERIC_WRITE , 0 , NULL , CREATE_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL );
	if( INVALID_HANDLE_VALUE == hFile )
	{
		// EGLogf( eg_log_t::Error , "OS Open File Error: Couldn't open output file \"%s\"" , EGString_ToMultibyte(Filename) );
		return false;
	}

	eg_uint SizeWritten = 0;
	eg_bool bSuccess = TRUE == WriteFile( hFile, FileData.GetData() , static_cast<DWORD>(FileData.GetSize()) , &SizeWritten , NULL );
	CloseHandle( hFile );

	if( !bSuccess || FileData.GetSize() != SizeWritten )
	{
		// EGLogf( eg_log_t::Error , ("OS Open File Error: Couldn't write output file \"%s\"") , EGString_ToMultibyte(Filename) );
		return false;
	}

	return true;
#else
	return false;
#endif
}
