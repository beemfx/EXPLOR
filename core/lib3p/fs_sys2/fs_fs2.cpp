#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fs_fs2.h"
#include "fs_bdio.h"
#include "fs_lpk.h"

#if defined(DEBUG)
#include <ctype.h>
#endif

//NOTES: The file system is fairly simply, but most of the methods have a
//wide character and multibyte method that way the file system is compatible
//both with Windows 98 and unicode operating systems.  The file system stores
//information about all files in a partition table that uses hash indexes to
//quickly access information about each of the files.

//TODO: Testing on windows 98
//Inusuring complete testing, and better support for the multi-byte methods.


//Typical mounting procedure.
//1) The first thing that needs to be done is to have the base get mounted.
//eg. 
// MountBase(".\"); //This would mount the current directory.
//This will get the base file system mounted.
//2) Mount any subdirectories.
//eg.
// Mount("base", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// Mount("expansionbase", "/base2/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// This will get the main folders mounted for the application.  Note that
// You could do something as follows:
// Mount("base", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// Mount("expansionbase", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// And that all files in the OS folder expansionbase would take precedence over
// any files in base, but the same mount path would be used no matter what.
//3) Mount any LPK files.
//eg.
// MountLPK("/base1/pak0.lpk", MOUNT_FILE_OVERWRITELPKONLY);
// MountLPK("/base1/pak1.lpk", MOUNT_FILE_OVERWRITELPKONLY);
// MountLPK("/base1/pak2.lpk", MOUNT_FILE_OVERWTITELPKONLY);
// Note that by specifying MOUNT_FILE_OVERWRITELPKONLY any files
// in pak1 would overwrite files in pak0, but not any files that
// are directly on the hard drive.
//4) Proceed to open, close, read, and write files as necessary.

CLFileSystem::CLFileSystem()
: m_PrtTableList(FS_NULL)
, m_bBaseMounted(FS_FALSE)
{
	m_szBasePath[0]=0;
}

CLFileSystem::~CLFileSystem()
{
	UnMountAll();
}

void CLFileSystem::PrintMountInfo(FS_PRINT_MOUNT_INFO_TYPE Type)
{
	if( PRINT_MOUNT_INFO_FULL == Type )
	{
		FS_ErrPrint(L"Base mounted at \"%s\"", ERR_LEVEL_ALWAYS, this->m_szBasePath);
		FS_ErrPrint(L"Mounted Files:", ERR_LEVEL_ALWAYS);
	}
	for( CPartTable* Table = m_PrtTableList; FS_NULL != Table; Table = Table->GetNext() )
	{
		Table->PrintMountInfo(Type);
	}
}

CLFileSystem::MOUNT_FILE* CLFileSystem::GetFileInfo( fs_cstr szMountPath , CPartTable** OwnerTable )
{
	fs_path sFixed;
	FS_FixCase(sFixed, szMountPath);
	MOUNT_FILE* FoundFile = FS_NULL;
	for( CPartTable* Table = m_PrtTableList; FS_NULL != Table && FS_NULL == FoundFile; Table = Table->GetNext() )
	{
		FoundFile = Table->FindFile(sFixed);
		if( FS_NULL != FoundFile && FS_NULL != OwnerTable )
		{
			*OwnerTable = Table;
		}
	}
	return FoundFile;
}


//Mount base must be called before any other mounting operations are called.
//Mount base basically sets the current directory.  From there on out any 
//calls to mount don't need full path names they only need path names
//relative to the OS path that was mounted at the base.  When MountBase is
//called any files immediately in the base directory are mounted, but not
//subdirectories.  Note that even if a base is mounted, files outside the
//base may still be mounted (for example a CD-ROM drive can be mounted).
//The path to the base is always /.  Note that any files that are created
//are always created relatvie to the base, and not other paths that may
//be mounted.  I want to change that, however, so that a path can be mounted
//to where files should be saved.

