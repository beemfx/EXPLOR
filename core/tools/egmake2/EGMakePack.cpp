// (c) 2017 Beem Media

#include <fs_sys2/fs_lpk.h>
#include "EGStdLibAPI.h"
#include "EGWindowsAPI.h"

static void* EGPack_Malloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line)
{
	unused( reason , type , file , line );
	return new fs_byte[size];
}

static void EGPack_Free(void* p, LF_ALLOC_REASON reason)
{
	unused( reason );
	delete[]p;
}

typedef eg_string_fixed_size<1024> CString;

static eg_bool g_bIsQuiet = false;
static EGArray<CString> g_FoundFiles;

static eg_bool g_bCompress=true;
static eg_bool g_bUnpack=false;

void EGPack_AddFiles(eg_cpstr strPath, eg_cpstr strBasePath, CLArchive* pAR)
{
	CString strSearch(strPath);
	strSearch.Append( "\\*" );
	WIN32_FIND_DATAA fd;
	HANDLE hFind=FindFirstFileA(strSearch.String(), &fd);
	if(!hFind)
		return;
		
	EGLogf( eg_log_t::General , "Copying files from \"%s\"..." , strSearch.String() );
	
	do
	{
		if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(fd.cFileName[0]=='.')
				continue;
				
			CString strRecDir(strPath);
			CString strRecBase(strBasePath);
			
			strRecDir+="\\";
			strRecDir+=fd.cFileName;
			
			strRecBase+=fd.cFileName;
			strRecBase+="/";
			EGPack_AddFiles(strRecDir.String(), strRecBase.String(), pAR);
			continue;
		}
		
		CString strOSFile, strArcFile;
		strOSFile=strPath;
		strOSFile+="\\";
		strOSFile+=fd.cFileName;
		strArcFile=strBasePath;
		strArcFile+=fd.cFileName;
		
		EGLogf( eg_log_t::Verbose , "\tCopying \"%s\" to \"%s\"..." , strOSFile.String(), strArcFile.String());
		pAR->AddFile(EGString_ToWide(strOSFile) , EGString_ToWide(strArcFile) , g_bCompress?LPK_ADD_ZLIBCMP:0);
		g_FoundFiles.Push( strArcFile );
	}while(FindNextFileA(hFind, &fd));

	FindClose(hFind);
	return;
}

int EGPack_main( int argc , char *argv[] )
{
	enum CURRENT_ITEM{ITEM_NONE, ITEM_SOURCE, ITEM_DEST, ITEM_COMPRESS, ITEM_MANIFEST, ITEM_LPKPATH };
	CURRENT_ITEM nItem=ITEM_NONE;
	eg_cpstr strSource=NULL, strDest=NULL, strManifest=NULL;
	eg_bool bIsUnpack = false;
	eg_char strLpkPath[1024];
	EGString_Copy( strLpkPath , "" , countof(strLpkPath) );
	
	for(int i=0; i<argc; i++)
	{
		if(argv[i][0]=='-')
		{
			if(EGString_EqualsI(argv[i], "-source") || EGString_EqualsI(argv[i], "-s"))
				nItem=ITEM_SOURCE;
			else if(EGString_EqualsI(argv[i], ("-dest")) || EGString_EqualsI(argv[i], ("-d")))
				nItem=ITEM_DEST;
			else if(EGString_EqualsI(argv[i], ("-compress")) || EGString_EqualsI(argv[i], ("-c")))
				nItem=ITEM_COMPRESS;
			else if(EGString_EqualsI(argv[i], ("-quiet")) || EGString_EqualsI(argv[i], ("-q")))
				g_bIsQuiet = true;
			else if(EGString_EqualsI(argv[i], ("-manifest")) || EGString_EqualsI(argv[i], ("-mf")))
				nItem=ITEM_MANIFEST;
			else if(EGString_EqualsI(argv[i], ("-lpkpath")) || EGString_EqualsI(argv[i], ("-lp")))
				nItem=ITEM_LPKPATH;
			else if( EGString_EqualsI(argv[i], "-unpack") || EGString_EqualsI(argv[i], "-u"))
				g_bUnpack = true;
		}
		else
		{
			if(ITEM_SOURCE==nItem)
			{
				strSource=argv[i];
			}
			else if(ITEM_DEST==nItem)
			{
				strDest=argv[i];
			}
			else if( ITEM_LPKPATH ==nItem )
			{
				EGString_Copy( strLpkPath , argv[i] , countof(strLpkPath) );
				//If the last char is not a / then put in a /
				size_t Len = EGString_StrLen( strLpkPath );
				if( Len > 0 && strLpkPath[Len-1] != '/' )
				{
					strLpkPath[Len] = '/';
					strLpkPath[Len+1] = 0;
				}
			}
			else if(ITEM_COMPRESS==nItem)
			{
				if(EGString_EqualsI(argv[i], ("true")))
					g_bCompress=TRUE;
				else
					g_bCompress=FALSE;
			}
			else if(ITEM_MANIFEST==nItem)
			{
				strManifest=argv[i];
			}
			
			nItem=ITEM_NONE;
		}
	}
	
	if(!strSource || !strDest)
	{
		g_bIsQuiet = false;
		EGLogf( eg_log_t::General , "PACK -source \"string_source_dir\" -dest \"dest_file_path\" [-compress \"true\"|\"false\" -quiet -manifest \"dest_manifest_file_path\" -lpkpath \"lpk_path\"]" );
		return 0;
	}
	
	EGLogf( eg_log_t::General , "Packing \"%s\" to \"%s\"%s...", strSource, strDest, g_bCompress ? " (Compressing)" : "");
	
	//First attempt to open the destination file:
	FS_SetMemFuncs(EGPack_Malloc, EGPack_Free);
	::CLArchive ar;
	if(!ar.CreateNew( EGString_ToWide(strDest)))
	{
		EGLogf( eg_log_t::General , "Failed to open \"%s\" for writing." , strDest );
		return 0;
	}
	
	EGPack_AddFiles(strSource, strLpkPath, &ar);
	
	ar.Close();

	if( NULL != strManifest )
	{
		FILE* ManFile = nullptr;
		errno_t fopenRes = fopen_s( &ManFile , strManifest , ("wb") );
		if( 0 == fopenRes && nullptr != ManFile )
		{
			for( fs_uint i=0; i<g_FoundFiles.LenAs<fs_uint>(); i++ )
			{
				const eg_char NewLine[] = "\r\n";
				fwrite( g_FoundFiles[i].String() , sizeof(eg_char), g_FoundFiles[i].Len(), ManFile );
				fwrite( NewLine, sizeof(eg_char), 2, ManFile );
			}
			fclose( ManFile );
			ManFile = nullptr;
		}
	}

	return 0;
}