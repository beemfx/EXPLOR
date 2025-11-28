// (c) 2018 Beem Media

#pragma once

class EGFileData;

enum class eg_lib_file_t
{
	OS,
	GameData,
};

typedef eg_bool ( * EGLibFileOpenCb )( eg_cpstr16 Filename , EGFileData& FileDataOut );
typedef eg_bool ( * EGLibFileSaveCb )( eg_cpstr16 Filename , const EGFileData& FileData );

void EGLibFile_SetFunctionsToDefaultOS( eg_lib_file_t FileType );
void EGLibFile_SetFunctions( eg_lib_file_t FileType , EGLibFileOpenCb OpenFn , EGLibFileSaveCb SaveFn );

eg_bool EGLibFile_OpenFile( eg_cpstr16 Filename, eg_lib_file_t FileType, EGFileData& FileData );
eg_bool EGLibFile_SaveFile( eg_cpstr16 Filename, eg_lib_file_t FileType, const EGFileData& FileData );
static inline eg_bool EGLibFile_OpenFile( eg_cpstr8 Filename, eg_lib_file_t FileType, EGFileData& FileData ) { return EGLibFile_OpenFile( EGString_ToWide( Filename ), FileType, FileData ); }
static inline eg_bool EGLibFile_SaveFile( eg_cpstr8 Filename, eg_lib_file_t FileType, const EGFileData& FileData ) { return EGLibFile_SaveFile( EGString_ToWide( Filename ), FileType, FileData ); }

eg_bool EGLibFile_OpenFile( eg_cpstr16 Filename , eg_lib_file_t FileType , EGArray<eg_byte>& FileData );
eg_bool EGLibFile_SaveFile( eg_cpstr16 Filename , eg_lib_file_t FileType , const EGArray<eg_byte>& FileData );
static inline eg_bool EGLibFile_OpenFile( eg_cpstr8 Filename , eg_lib_file_t FileType , EGArray<eg_byte>& FileData ) { return EGLibFile_OpenFile( EGString_ToWide(Filename) , FileType , FileData ); }
static inline eg_bool EGLibFile_SaveFile( eg_cpstr8 Filename , eg_lib_file_t FileType , const EGArray<eg_byte>& FileData ) { return EGLibFile_SaveFile( EGString_ToWide(Filename) , FileType , FileData ); }
