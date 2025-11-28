// (c) 2018 Beem Media

#include "EGSync.h"
#include "EGOsFile.h"
#include "EGLibFile.h"
#include "EGFileData.h"
#include "EGPath2.h"

static const eg_uint32 EGSync_BigFileType = (('E' << 0) | ('G' << 8) | ('B' << 16) | ('G' << 24));
static const eg_uint32 EGSync_BigFileVersion = 100;
static const eg_uint32 EGSync_FileHeader = (('F' << 0) | ('I' << 8) | ('L' << 16) | ('E' << 24));

void EGSync_PackBigFile( eg_cpstr InDir , eg_cpstr OutFile )
{
	EGLogf( eg_log_t::General , "Packing \"%s\" to \"%s\"..." , InDir , OutFile );

	eg_d_string OutDir = *EGPath2_BreakPath( OutFile ).GetDirectory();
	EGOsFile_CreateDirectory( *OutDir );

	// Compress the entire source director into a monolithic egbigfile:
	EGArray<eg_d_string> AllFiles;
	EGOsFile_FindAllFiles( InDir , nullptr , AllFiles );

	EGFileData BigFileData( eg_file_data_init_t::HasOwnMemory );
	BigFileData.Write<eg_uint32>( EGSync_BigFileType );
	BigFileData.Write<eg_uint32>( EGSync_BigFileVersion );
	for( const eg_d_string& Filename : AllFiles )
	{
		EGLogf( eg_log_t::General , "Packing \"%s\"..." , *Filename );
		eg_d_string FilenameFull = InDir;
		FilenameFull.Append( *Filename );
		EGFileData InFile( eg_file_data_init_t::HasOwnMemory );
		EGLibFile_OpenFile( *FilenameFull , eg_lib_file_t::OS , InFile );
		BigFileData.Write<eg_uint32>( EGSync_FileHeader );
		BigFileData.Write<eg_uint32>( EG_To<eg_uint32>(InFile.GetSize()) );
		BigFileData.Write<eg_uint32>( Filename.LenAs<eg_uint32>() );
		BigFileData.Write( *Filename , Filename.LenAs<eg_uint32>() );
		BigFileData.Write( InFile.GetData() , EG_To<eg_uint32>(InFile.GetSize()) );
	}
	EGLibFile_SaveFile( OutFile , eg_lib_file_t::OS , BigFileData );
}

void EGSync_UnpackBigFile( eg_cpstr InFile, eg_cpstr OutDir )
{
	EGOsFile_CreateDirectory( OutDir );
	EGOsFile_ClearDirectory( OutDir );

	EGLogf( eg_log_t::General , "Unpacking \"%s\" to \"%s\"..." , InFile , OutDir );

	EGFileData BigFileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( InFile , eg_lib_file_t::OS , BigFileData );
	const eg_uint32 FileType = BigFileData.Read<eg_uint32>();
	const eg_uint32 FileVersion = BigFileData.Read<eg_uint32>();
	if( !(FileType == EGSync_BigFileType && FileVersion == EGSync_BigFileVersion) )
	{
		EGLogf( eg_log_t::Error , "Error: Invalid big file type or version." );
		return;
	}

	EGArray<eg_byte> FileData;
	eg_bool bEndFound = false;
	while( !bEndFound )
	{
		// Read the header
		const eg_uint32 FileHeader = BigFileData.Read<eg_uint32>();
		const eg_uint32 FileSize = BigFileData.Read<eg_uint32>();
		const eg_uint32 FilenameSize = BigFileData.Read<eg_uint32>();

		if( FileHeader != EGSync_FileHeader )
		{
			EGLogf( eg_log_t::Error , "Error: Invalid file header found." );
			return;
		}

		eg_char Filename[256];
		if( FilenameSize >= countof(Filename) )
		{
			EGLogf( eg_log_t::Error , "Error: Filename too long." );
			return;
		}
		BigFileData.Read( Filename , FilenameSize );
		Filename[FilenameSize] = '\0';
		EGLogf( eg_log_t::General , "Extracting \"%s\"..." , Filename );

		FileData.Resize( FileSize );
		EGMem_Set( FileData.GetArray() , 0 , FileData.Len() );
		BigFileData.Read( FileData.GetArray() , FileData.Len() );

		EGFileData ExtractFileData( FileData );
		eg_d_string OutFilenameFull = OutDir;
		OutFilenameFull.Append( Filename );
		EGOsFile_CreateDirectory( *eg_d_string8(*EGPath2_BreakPath(*OutFilenameFull).GetDirectory()) );
		EGLibFile_SaveFile( *OutFilenameFull , eg_lib_file_t::OS , ExtractFileData );

		if( BigFileData.Tell() == BigFileData.GetSize() )
		{
			bEndFound = true;
		}
	}
}

