// (c) 2018 Beem Media

#pragma once

#include "EGCppLibAPI.h"

struct egOsFileAttributes
{
	eg_bool bDirectory:1;
	eg_bool bHidden:1;
	eg_bool bReadOnly:1;
	eg_bool bSystem:1;
	eg_bool bTemp:1;

	egOsFileAttributes()
	: bDirectory( false )
	, bHidden( false )
	, bReadOnly( false )
	, bSystem( false )
	, bTemp( false )
	{
	}

	explicit egOsFileAttributes( eg_uint32 OsFlagsIn );
};
typedef std::function<eg_bool(eg_cpstr Filename,const egOsFileAttributes& FileAttr)> EGOsFileWantsFileFn;
typedef std::function<eg_bool(eg_cpstr16 Filename,const egOsFileAttributes& FileAttr)> EGOsFileWantsFileFn16;
typedef std::function<void(eg_cpstr16 Filename,const egOsFileAttributes& FileAttr)> EGOsFileWalkFn16;

void EGOsFile_SetCWD( eg_cpstr8 RootDir );
void EGOsFile_SetCWD( eg_cpstr16 RootDir );
eg_d_string16 EGOsFile_GetCWD();
void EGOsFile_DeleteFile( eg_cpstr16 Filename );
void EGOsFile_DeleteFile( eg_cpstr8 Filename );
void EGOsFile_DeleteDirectory( eg_cpstr16 Directory );
void EGOsFile_DeleteDirectory( eg_cpstr8 Directory );
void EGOsFile_ClearDirectory( eg_cpstr16 Directory );
void EGOsFile_ClearDirectory( eg_cpstr8 Directory );
eg_bool EGOsFile_CreateDirectory( eg_cpstr8 DirPath );
eg_bool EGOsFile_CreateDirectory( eg_cpstr16 DirPath );
eg_bool EGOsFile_CopyFile( eg_cpstr16 SourceFile , eg_cpstr16 DestFile );
eg_bool EGOsFile_CopyFile( eg_cpstr8 SourceFile , eg_cpstr8 DestFile );
eg_bool EGOsFile_CopyDirectory( eg_cpstr16 SourceDir , eg_cpstr16 DestDir );
eg_bool EGOsFile_CopyDirectory( eg_cpstr8 SourceDir , eg_cpstr8 DestDir );
eg_bool EGOsFile_DoesFileExist( eg_cpstr8 Path );
eg_bool EGOsFile_DoesFileExist( eg_cpstr16 Path );
eg_bool EGOsFile_RemoveReadOnlyAttribute( eg_cpstr8 Path);
eg_bool EGOsFile_RemoveReadOnlyAttribute( eg_cpstr16 Path);
eg_bool EGOsFile_DoesDirExist( eg_cpstr8 Path );
eg_bool EGOsFile_DoesDirExist( eg_cpstr16 Path );
void EGOsFile_FindAllFiles( eg_cpstr RootDir , eg_cpstr TargetDir , const EGArray<eg_string_small>& WantedTypes , const EGArray<eg_string_small>& IgnorePaths , EGArray<eg_string_big>& Out );
void EGOsFile_FindAllFiles( eg_cpstr RootDir , EGOsFileWantsFileFn WantsFile , EGArray<eg_string_big>& Out );
void EGOsFile_FindAllFiles( eg_cpstr RootDir , EGOsFileWantsFileFn WantsFile , EGArray<eg_d_string>& Out );

struct egOsFileInfo
{
	eg_d_string16         Name;
	egOsFileAttributes    Attributes;
	eg_size_t             FileSize = 0;
	EGArray<egOsFileInfo> Children;

	template<typename LessThanFn>
	void Sort( LessThanFn& IsLessThan )
	{
		Children.Sort( IsLessThan );
		for( egOsFileInfo& Child : Children )
		{
			Child.Sort( IsLessThan );
		}
	}

	void GetDirectoryContents( eg_cpstr16 Path , EGArray<egOsFileInfo>& Out ) const;
	void DebugPrint( eg_cpstr Prefix , eg_uint Depth = 0 ) const;
	void WalkTree( EGOsFileWalkFn16 Function , eg_cpstr16 Prefix = nullptr ) const;
};

void EGOsFile_BuildTree( eg_cpstr16 RootDir , EGOsFileWantsFileFn16 WantsFile , egOsFileInfo& TreeOut );
void EGOsFile_GetDirectoryContents( eg_cpstr16 RootDir , egOsFileInfo& TreeOut );