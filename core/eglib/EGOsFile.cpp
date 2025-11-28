// (c) 2018 Beem Media

#include "EGOsFile.h"
#include "EGWindowsAPI.h"
#include "EGPath2.h"

struct egOsFileFoundFile
{
	eg_string_big Filename = CT_Clear;
	eg_flags      FileAttr = CT_Clear;

	egOsFileFoundFile() = default;
	egOsFileFoundFile( eg_cpstr FilenameIn , eg_flags FileAttrIn ): Filename( FilenameIn ) , FileAttr( FileAttrIn ) { }
};

egOsFileAttributes::egOsFileAttributes( eg_uint32 OsFlagsIn )
: bDirectory( 0 != (FILE_ATTRIBUTE_DIRECTORY&OsFlagsIn) )
, bHidden   ( 0 != (FILE_ATTRIBUTE_HIDDEN&OsFlagsIn) )
, bReadOnly ( 0 != (FILE_ATTRIBUTE_READONLY&OsFlagsIn) )
, bSystem   ( 0 != (FILE_ATTRIBUTE_SYSTEM&OsFlagsIn) )
, bTemp     ( 0 != (FILE_ATTRIBUTE_TEMPORARY&OsFlagsIn) )
{

}


void EGOsFile_SetCWD( eg_cpstr8 RootDir )
{
	SetCurrentDirectoryA( RootDir );
}

void EGOsFile_SetCWD( eg_cpstr16 RootDir )
{
	SetCurrentDirectoryW( RootDir );
}

eg_d_string16 EGOsFile_GetCWD()
{
	eg_d_string16 Out;

	eg_char16 TempBuffer[1024];
	GetCurrentDirectoryW( countof(TempBuffer) , TempBuffer );
	Out = TempBuffer;
	return Out;
}

void EGOsFile_DeleteFile( eg_cpstr16 Filename )
{
	DeleteFileW( Filename );
}

void EGOsFile_DeleteFile( eg_cpstr8 Filename )
{
	EGOsFile_DeleteFile( *eg_d_string16(Filename) );
}

void EGOsFile_DeleteDirectory( eg_cpstr16 Directory )
{
	EGLogf( eg_log_t::General , "Deleting %s..." , *eg_s_string_sml8(Directory) );

	if( !EGOsFile_DoesDirExist( Directory ) )
	{
		EGLogf( eg_log_t::Warning , "No such directory." );
		return;
	}

	DWORD DirAttrs = GetFileAttributesW( Directory );
	if( 0 != (DirAttrs&FILE_ATTRIBUTE_SYSTEM) )
	{
		EGLogf( eg_log_t::Warning , "Directory is system directory." );
		return;
	}

	EGOsFile_ClearDirectory( Directory );

	// EGLogf( eg_log_t::Warning , "RemoveDirectory( \"%s\" );" , Directory );
	RemoveDirectoryW( Directory );
}

void EGOsFile_DeleteDirectory( eg_cpstr8 Directory )
{
	EGOsFile_DeleteDirectory( *eg_d_string16(Directory) );
}