fs_dword CLFileSystem::MountBase(fs_cstr szOSPath)
{
	if(m_bBaseMounted)
	{
		FS_ErrPrint(L"MountBase Error 1: The base file system is already mounted.  Call UnMountAll before mounting the base.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	if(!BDIO_ChangeDir(szOSPath))
	{
		FS_ErrPrint(L"MountBase Error 2: Could not mount base file system.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	BDIO_GetCurrentDir(m_szBasePath, FS_MAX_PATH);	
	m_bBaseMounted=FS_TRUE;
	fs_dword nMountCount=Mount(L".", L"/", 0);
	FS_ErrPrint(L"Mounted \"%s\" to \"/\"", ERR_LEVEL_DETAIL, m_szBasePath);
	return nMountCount;
}

//MountLPK mounts a Legacy PaKage file into the file system.  Note that before
//an LPK can be mounted the actually LPK file must be within the file system.
//Then when MountLPK is called the path to the mounted file (e.g. /base1/pak0.lpk)
//should be specifed as a parameter, and not the os path (e.g. NOT .\base\pak0.lpk).
//This method will expand the archive file into the current directory, so if
//a file was stored in the LPK as Credits.txt it would now be /base1/Credits.txt
//and Textures/Tex1.tga would be /base1/Textures/Tex1.tga.
//Note that for the flags either MOUNT_FILE_OVERWRITE or MOUNT_FILE_OVERWRITELPKONLY
//may be specifed.
//
//MOUNT_FILE_OVERWRITE:  If any file with the same mout path as the new file
//already exists, then the new file will take precedence over the old file
//that is to say that when calls to open a file with the specified name
//are made the file most recently mounted file will be opened (it doens't
//mean that the OS file will be overwritten, because it won't).
//
//MOUNT_FILE_OVERWRITELPKONLY: Is similar to the above flag, but a newer file
//will only take precedence over other archived files.  So fore example if two
//LPK files have a file with the same name stored in them, the most recently
//mounted LPK file will take precedence, but if there was a file directly on
//the hard drive, the hard drive file will take precedence, whether or not
//the archive was mounted first or second.
//
//If neither flag is specified files will never get overwritten and the first
//file to be mounted will always take precidence.

fs_dword CLFileSystem::MountLPK(fs_cstr szMountedFileOrg, fs_dword Flags)
{
	fs_path szMountedFile;
	FS_FixCase(szMountedFile, szMountedFileOrg);

	MOUNT_FILE* pMountFile=GetFileInfo(szMountedFile,FS_NULL);
	if(!pMountFile)
	{
		FS_ErrPrint(L"MountLPK Error 5: The archive file \"%s\" has not been mounted.", ERR_LEVEL_ERROR, szMountedFile);
		return 0;
	}
	fs_path szMountDir;
	FS_GetDirFromPath(szMountDir, szMountedFile);
	FS_ErrPrint(L"Expanding \"%s\" to \"%s\"", ERR_LEVEL_NOTICE, szMountedFile, szMountDir);
	
	CLArchive lpkFile;
	if(!lpkFile.Open(pMountFile->szOSFileW))
	{
		FS_ErrPrint(L"Could not mount \"%s\".  Possibly not a valid archive.", ERR_LEVEL_ERROR, szMountedFile);
		return 0;
	}

	pMountFile->Flags |= MOUNT_FILE::MOUNT_FILE_READONLY;
	
	fs_dword dwMounted=0;
	for(fs_dword i=0; i<lpkFile.GetNumFiles(); i++)
	{
		LPK_FILE_INFO fileInfo;
		if(!lpkFile.GetFileInfo(i, &fileInfo))
			continue;
		
		MOUNT_FILE mountFile;
		fs_path CorrectedFilename;
		FS_FixCase( CorrectedFilename , fileInfo.szFilename );
		swprintf(mountFile.szMountFileW, FS_MAX_PATH, L"%s%s", szMountDir, CorrectedFilename);
		swprintf(mountFile.szOSFileW, FS_MAX_PATH, L"%s", pMountFile->szOSFileW);
		mountFile.nSize=fileInfo.nSize;
		mountFile.nCmpSize=fileInfo.nCmpSize;
		mountFile.nOffset=fileInfo.nOffset;	
		mountFile.Flags=MOUNT_FILE::MOUNT_FILE_ARCHIVED|MOUNT_FILE::MOUNT_FILE_READONLY;
		if(fileInfo.nType==LPK_FILE_TYPE_ZLIBCMP)
			mountFile.Flags|=MOUNT_FILE::MOUNT_FILE_ZLIBCMP;
		
		if(MountFile(&mountFile, Flags))
			dwMounted++;
		else
		{
			FS_ErrPrint(
				L"Mount Error 6: Could not mount \"%s\" (%s).", 
					ERR_LEVEL_ERROR, 
					mountFile.szMountFileW,
					mountFile.szOSFileW);
		}
	}
	return dwMounted;
}

fs_dword CLFileSystem::MountDir(fs_cstr szOSPath, fs_cstr szMount, fs_dword Flags)
{
	fs_dword nMountCount=0;
	fs_bool bRes=0;
	
	fs_path szSearchPath;
	_snwprintf(szSearchPath, FS_MAX_PATH, L"%s%c*", szOSPath, BDIO_GetOsPathSeparator());
	//Recursivley mount all files...
	BDIO_FIND_DATAW sData;
	BDIO_FIND hFind=BDIO_FindFirstFile(szSearchPath, &sData);
	if(!hFind)
	{
		FS_ErrPrint(
			L"Mount Error 9: Could not mount any files.  \"%s\" may not exist.",
			ERR_LEVEL_ERROR,
			szOSPath);
		return 0;
	}
	//Mount the directory, itself...
	MOUNT_FILE mountInfo;
	mountInfo.nCmpSize=mountInfo.nOffset=mountInfo.nSize=0;
	mountInfo.Flags=MOUNT_FILE::MOUNT_FILE_DIRECTORY;
	wcsncpy(mountInfo.szOSFileW, szOSPath, FS_MAX_PATH);
	wcsncpy(mountInfo.szMountFileW, szMount, FS_MAX_PATH);
	//Directories take the same overwrite priority, so overwritten
	//directories will be used when creating new files.
	bRes=MountFile(&mountInfo, Flags);
	if(!bRes)
	{
		FS_ErrPrint(L"Mount Error 10: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		BDIO_FindClose(hFind);
		return nMountCount;
	}
	
	do
	{
		//Generate the mounted path and OS path..
		MOUNT_FILE sMountFile;
		_snwprintf(sMountFile.szOSFileW, FS_MAX_PATH, L"%s%c%s", szOSPath, BDIO_GetOsPathSeparator(), sData.szFilename);
		_snwprintf(sMountFile.szMountFileW, FS_MAX_PATH, L"%s%s", szMount, sData.szFilename);
		FS_FixCase(sMountFile.szMountFileW, sMountFile.szMountFileW);
			
		//If a directory was found mount that directory...
		if(sData.bDirectory)
		{
			//Ignore . and .. directories.
			if(sData.szFilename[0]=='.')
				continue;
				
			if(FS_CheckFlag(Flags, MOUNT_MOUNT_SUBDIRS))
			{
				wcsncat(sMountFile.szMountFileW, L"/", FS_min<fs_long>(1, FS_MAX_PATH-wcslen(sMountFile.szMountFileW)));
				nMountCount+=MountDir(sMountFile.szOSFileW, sMountFile.szMountFileW, Flags);
			}
		}
		else
		{
			if(sData.nFileSize.dwHighPart)
			{
				FS_ErrPrint(L"\"%s\" is too large to mount!", ERR_LEVEL_ERROR, sMountFile.szOSFileW);
				continue;
			}
			
			sMountFile.nSize=sData.nFileSize.dwLowPart;
			sMountFile.nCmpSize=sMountFile.nSize;
			sMountFile.nOffset=0;
			sMountFile.Flags=0;
			if(sData.bReadOnly)
				sMountFile.Flags|=MOUNT_FILE::MOUNT_FILE_READONLY;
			if(MountFile(&sMountFile, Flags))
				nMountCount++;
			else
				FS_ErrPrint(L"Mount Error 11: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		}
	}while(BDIO_FindNextFile(hFind, &sData));
	
	BDIO_FindClose(hFind);
	
	return nMountCount;
}

//The Mount method mounts a directory to the file
//system.  If the MOUNT_MOUNT_SUBDIRS flag is specifed
//then all subdirectories will be mounted as well.

fs_dword CLFileSystem::Mount(fs_cstr szOSPathToMount, fs_cstr szMountToPathOrg, fs_dword Flags)
{
	fs_path szMountToPath;
	FS_FixCase(szMountToPath, szMountToPathOrg);

	fs_dword nMountCount=0;
	fs_path szMount;
	fs_path szOSPath;
	
	//Get the full path to the specified directory...
	BDIO_GetFullPathName(szOSPath, szOSPathToMount);
	//Insure that the mounting path does not have a / or \.
	size_t nLast=wcslen(szOSPath)-1;
	if(szOSPath[nLast]==L'/' || szOSPath[nLast]==L'\\')
		szOSPath[nLast]=0;
	
	if(szMountToPath[0]!='/')
	{
		FS_ErrPrint(
			L"The path must be mounted at least to the base.  Try mount \"%s\" \"/%s\" instead.",
			ERR_LEVEL_WARNING,
			szOSPathToMount,
			szMountToPath);
			
		return 0;
	}
	
	//Insure that the mount path has a / at the end of it
	if(szMountToPath[wcslen(szMountToPath)-1]!=L'/')
		_snwprintf(szMount, FS_MAX_PATH, L"%s/", szMountToPath);
	else
		_snwprintf(szMount, FS_MAX_PATH, L"%s", szMountToPath);
		
	FS_ErrPrint(L"Mounting \"%s\" to \"%s\"...", ERR_LEVEL_NOTICE, szOSPath, szMount);
	
	return MountDir(szOSPath, szMount, Flags);
}

fs_bool CLFileSystem::Exists(fs_cstr szFilename)
{
	//Case is fixed by GetFileInfoW
	const MOUNT_FILE* pMountInfo=GetFileInfo( szFilename , FS_NULL );
	return pMountInfo!=FS_NULL;
}

fs_bool CLFileSystem::UnMountAll()
{
	FS_ErrPrint(L"Clearing partition table...", ERR_LEVEL_NOTICE);
	while( FS_NULL != m_PrtTableList )
	{
		CPartTable* OldTable = m_PrtTableList;
		m_PrtTableList = OldTable->GetNext();

		OldTable->Clear();
		delete OldTable;
	}
	FS_ErrPrint(L"Unmounting the base...", ERR_LEVEL_NOTICE);
	m_bBaseMounted=FS_FALSE;
	m_szBasePath[0]=0;
	return FS_TRUE;
}

fs_bool CLFileSystem::MountFile(MOUNT_FILE* pFile, fs_dword dwFlags)
{
	#if defined(DEBUG)
	{
		for(int i=0; i<FS_MAX_PATH; i++)
		{
			if(0 == pFile->szMountFileW[i])
				break;

			if( isupper(pFile->szMountFileW[i]) )
				__debugbreak();
		}
	}
	#endif
	FS_ErrPrint(L"Mounting \"%s\"", ERR_LEVEL_SUPERDETAIL, pFile->szMountFileW);

	//If the current partition table doesn't exist, or if it is full,
	//create a new one for this file.
	if( FS_NULL == m_PrtTableList || m_PrtTableList->IsFull() )
	{
		m_PrtTableList = new CPartTable( m_PrtTableList , PART_TABLE_SIZE );
	}

	if( (dwFlags&MOUNT_FILE_OVERWRITELPKONLY) != 0 )
	{
		CPartTable* PartTable = nullptr;
		if( MOUNT_FILE* MountFile = GetFileInfo( pFile->szMountFileW, &PartTable ) )
		{
			if( PartTable )
			{
				return FS_NULL != PartTable->MountFile(pFile, dwFlags);
			}
		}
	}

	return FS_NULL != m_PrtTableList ? m_PrtTableList->MountFile(pFile, dwFlags) : false;
}



/*********************************************************
	File opening code, note that files are opened and
	closed using the file system, but they are accessed
	(read and written to) using the CLFile class.
*********************************************************/
CLFile* CLFileSystem::OpenFile(fs_cstr szFilename, fs_dword Access, fs_dword CreateMode)
{
	fs_bool bOpenExisting=FS_TRUE;
	fs_bool bNew=FS_FALSE;
	
	//Check the flags to make sure they are compatible.
	if(FS_CheckFlag(Access, LF_ACCESS_WRITE) && FS_CheckFlag(Access, LF_ACCESS_MEMORY))
	{
		FS_ErrPrint(L"OpenFile Error: Cannot open a file with memory access and write access.", ERR_LEVEL_ERROR);
		return FS_NULL;
	}
	//Note that if creating a new file, then we shouldn't end here.
	const MOUNT_FILE* pMountInfo=GetFileInfo(szFilename,FS_NULL);
	if(CreateMode==LF_OPEN_EXISTING)
	{
		if(!pMountInfo)
		{
			FS_ErrPrint(L"OpenFile Warning: \"%s\" could not be found in the file system.", ERR_LEVEL_WARNING, szFilename);
			return FS_NULL;
		}
		bOpenExisting=FS_TRUE;
		bNew=FS_FALSE;
	}
	else if(CreateMode==LF_OPEN_ALWAYS)
	{
		bOpenExisting=pMountInfo?FS_TRUE:FS_FALSE;
		bNew=FS_FALSE;
	}
	else if(CreateMode==LF_CREATE_NEW)
	{
		if(pMountInfo)
		{
			FS_ErrPrint(L"OpenFile Error: Cannot create \"%s\", it already exists.", ERR_LEVEL_ERROR, szFilename);
			return FS_NULL;
		}
		bOpenExisting=FS_FALSE;
		bNew=FS_TRUE;
	}
	else if(CreateMode==LF_CREATE_ALWAYS)
	{
		if(pMountInfo)
		{
			bOpenExisting=FS_TRUE;
			bNew=FS_TRUE;
		}
		else
		{
			bOpenExisting=FS_FALSE;
			bNew=FS_TRUE;
		}
	}
	else
	{
		FS_ErrPrint(L"OpenFile Error: Invalid creation mode specifed.", ERR_LEVEL_ERROR);
		return FS_NULL;
	}
	
	//If the file is mounted then we are opening an existing file.
	if(bOpenExisting)
	{
		if(FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			FS_ErrPrint(L"OpenFile Error: \"%s\" is a directory, not a file.", ERR_LEVEL_ERROR, pMountInfo->szMountFileW);
			return FS_NULL;
		}
		//If we're trying to create a new file, and the existing file is read
		//only then we can't.
		if(FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY) && bNew)
		{
			FS_ErrPrint(L"OpenFile Error: \"%s\" cannot be created, because an existing file is read only.",
				ERR_LEVEL_ERROR,
				szFilename);
		
			return FS_NULL;
		}
		BDIO_FILE bdioFile=BDIO_Open(pMountInfo->szOSFileW, bNew?LF_CREATE_ALWAYS:LF_OPEN_EXISTING, Access);
		if(!bdioFile)
		{
			FS_ErrPrint(L"Could not open the OS file: \"%s\".", ERR_LEVEL_ERROR, pMountInfo->szOSFileW);
			return FS_NULL;
		}
		return OpenExistingFile(pMountInfo, bdioFile, Access, bNew);
	}
	else
	{
		//TODO: Should also check to see if it is likely that the
		//file exists (because some OSs are not case sensitive
		//in which case the file may exist, but the path specified
		//may be a mounted file.
		
		//If it is necessary to create a new file, then we need to prepare
		//the mount info for the new file, as well as create an OS file.
		MOUNT_FILE mountInfo;
		//Get the specifie directory for the file,
		//then find the information about that directory in
		//the partition table.
		fs_path szDir;
		FS_GetDirFromPath(szDir, szFilename);
		//The mount filename will be the same
		wcsncpy(mountInfo.szMountFileW, szFilename, FS_MAX_PATH);
		const MOUNT_FILE* pDirInfo=GetFileInfo(szDir,FS_NULL);
		if(!pDirInfo || !FS_CheckFlag(pDirInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			FS_ErrPrint(
				L"OpenFile Error: Cannot create the specified file, the directory \"%s\" does not exist.",
				ERR_LEVEL_ERROR,
				szDir);
				
			return FS_NULL;
		}
		//Get just the filename for the file,
		//this will be used to create the OS file.
		fs_path szFile;
		FS_GetFileNameFromPath(szFile, szFilename);
		FS_FixCase(szFile, szFile);
		//The OS filename is the information from the dir + the filename
		//Note that the OS separator must be inserted.
		_snwprintf(mountInfo.szOSFileW, FS_MAX_PATH, L"%s%c%s", pDirInfo->szOSFileW, BDIO_GetOsPathSeparator(), szFile);
		
		//The initial file information is all zeroes.
		mountInfo.Flags=0;
		mountInfo.nCmpSize=0;
		mountInfo.nSize=0;
		mountInfo.nOffset=0;
		
		//Create the OS file.
		BDIO_FILE bdioFile=BDIO_Open(mountInfo.szOSFileW, LF_CREATE_NEW, Access);
		if(!bdioFile)
		{
			FS_ErrPrint(
				L"OpenFile Error: Could not create new file: \"%s\".",
				ERR_LEVEL_ERROR,
				mountInfo.szMountFileW);
				
			return FS_NULL;
		}
		//If the OS file was created, then we can mount the new file
		//and open the existing file.
		MountFile(&mountInfo, MOUNT_FILE_OVERWRITE);
		return OpenExistingFile(&mountInfo, bdioFile, Access, FS_TRUE);
	}
}

CLFile* CLFileSystem::OpenExistingFile(const MOUNT_FILE* pMountInfo, BDIO_FILE File, fs_dword Access, fs_bool bNew)
{
	//First thing's first, allocate memory for the file.
	CLFile* pFile=new CLFile;
	if(!pFile)
	{
		FS_ErrPrint(L"OpenFile Error: Could not allocate memory for file.", ERR_LEVEL_ERROR);
		return FS_NULL;
	}
	
	//Initialize the file.
	pFile->m_BaseFile=File;
	pFile->m_bEOF=FS_FALSE;
	pFile->m_nAccess=Access;
	pFile->m_nBaseFileBegin=pMountInfo->nOffset;
	pFile->m_nFilePointer=0;
	//If creating a new file, the size needs to be set to 0.
	pFile->m_nSize=bNew?0:pMountInfo->nSize;
	pFile->m_pData=FS_NULL;
	wcsncpy(pFile->m_szPathW, pMountInfo->szMountFileW, FS_MAX_PATH);
	
	if(FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY) && bNew)
	{
		FS_ErrPrint(L"OpenFile Error: \"%s\" cannot be created, because an existing file is read only.",
			ERR_LEVEL_ERROR,
			pFile->m_szPathW);
		
		BDIO_Close(pFile->m_BaseFile);
		delete pFile;
		return FS_NULL;
	}
	
	//Check infor flags again.
	if(FS_CheckFlag(pFile->m_nAccess, LF_ACCESS_WRITE) 
		&& FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY))
	{
		FS_ErrPrint(L"OpenFile Error: \"%s\" is read only, cannot open for writing.", ERR_LEVEL_ERROR, pFile->m_szPathW);
		BDIO_Close(pFile->m_BaseFile);
		delete pFile;
		return FS_NULL;
	}
	
	FS_ErrPrint(L"Opening \"%s\"...", ERR_LEVEL_DETAIL, pFile->m_szPathW);
	//If the file is in an archive and is compressed it needs to be opened as a
	//memory file.
	if(FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED)
		&& FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
	{
		pFile->m_nAccess|=LF_ACCESS_MEMORY;
	}
	
	//If the file is a memory file then we just open it,
	//allocate memory for it, and copy the file data into
	//the buffer.
	if(FS_CheckFlag(pFile->m_nAccess, LF_ACCESS_MEMORY))
	{
		//Allocate memory...
		pFile->m_pData=static_cast<fs_byte*>(FS_Malloc(pFile->m_nSize, LF_ALLOC_REASON_FILE, "FS", __FILE__, __LINE__));
		if(!pFile->m_pData)
		{
			FS_ErrPrint(L"OpenFile Error: Could not allocte memory for \"%s\".", ERR_LEVEL_ERROR, pFile->m_szPathW);
			BDIO_Close(pFile->m_BaseFile);
			delete pFile;
			return FS_NULL;
		}
		//Read the file data...
		fs_dword nRead=0;
		//Seek to the beginning of the file...
		BDIO_Seek(pFile->m_BaseFile, pFile->m_nBaseFileBegin, LF_SEEK_BEGIN);
		//Then read either the compress data, or the uncompressed data.
		if(FS_CheckFlag(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
		{
			FS_ErrPrint(
				L"Reading and uncompressing \"%s\"...",
				ERR_LEVEL_DETAIL,
				pFile->m_szPathW);
			nRead=BDIO_ReadCompressed(pFile->m_BaseFile, pFile->m_nSize, pFile->m_pData);
		}
		else
		{
			FS_ErrPrint(
				L"Reading \"%s\"...",
				ERR_LEVEL_DETAIL,
				pFile->m_szPathW);
			nRead=BDIO_Read(pFile->m_BaseFile, pFile->m_nSize, pFile->m_pData);
		}
		
		if(nRead!=pFile->m_nSize)
		{
			FS_ErrPrint(
				L"OpenFile Warning: Only read %d out of %d bytes in \"%s\".", 
				ERR_LEVEL_WARNING, 
				nRead, 
				pFile->m_nSize, 
				pFile->m_szPathW);
				
			
			pFile->m_nSize=nRead;	
		}
		
		//We have the file open in memory so we no longer need to keep the
		//BDIO file open.
		BDIO_Close(pFile->m_BaseFile);
		pFile->m_BaseFile=FS_NULL;
	}
	else
	{
		//The file is not being opened as a memory file.
		//So we just leave it as it is and all file
		//reading/seeking functions will take care of everything.
	}
	pFile->m_bEOF=pFile->m_nFilePointer>=pFile->m_nSize;
	return pFile;
}

fs_bool CLFileSystem::CloseFile(CLFile* pFile)
{
	fs_dword nSize=0;
	
	if(!pFile)
		return FS_FALSE;
	FS_ErrPrint(L"Closing \"%s\"...", ERR_LEVEL_DETAIL, pFile->m_szPathW);
	if(pFile->m_BaseFile)
	{
		nSize=BDIO_GetSize(pFile->m_BaseFile);
		BDIO_Close(pFile->m_BaseFile);
	}	
	FS_SAFE_DELETE_ARRAY(pFile->m_pData, LF_ALLOC_REASON_FILE);
	//If the file was writeable we need to update the data in the
	//partition table.
	if(FS_CheckFlag(pFile->m_nAccess, LF_ACCESS_WRITE))
	{
		CPartTable* OwnerTable = FS_NULL;
		const MOUNT_FILE* pMountInfo=GetFileInfo(pFile->m_szPathW,&OwnerTable);
		if(!pMountInfo)
		{
			FS_ErrPrint(L"CloseFile Error: Could not update partition table.", ERR_LEVEL_ERROR);
		}
		else
		{
			MOUNT_FILE mountInfo=*pMountInfo;
			mountInfo.nSize=nSize;
			mountInfo.nCmpSize=nSize;
			OwnerTable->MountFile(&mountInfo, MOUNT_FILE_OVERWRITE);
		}
	}
	FS_SAFE_DELETE(pFile, LF_ALLOC_REASON_FILE);
	return FS_TRUE;
}

void CLFileSystem::EnumerateFilesOfType( fs_cstr Ext , FS_ENUMERATE_FUNC EnumerateFunc , void* Data )
{
	for( CPartTable* Table = m_PrtTableList; FS_NULL != Table; Table = Table->GetNext() )
	{
		Table->EnumerateFilesOfType( Ext , EnumerateFunc , Data );
	}
}

/********************************************************
*** The Partition Table Code
*** A partition table keeps track of mounted files
*** the table stores information about each file.
*** 
*** The partition table stores all the file information
*** in a hash table with HASH_SIZE entries.  If a filename
*** has a duplicate hash value then the table points
*** to a linked list with all the filenames.
********************************************************/
CLFileSystem::CPartTable::CPartTable( CPartTable* Next , fs_dword nMaxFiles )
: m_nMaxFiles(nMaxFiles)
, m_pMasterList(FS_NULL)
, m_Next( Next )
{		
	for(fs_dword i=0; i<HASH_SIZE; i++)
	{
		m_HashList[i]=FS_NULL;
	}

	//Create an initialize the master list
	m_pMasterList=static_cast<MOUNT_FILE_EX*>(FS_Malloc(sizeof(MOUNT_FILE_EX)*nMaxFiles, LF_ALLOC_REASON_SYSTEM, "FS", __FILE__, __LINE__));
	if(!m_pMasterList)
	{
		return;
	}
	m_UnusedFiles.Init(m_pMasterList, nMaxFiles, sizeof(MOUNT_FILE_EX));
}

CLFileSystem::CPartTable::~CPartTable()
{
	Clear();
	//Delete the hash table (note that the Clear method has already
	//deleted any linked lists that may exist, so the array can
	//safely be deleted without memory leaks.
	FS_SAFE_DELETE_ARRAY(m_pMasterList, LF_ALLOC_REASON_SYSTEM);
}

void CLFileSystem::CPartTable::Clear()
{
	m_UnusedFiles.Init(m_pMasterList, m_nMaxFiles, sizeof(MOUNT_FILE_EX));
	for(fs_dword i=0; i<HASH_SIZE; i++)
	{
		m_HashList[i]=FS_NULL;
	}
}

fs_bool CLFileSystem::CPartTable::IsFull()const
{
	return m_UnusedFiles.IsEmpty();
}

//FindFile finds a file in the partition table with the specified path.
//If not file exists it returns NULL, also note that
//this is CASE SENSITIVE so /base1/Tex.tga and /base1/tex.tga are not
//the same file.

CLFileSystem::MOUNT_FILE* CLFileSystem::CPartTable::FindFile(fs_cstr szFile)
{
	//To find a file just get the hash value for the specified filename
	//and check to see if a file is at that hash entry, not that
	//the linked list should be checked at that hash entry.
	fs_dword nHash=GetHashValue1024(szFile);
	//Loop through the linked list (usually 1-3 items long) and find
	//the filename.  
	for(MOUNT_FILE_EX* pItem=m_HashList[nHash]; pItem; pItem=pItem->pNext)
	{
		//Note that if a file is empty we don't need to worry
		//about it (we actually NEED to continue, becuase if a file
		//was mounted in that spot and was then unmounted all the
		//data is still there, only the Flags value has been changed.)
		if(pItem->Flags==MOUNT_FILE::MOUNT_FILE_EMPTY)
			continue;
			
		//If the string matches then return the file info.
		if(pItem->Flags!=MOUNT_FILE::MOUNT_FILE_ARCHIVED 
			&& (wcscmp(pItem->szMountFileW, szFile)==0))
			return pItem;
	}
	return FS_NULL;
}

fs_bool CLFileSystem::CPartTable::MountFile(CLFileSystem::MOUNT_FILE* pFile, fs_dword Flags)
{
	//Get the hash value for the mounted filename
	//note that we always use the mounted filename for the
	//hash table and not the OS filename.
	fs_dword nHash=GetHashValue1024(pFile->szMountFileW);

	//Figure out what the file extension is. (It'll be null terminated if there isn't one)
	fs_dword NameLen = wcslen( pFile->szMountFileW );
	pFile->ExtOffset = NameLen;
	for( fs_dword i=0;i<NameLen;i++)
	{
		if( pFile->szMountFileW[i] == '.' )
		{
			pFile->ExtOffset = i+1;
		}
	}
	
	//If the file at that hash entry is empty and there is no
	//linked list attached we can simply add the file to that spot.
	if( (m_HashList[nHash]==FS_NULL))
	{
		//Create a new files;
		MOUNT_FILE_EX* pNewFile=(MOUNT_FILE_EX*)m_UnusedFiles.Pop();
		if(!pNewFile)
			return FS_FALSE;
			
		pNewFile->pNext=FS_NULL;
		pNewFile->nHashValue=nHash;
		
		//Copy over the information for the file:
		memcpy(pNewFile, pFile, sizeof(MOUNT_FILE));
		////Copy just the MOUNT_FILE data, that way we don't overwrite the pNext value.
		//memcpy(m_pHashList[nHash], pFile, sizeof(MOUNT_FILE));
		m_HashList[nHash]=pNewFile;
		return FS_TRUE;
	}
	else
	{
		//The same filename may already exist.  So loop through the table and find
		//out if it does, if it does either skip the file or overwrite it based on
		//the flag, if the file doesn't exist add it on the end.
		for(MOUNT_FILE_EX* pItem=m_HashList[nHash]; pItem; pItem=pItem->pNext)
		{
			if(wcscmp(pItem->szMountFileW, pFile->szMountFileW)==0)
			{
				//The same file already exists, check flags and act appropriately.
				//If we need to overwrite an old file then we just copy the info
				if(FS_CheckFlag(Flags, MOUNT_FILE_OVERWRITELPKONLY))
				{
					if(FS_CheckFlag(pItem->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED))
					{
						FS_ErrPrint(L"Copying %s over old archive file.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
						memcpy(pItem, pFile, sizeof(MOUNT_FILE));
						return FS_TRUE;
					}
					else
					{
						FS_ErrPrint(L"Cannot mount %s.  A file with the same name has already been mounted.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
						return FS_FALSE;
					}	
				}
				else if(FS_CheckFlag(Flags, MOUNT_FILE_OVERWRITE))
				{
					FS_ErrPrint(L"Copying %s over old file.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
					memcpy(pItem, pFile, sizeof(MOUNT_FILE));
					return FS_TRUE;
				}
				else
				{
					FS_ErrPrint(L"Cannot mount %s.  A file with the same name has already been mounted.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
					return FS_FALSE;
				}
			}
			
			if(pItem->pNext==FS_NULL)
			{
				//We got to the end of the list and the file wasn't found,
				//so we proceed to add the new file onto the end of the list.
				MOUNT_FILE_EX* pNew=(MOUNT_FILE_EX*)m_UnusedFiles.Pop();
				if(!pNew)
					return FS_FALSE;
					
				memcpy(pNew, pFile, sizeof(MOUNT_FILE));
				pNew->nHashValue=nHash;
				pNew->pNext=FS_NULL;
				pItem->pNext=pNew;
				return FS_TRUE;
			}
			
			//^^^...Loop to the next item in the list...^^^
		}
		
	}
	//Shoul never actually get here.
	return FS_FALSE;
}


fs_dword CLFileSystem::CPartTable::GetHashValue1024(fs_cstr szString)
{
	//This is basically Bob Jenkins' One-At-A-Time-Hash.
	//http://www.burtleburtle.net/bob/hash/doobs.html
	fs_dword nLen=wcslen(szString);
	fs_dword nHash=0;
	fs_dword i=0;
	
	for(i=0; i<nLen; i++)
	{
		nHash+=szString[i];
		nHash+=(nHash<<10);
		nHash^=(nHash>>6);
	}
	nHash+=(nHash<<3);
	nHash^=(nHash>>11);
	nHash+=(nHash<<15);
	//We'll limit our hash value from 0 to HASH_SIZE-1.
	return nHash%HASH_SIZE;
}

void CLFileSystem::CPartTable::PrintMountInfo(FS_PRINT_MOUNT_INFO_TYPE Type)
{
	//Loop through all hash indexes and if the file exists
	//then print out the information about it.
	if( PRINT_MOUNT_INFO_FULL == Type )
	{
		FS_ErrPrint(L"FLAGS  SIZE       PACKED    PATH", ERR_LEVEL_ALWAYS);
		FS_ErrPrint(L"-----  ----       ------    ----", ERR_LEVEL_ALWAYS);
	}
	fs_dword nFiles=0;
	for(fs_dword i=0; i<HASH_SIZE; i++)
	{
		for(MOUNT_FILE_EX* pItem=m_HashList[i]; pItem; pItem=pItem->pNext)
		{
			nFiles++;

			if( PRINT_MOUNT_INFO_FILENAME ==Type )
			{
					fs_path szOutput;
					_snwprintf(szOutput, FS_MAX_PATH, L"%s", pItem->szMountFileW);
					szOutput[FS_MAX_PATH]=0;
					FS_ErrPrint(szOutput, ERR_LEVEL_ALWAYS);
			}
			else
			{
				PrintFileInfo(pItem);
			}
		}
	}

	if( PRINT_MOUNT_INFO_FULL == Type )
	{
		FS_ErrPrint(L"Totals: %d files", ERR_LEVEL_ALWAYS, nFiles);
	}
}

void CLFileSystem::CPartTable::PrintFileInfo(MOUNT_FILE_EX* pFile)
{
	//If there was not file in that table position we'll simply return.
	if(pFile->Flags==MOUNT_FILE::MOUNT_FILE_EMPTY)
		return;
		
	fs_char szOutput[29+FS_MAX_PATH];
	szOutput[5]=' ';
	
	//First thing print all the file flags.
	if(FS_CheckFlag(pFile->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		szOutput[0]=('d');
	else
		szOutput[0]=('-');
	if(FS_CheckFlag(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED))
		szOutput[1]=('a');
	else
		szOutput[1]=('-');
				
	if(FS_CheckFlag(pFile->Flags, MOUNT_FILE::MOUNT_FILE_READONLY))
	{
		szOutput[2]=('r');
		szOutput[3]=('-');
	}
	else
	{
		szOutput[2]=('r');
		szOutput[3]=('w');
	}
			
	if(FS_CheckFlag(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
		szOutput[4]=('z');
	else
		szOutput[4]=('-');
	
	_snwprintf(&szOutput[6], 10, L"%10u", pFile->nSize);
	szOutput[16]=' ';
	_snwprintf(&szOutput[17], 10, L"%10u", pFile->nCmpSize);
	szOutput[27]=' ';
	_snwprintf(&szOutput[28], FS_MAX_PATH, L"%s", pFile->szMountFileW);
	szOutput[28+FS_MAX_PATH]=0;
	FS_ErrPrint(szOutput, ERR_LEVEL_ALWAYS);
	
	_snwprintf(szOutput, FS_MAX_PATH+27, L"%4u: %10u           (%s)", pFile->nHashValue, pFile->nOffset, pFile->szOSFileW);
	FS_ErrPrint(szOutput, ERR_LEVEL_DETAIL);
}

void CLFileSystem::CPartTable::EnumerateFilesOfType( fs_cstr Ext , FS_ENUMERATE_FUNC EnumerateFunc , void* Data )
{
	for(fs_dword HashPos=0; HashPos<HASH_SIZE; HashPos++)
	{
		for(MOUNT_FILE_EX* pItem=m_HashList[HashPos]; pItem; pItem=pItem->pNext)
		{	
			fs_cstr FileExt = &pItem->szMountFileW[pItem->ExtOffset];
			if( 0 == _wcsicmp( FileExt , Ext ) )
			{
				EnumerateFunc( pItem->szMountFileW , Data );
			}
		}
	}
}