void EGSync_SplitBigFile( eg_cpstr BigFile , eg_cpstr OutDir )
{
	EGArray<eg_s_string_sml8> Manifest;

	EGOsFile_CreateDirectory( OutDir );

	EGFileData BigFileData( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( BigFile , eg_lib_file_t::OS , BigFileData );
	const eg_size_t FileSize = BigFileData.GetSize();
	// We'll split the whole thing into XMB chunks.
	const eg_size_t ChunkSize = 5*1024*1024;
	const eg_size_t TotalChunks = FileSize/ChunkSize + ((FileSize%ChunkSize != 0) ? 1 : 0);
	EGLogf( eg_log_t::General , "Breaking \"%s\" into %d chunks..." , BigFile , TotalChunks );

	for( eg_size_t i=0; i<TotalChunks; i++ )
	{
		const eg_size_t ChunkStart = i*ChunkSize;
		const eg_size_t ThisChunkSize = (i<(TotalChunks-1)) ? ChunkSize : (FileSize%ChunkSize);
		eg_d_string FileName = EGString_Format( "chunk_%04d.egpart" , i );
		eg_d_string FullFileName = OutDir;
		FullFileName.Append( *FileName );
		Manifest.Append( *FileName );
		EGLogf( eg_log_t::General , "Writing %s (%.2fMB)" , *FileName , EG_To<eg_real>(ThisChunkSize/(1024.f*1024.f)) );
		EGFileData TempFile( BigFileData.GetDataAt(ChunkStart) , ThisChunkSize );
		EGLibFile_SaveFile( *FullFileName , eg_lib_file_t::OS , TempFile );
	}

	// Save the manifest.
	EGFileData ManifestFileData( eg_file_data_init_t::HasOwnMemory );
	for( const eg_s_string_sml8& ManFile : Manifest )
	{
		ManifestFileData.WriteStr8( EGString_Format( "%s\r\n" , *ManFile ) );
	}
	eg_d_string ManifestFileFull = EGString_Format( "%smanifest.html" , OutDir );
	EGLibFile_SaveFile( *ManifestFileFull , eg_lib_file_t::OS , ManifestFileData );
}

void EGSync_AssembleBigFile( eg_cpstr InDir , eg_cpstr OutFile )
{
	eg_d_string OutFileDir = *EGPath2_BreakPath( OutFile ).GetDirectory( '/' );
	EGOsFile_CreateDirectory( *OutFileDir );

	eg_d_string ManifestFilename = InDir;
	ManifestFilename.Append( "manifest.html" );

	EGLogf( eg_log_t::General , "Reading \"%s\" manifest for assembly..." , *ManifestFilename );

	// Read the manifest:
	EGArray<eg_s_string_sml8> Manifest;
	{
		EGArray<eg_byte> ManifestFileData;
		EGLibFile_OpenFile( *ManifestFilename , eg_lib_file_t::OS , ManifestFileData );
		EgSync_ReadManifest( ManifestFileData , Manifest );
	}

	EGLogf( eg_log_t::General , "%d chunks found. Assembling..." , Manifest.LenAs<eg_uint32>() );

	// Reassemble the big file
	EGFileData BigFileData( eg_file_data_init_t::HasOwnMemory );
	for( const eg_s_string_sml8& ManFilename : Manifest )
	{
		eg_d_string ManFilenameFull = InDir;
		ManFilenameFull.Append( *ManFilename );
		EGLogf( eg_log_t::General , "Assembling \"%s\"..." , *ManFilename );
		EGFileData ManFileData( eg_file_data_init_t::HasOwnMemory );
		EGLibFile_OpenFile( *ManFilenameFull , eg_lib_file_t::OS , ManFileData );
		BigFileData.Write( ManFileData.GetData() , ManFileData.GetSize() );
	}
	EGLibFile_SaveFile( OutFile , eg_lib_file_t::OS , BigFileData );
}

void EgSync_ReadManifest(const EGArray<eg_byte>& FileData, EGArray<eg_s_string_sml8>& Out)
{
	eg_s_string_sml8 CurWord;
	for( eg_size_t i=0; i<FileData.Len(); i++ )
	{
		const eg_char CurChar = FileData[i];
		if( CurChar == '\r' || CurChar == '\n' )
		{
			if( CurWord.Len() > 0 )
			{
				Out.Append( CurWord );
			}
			CurWord.Clear();
		}
		else
		{
			CurWord.Append( CurChar );
		}
	}
	if( CurWord.Len() > 0 )
	{
		Out.Append( CurWord );
	}
	CurWord.Clear();
}