void EGOsFile_ClearDirectory( eg_cpstr16 Directory )
{
	EGLogf( eg_log_t::General , "Clearing %s..." , *eg_s_string_sml8(Directory) );

	if( !EGOsFile_DoesDirExist( Directory ) )
	{
		EGLogf( eg_log_t::Warning , "No such directory." );
		return;
	}

	DWORD DirAttrs = GetFileAttributesW( Directory );
	if( 0 != (DirAttrs&FILE_ATTRIBUTE_SYSTEM) )
	{
		EGLogf( eg_log_t::Warning , "Directory is system directory." );
		return;
	}

	eg_int FilesRemoved = 0;

	WIN32_FIND_DATAW FindData;
	zero( &FindData );

	eg_d_string16 Pattern = Directory;
	Pattern.Append( L"/*.*" );
	HANDLE hFile = FindFirstFileW( *Pattern , &FindData);
	if( hFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			eg_d_string16 ChildPath = Directory;
			if( ChildPath.Len() == 0 || (ChildPath[ChildPath.Len()-1] != '/' && ChildPath[ChildPath.Len()-1] != '\\') )
			{
				ChildPath.Append( L"/" );
			}
			ChildPath.Append( FindData.cFileName );

			if( FindData.cFileName[0] == '\0' || FindData.cFileName[0] == '.' )
			{
				// Nothing to do...
			}
			else if( 0 != (FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
			{
				ChildPath.Append( L"/" );
				EGOsFile_DeleteDirectory( *ChildPath );
			}
			else
			{
				// EGLogf( eg_log_t::Warning , "DeleteFile( \"%s\" );" , *ChildPath );
				EGOsFile_DeleteFile( *ChildPath );
				FilesRemoved++;
			}
		} while( FindNextFileW( hFile , &FindData ) );

		FindClose( hFile );
	}

	if( FilesRemoved > 0 )
	{
		// EGLogf( eg_log_t::General , "Removed %d files." , FilesRemoved );
	} 
}

void EGOsFile_ClearDirectory( eg_cpstr8 Directory )
{
	EGOsFile_ClearDirectory( *eg_d_string16(Directory) );
}

eg_bool EGOsFile_CreateDirectory( eg_cpstr8 DirPath )
{
	return EGOsFile_CreateDirectory( *eg_d_string16(DirPath) );
}

eg_bool EGOsFile_CreateDirectory( eg_cpstr16 DirPath )
{
	if( EGOsFile_DoesDirExist( DirPath ) )
	{
		return true;
	}

	eg_d_string16 AdjustedDirName = DirPath;
	AdjustedDirName += "/"; // This will make sure that the last thing found in BreakPath is a directory and not a filename

	egPathParts2 PathParts;
	EGPath2_BreakPath( *AdjustedDirName, PathParts );

	eg_d_string16 CurrentFolder = *PathParts.GetRoot( '\\' );
	for( const eg_d_string16& Folder : PathParts.Folders )
	{
		CurrentFolder.Append( Folder );
		if( !EGOsFile_DoesDirExist( *CurrentFolder ) )
		{
			BOOL bCreatedDir = CreateDirectoryW( *CurrentFolder, NULL );
			if( !bCreatedDir )
			{
				DWORD LastError = GetLastError();
				LastError = LastError;
				assert( false );
			}
		}
		CurrentFolder.Append( '\\' );
	}

	return EGOsFile_DoesDirExist( DirPath );
}

eg_bool EGOsFile_CopyFile( eg_cpstr16 SourceFile , eg_cpstr16 DestFile )
{
	const eg_d_string16 DestDir = EGPath2_BreakPath( DestFile ).GetDirectory();
	eg_bool bHasDirectory = true;
	if( !EGOsFile_DoesDirExist( *DestDir ) )
	{
		bHasDirectory = EGOsFile_CreateDirectory( *DestDir );
	}

	if( !bHasDirectory )
	{
		return false;
	}

	return TRUE == CopyFileW( SourceFile , DestFile , FALSE );
}

eg_bool EGOsFile_CopyFile( eg_cpstr8 SourceFile , eg_cpstr8 DestFile )
{
	return EGOsFile_CopyFile( *eg_d_string16(SourceFile) , *eg_d_string16(DestFile) );
}

eg_bool EGOsFile_CopyDirectory( eg_cpstr16 SourceDir , eg_cpstr16 DestDir )
{
	if( EGOsFile_DoesDirExist( DestDir ) )
	{
		EGOsFile_ClearDirectory( DestDir );
	}
	else
	{
		EGOsFile_CreateDirectory( DestDir );
	}

	if( !EGOsFile_DoesDirExist( DestDir ) )
	{
		return false;
	}

	egOsFileInfo DirTree;
	EGOsFile_BuildTree( SourceDir , nullptr , DirTree );

	DirTree.WalkTree( [&SourceDir,&DestDir]( eg_cpstr16 Filename , const egOsFileAttributes& FileAttr ) -> void
	{
		const eg_d_string16 Source = EGPath2_CleanPath( *EGSFormat16( L"{0}/{1}" , SourceDir , Filename ) , '/' );
		const eg_d_string16 Dest = EGPath2_CleanPath( *EGSFormat16( L"{0}/{1}" , DestDir , Filename ) , '/' );

		if( FileAttr.bDirectory )
		{

		}
		else
		{
			EGLogf( eg_log_t::General , "Copying %s -> %s..." , *eg_d_string8(*Source) , *eg_d_string8(*Dest) );
			EGOsFile_CopyFile( *Source , *Dest );
		}
	} );

	return false;
}

eg_bool EGOsFile_CopyDirectory( eg_cpstr8 SourceDir , eg_cpstr8 DestDir )
{
	return EGOsFile_CopyDirectory( *eg_d_string16(SourceDir) , *eg_d_string16(DestDir) );
}

eg_bool EGOsFile_DoesFileExist( eg_cpstr8 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesA( Path );
	return INVALID_FILE_ATTRIBUTES != TargetFileAttributes && 0 == (TargetFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
}

eg_bool EGOsFile_DoesFileExist( eg_cpstr16 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesW( Path );
	return INVALID_FILE_ATTRIBUTES != TargetFileAttributes && 0 == ( TargetFileAttributes&FILE_ATTRIBUTE_DIRECTORY );
}

eg_bool EGOsFile_RemoveReadOnlyAttribute( eg_cpstr8 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesA( Path );
	if( INVALID_FILE_ATTRIBUTES == TargetFileAttributes )
	{
		return false;
	}

	if( 0 != (TargetFileAttributes & FILE_ATTRIBUTE_READONLY) )
	{
		const DWORD NewAttributes = TargetFileAttributes & ~FILE_ATTRIBUTE_READONLY;
		const BOOL Res = SetFileAttributesA( Path , NewAttributes );
		return Res != 0;
	}

	return true;
}

eg_bool EGOsFile_RemoveReadOnlyAttribute( eg_cpstr16 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesW( Path );
	if( INVALID_FILE_ATTRIBUTES == TargetFileAttributes )
	{
		return false;
	}

	if( 0 != (TargetFileAttributes & FILE_ATTRIBUTE_READONLY) )
	{
		const DWORD NewAttributes = TargetFileAttributes & ~FILE_ATTRIBUTE_READONLY;
		const BOOL Res = SetFileAttributesW( Path , NewAttributes );
		return Res != 0;
	}

	return true;
}

eg_bool EGOsFile_DoesDirExist( eg_cpstr8 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesA( Path );
	return INVALID_FILE_ATTRIBUTES != TargetFileAttributes && 0 != (TargetFileAttributes&FILE_ATTRIBUTE_DIRECTORY);
}

eg_bool EGOsFile_DoesDirExist( eg_cpstr16 Path )
{
	const DWORD TargetFileAttributes = GetFileAttributesW( Path );
	return INVALID_FILE_ATTRIBUTES != TargetFileAttributes && 0 != ( TargetFileAttributes&FILE_ATTRIBUTE_DIRECTORY );
}

static void EGOsFile_FindAllFiles_Recursive( eg_cpstr RootDir , EGArray<egOsFileFoundFile>& Out )
{
	eg_string_big strSearch = RootDir;
	strSearch.Append( '*' );

	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA( strSearch.String() , &fd );
	if( INVALID_HANDLE_VALUE == hFind )
	{
		return;
	}

	do
	{
		if( fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
		{
			if( fd.cFileName[0] == '.' )
			{
				continue;
			}

			eg_string_big NextRoot = EGString_Format( "%s%s\\" , RootDir , fd.cFileName );
			EGOsFile_FindAllFiles_Recursive( NextRoot , Out );
			continue;
		}

		eg_string_big FoundFile = EGString_Format( "%s%s" , RootDir , fd.cFileName );
		Out.Append( egOsFileFoundFile( FoundFile , fd.dwFileAttributes ) );
	} while( FindNextFileA( hFind , &fd ) );

	FindClose( hFind );
}

void EGOsFile_FindAllFiles( eg_cpstr RootDir, eg_cpstr TargetDir, const EGArray<eg_string_small>& WantedTypes, const EGArray<eg_string_small>& IgnorePaths , EGArray<eg_string_big>& Out )
{
	eg_d_string8 StrSearch = EGPath2_CleanPath( EGString_Format( "%s/%s/" , RootDir , TargetDir ) , '/' );

	EGArray<eg_d_string8> RelativeIgnorePaths;
	for( const eg_string_small& IngorePath : IgnorePaths )
	{
		eg_d_string8 RelativeIgnorePath = EGPath2_GetRelativePathTo( IngorePath.String() , TargetDir );
		RelativeIgnorePaths.Append( RelativeIgnorePath );
	}

	auto ShouldIgnore = [&RelativeIgnorePaths]( const eg_cpstr8 FileToCheck ) -> eg_bool
	{
		for( const eg_d_string8& IgnorePath : RelativeIgnorePaths )
		{
			if( EGString_EqualsCount(FileToCheck , *IgnorePath , IgnorePath.Len() ) )
			{
				return true;
			}
		}
		return false;
	};
	
	EGOsFileWantsFileFn WantsFile = [&ShouldIgnore,TargetDir,WantedTypes,IgnorePaths,Out]( eg_cpstr Filename , const egOsFileAttributes& FileAttr ) -> eg_bool
	{
		egPathParts2 PathParts = EGPath2_BreakPath( Filename );
		return !FileAttr.bDirectory && !FileAttr.bSystem && WantedTypes.Contains( *PathParts.Ext ) && !ShouldIgnore( Filename );
	};

	EGOsFile_FindAllFiles( *StrSearch , WantsFile , Out );
	if( TargetDir && TargetDir[0] != '\0' )
	{
		for( eg_string_big& Filename : Out )
		{
			Filename = *EGPath2_CleanPath( EGString_Format( "%s/%s" , TargetDir , Filename.String() ) , '/' );
		}
	}
}

void EGOsFile_FindAllFiles( eg_cpstr RootDir , EGOsFileWantsFileFn WantsFile , EGArray<eg_string_big>& Out )
{
	eg_string_big StrSearch = RootDir;
	StrSearch += '/';
	StrSearch = *EGPath2_CleanPath( StrSearch , '/' );
	EGArray<egOsFileFoundFile> AllFiles;
	EGOsFile_FindAllFiles_Recursive( StrSearch , AllFiles );

	for( const egOsFileFoundFile& File : AllFiles )
	{
		eg_string_big RelativeFilename = *EGPath2_GetRelativePathTo( File.Filename , StrSearch );
		if( nullptr != WantsFile )
		{
			if(WantsFile( RelativeFilename , egOsFileAttributes(File.FileAttr) ) )
			{
				Out.Append( RelativeFilename );
			}
		}
		else
		{
			Out.Append( RelativeFilename );
		}
	}
}

void EGOsFile_FindAllFiles( eg_cpstr RootDir, EGOsFileWantsFileFn WantsFile, EGArray<eg_d_string>& Out )
{
	eg_string_big StrSearch = RootDir;
	StrSearch += '/';
	StrSearch = *EGPath2_CleanPath( StrSearch , '/' );
	EGArray<egOsFileFoundFile> AllFiles;
	EGOsFile_FindAllFiles_Recursive( StrSearch , AllFiles );

	for( const egOsFileFoundFile& File : AllFiles )
	{
		eg_d_string RelativeFilename = *EGPath2_GetRelativePathTo( File.Filename , StrSearch );
		if( nullptr != WantsFile )
		{
			if(WantsFile( *RelativeFilename , egOsFileAttributes(File.FileAttr) ) )
			{
				Out.Append( RelativeFilename );
			}
		}
		else
		{
			Out.Append( RelativeFilename );
		}
	}
}

static void EGOsFile_BuildTree_GetItemsInDirectory( eg_cpstr16 RootDir , egOsFileInfo& RootInfo )
{
	// EGLogf( eg_log_t::General , "Found: %s" , EGString_ToMultibyte( RootDir ) );

	// We presume the path has already been cleaned.
	eg_d_string16 FullPath = RootDir;
	FullPath.Append( *RootInfo.Name );
	FullPath.Append( "/" );
	FullPath = EGPath2_CleanPath( *FullPath , '/' );
	eg_d_string16 SearchStr = FullPath + "*";

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = FindFirstFileW( *SearchStr , &FindData );
	if( INVALID_HANDLE_VALUE == hFind )
	{
		return;
	}

	do
	{
		egOsFileInfo FoundObject;
		FoundObject.Name = FindData.cFileName;
		FoundObject.Attributes = egOsFileAttributes( FindData.dwFileAttributes );
		FoundObject.FileSize = (FindData.nFileSizeHigh * (MAXDWORD+1)) + FindData.nFileSizeLow;

		if( FindData.cFileName[0] == '.' )
		{
			continue;
		}

		RootInfo.Children.Append( std::move(FoundObject) );

	} while( FindNextFileW( hFind, &FindData ) );

	FindClose( hFind );
}

static void EGOsFile_BuildTree_FindFilesRecursive( eg_cpstr16 RootDir , egOsFileInfo& RootInfo , EGOsFileWantsFileFn16& WantsFile )
{
	// EGLogf( eg_log_t::General , "Found: %s" , EGString_ToMultibyte( RootDir ) );

	// We presume the path has already been cleaned.
	eg_d_string16 FullPath = RootDir;
	FullPath.Append( *RootInfo.Name );
	FullPath.Append( "/" );
	FullPath = EGPath2_CleanPath( *FullPath , '/' );
	eg_d_string16 SearchStr = FullPath + "*";

	WIN32_FIND_DATAW FindData;
	HANDLE hFind = FindFirstFileW( *SearchStr , &FindData );
	if( INVALID_HANDLE_VALUE == hFind )
	{
		return;
	}

	do
	{
		egOsFileInfo FoundObject;
		FoundObject.Name = FindData.cFileName;
		FoundObject.Attributes = egOsFileAttributes( FindData.dwFileAttributes );
		FoundObject.FileSize = (FindData.nFileSizeHigh * (MAXDWORD+1)) + FindData.nFileSizeLow;

		if( FoundObject.Attributes.bDirectory )
		{
			if( FindData.cFileName[0] == '.' )
			{
				continue;
			}

			if( nullptr == WantsFile || WantsFile( *FoundObject.Name , FoundObject.Attributes ) )
			{
				EGOsFile_BuildTree_FindFilesRecursive( *FullPath , FoundObject , WantsFile );
				RootInfo.Children.Append( std::move(FoundObject) );
			}
			continue;
		}
		else
		{
			if( nullptr == WantsFile || WantsFile( *FoundObject.Name , FoundObject.Attributes ) )
			{
				RootInfo.Children.Append( std::move(FoundObject) );
			}
		}
	} while( FindNextFileW( hFind, &FindData ) );

	FindClose( hFind );
}

void EGOsFile_BuildTree( eg_cpstr16 RootDir, EGOsFileWantsFileFn16 WantsFile , egOsFileInfo& TreeOut )
{
	if( EGOsFile_DoesDirExist( RootDir ) )
	{
		TreeOut.Name += L"/";
		TreeOut.Name = RootDir;
		TreeOut.Name = EGPath2_CleanPath( *TreeOut.Name , '/' );
		TreeOut.Attributes.bDirectory = true;
		TreeOut.FileSize = 0;
		TreeOut.Children.Clear();

		EGOsFile_BuildTree_FindFilesRecursive( L"" , TreeOut , WantsFile ); // Note we don't pass in the root directory because that is already in egOsFileInfo Out;
	}
	else
	{
		TreeOut.Name = "";
		TreeOut.Attributes = egOsFileAttributes();
		TreeOut.FileSize = 0;
		TreeOut.Children.Clear();
	}
}

void EGOsFile_GetDirectoryContents( eg_cpstr16 RootDir , egOsFileInfo& TreeOut )
{	
	if( EGOsFile_DoesDirExist( RootDir ) )
	{
		TreeOut.Name += L"/";
		TreeOut.Name = RootDir;
		TreeOut.Name = EGPath2_CleanPath( *TreeOut.Name , '/' );
		TreeOut.Attributes.bDirectory = true;
		TreeOut.FileSize = 0;
		TreeOut.Children.Clear();

		EGOsFile_BuildTree_GetItemsInDirectory( L"" , TreeOut ); // Note we don't pass in the root directory because that is already in egOsFileInfo Out;
	}
	else
	{
		TreeOut.Name = "";
		TreeOut.Attributes = egOsFileAttributes();
		TreeOut.FileSize = 0;
		TreeOut.Children.Clear();
	}
}

void egOsFileInfo::GetDirectoryContents( eg_cpstr16 Path , EGArray<egOsFileInfo>& Out ) const
{
	eg_d_string16 AdjustedDir = Path;
	AdjustedDir.Append( "/" );
	egPathParts2 PathParts = EGPath2_BreakPath( *AdjustedDir );
	const egOsFileInfo* CurrentInfo = this;
	for( const eg_d_string16& Part : PathParts.Folders )
	{
		if( Part.Len() == 0 )
		{
			break;
		}
		if( CurrentInfo )
		{
			bool bFoundDir = false;
			for( const egOsFileInfo& Info : CurrentInfo->Children )
			{
				if( Info.Attributes.bDirectory && Info.Name.EqualsI( *Part ) )
				{
					CurrentInfo = &Info;
					bFoundDir = true;
				}
			}
			if( !bFoundDir )
			{
				CurrentInfo = nullptr;
				break;
			}
		}
	}
	if( CurrentInfo )
	{
		for( const egOsFileInfo& Info : CurrentInfo->Children )
		{
			egOsFileInfo NewInfo;
			NewInfo.Name = Info.Name;
			NewInfo.Attributes = Info.Attributes;
			NewInfo.FileSize = Info.FileSize;
			// Don't copy children, we don't care
			Out.Append( NewInfo );
		}
	}
}

void egOsFileInfo::DebugPrint( eg_cpstr Prefix , eg_uint Depth /*= 0*/ ) const
{
	eg_d_string8 DisplayString;
	for( eg_uint i=0; i<Depth; i++ )
	{
		DisplayString += Prefix;
	}
	DisplayString += *Name;
	EGLogf( eg_log_t::General , "%s" , *DisplayString );
	for( const egOsFileInfo& Child : Children )
	{
		Child.DebugPrint( Prefix , Depth + 1 );
	}
}

void egOsFileInfo::WalkTree( EGOsFileWalkFn16 Function , eg_cpstr16 Prefix /*= nullptr */ ) const
{
	eg_d_string16 FullFilename = Prefix;
	if( Prefix )
	{
		if( FullFilename.Len() > 0 )
		{
			FullFilename += "/";
		}
		FullFilename += Name;
		Function( *FullFilename , Attributes );
	}

	for( const egOsFileInfo& Child : Children )
	{
		Child.WalkTree( Function , *FullFilename );
	}
}
