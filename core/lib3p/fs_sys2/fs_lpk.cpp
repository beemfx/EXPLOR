#include <string.h>
#include <stdlib.h>
#include "fs_lpk.h"
#include "fs_bdio.h"
#include "stdio.h"
#include "fs_internal.h"

CLArchive::CLArchive():
	m_bOpen(FS_FALSE),
	m_pFileList(FS_NULL),
	m_nType(0),
	m_nVersion(0),
	m_nCount(0),
	m_nInfoOffset(0),
	m_File(FS_NULL),
	m_TempFile(FS_NULL),
	m_nMainFileWritePos(0),
	m_bHasChanged(FS_FALSE),
	m_nCmpThreshold(20),
	m_bWriteable(FS_FALSE)
{

}

CLArchive::~CLArchive()
{
	Close();
}

fs_dword CLArchive::GetNumFiles()
{
	return m_nCount;
}

fs_bool CLArchive::IsOpen()
{
	return m_bOpen;
}

const LPK_FILE_INFO* CLArchive::GetFileInfo(fs_dword n)
{
	if(n>=m_nCount)
		return FS_NULL;
		
	return &m_pFileList[n];
}

fs_bool CLArchive::GetFileInfo(fs_dword nRef, LPK_FILE_INFO* pInfo)
{
	if(nRef>=m_nCount)
		return FS_FALSE;
		
	*pInfo=m_pFileList[nRef];
	return FS_TRUE;
}

fs_bool CLArchive::CreateNew(fs_cstr szFilename)
{
	Close();
	
	m_File=BDIO_Open(szFilename, LF_CREATE_ALWAYS, LF_ACCESS_READ|LF_ACCESS_WRITE);
	if(!m_File)
		return FS_FALSE;
		
	m_bWriteable=FS_TRUE;
	m_TempFile=BDIO_OpenTempFile(LF_CREATE_ALWAYS, LF_ACCESS_READ|LF_ACCESS_WRITE|LF_ACCESS_BDIO_TEMP);
	if(!m_TempFile)
	{
		BDIO_Close(m_File);
		m_File=NULL;
		m_TempFile=NULL;
		return FS_FALSE;
	}
	
	m_nType=LPK_TYPE;
	m_nVersion=LPK_VERSION;
	m_nCount=0;
	m_nInfoOffset=0;
	m_pFileList=FS_NULL;
	m_bOpen=FS_TRUE;
	m_bHasChanged=FS_TRUE;
	return m_bOpen;
}

fs_bool CLArchive::Open(fs_cstr szFilename, fs_dword Flags)
{
	Close();
	
	fs_dword dwAccess=LF_ACCESS_READ;
	
	if(FS_CheckFlag(Flags, LPK_OPEN_ENABLEWRITE))
	{
		m_bWriteable=FS_TRUE;
		dwAccess|=LF_ACCESS_WRITE;
	}
	
	m_File=BDIO_Open(szFilename, LF_OPEN_ALWAYS, dwAccess);
	if(!m_File)
	{
		Close();
		return FS_FALSE;
	}
	
	//If the archive is going to be writable then we open a temp file.
	//if a temp file can't be openened then the archive is set to read
	//only.
	if(m_bWriteable)
	{
		m_TempFile=BDIO_OpenTempFile(LF_CREATE_ALWAYS, LF_ACCESS_READ|LF_ACCESS_WRITE|LF_ACCESS_BDIO_TEMP);
		if(!m_TempFile)
			m_bWriteable=FS_FALSE;
	}	
	else
		m_TempFile=FS_NULL;

		
	if(!ReadArcInfo())
	{
		Close();
		return FS_FALSE;
	}	
	m_bHasChanged=FS_FALSE;
	m_bOpen=FS_TRUE;
	return FS_TRUE;
}

fs_bool CLArchive::Open( const void* pData , fs_dword DataSize , fs_dword Flags /* = 0 */ )
{
	Close();

	fs_dword dwAccess=LF_ACCESS_READ;

	if(FS_CheckFlag(Flags, LPK_OPEN_ENABLEWRITE))
	{
		m_bWriteable=FS_TRUE;
		dwAccess|=LF_ACCESS_WRITE;
	}

	m_File=BDIO_OpenTempFile(LF_CREATE_ALWAYS, dwAccess|LF_ACCESS_WRITE);
	if(!m_File)
	{
		Close();
		return FS_FALSE;
	}

	fs_bool bWritten = BDIO_Write( m_File , DataSize , pData ) == DataSize;
	if( !bWritten )
	{
		Close();
		return FS_FALSE;
	}
	BDIO_Seek( m_File , 0 , LF_SEEK_TYPE::LF_SEEK_BEGIN );

	//If the archive is going to be writable then we open a temp file.
	//if a temp file can't be openened then the archive is set to read
	//only.
	if(m_bWriteable)
	{
		m_TempFile=BDIO_OpenTempFile(LF_CREATE_ALWAYS, LF_ACCESS_READ|LF_ACCESS_WRITE|LF_ACCESS_BDIO_TEMP);
		if(!m_TempFile)
			m_bWriteable=FS_FALSE;
	}	
	else
		m_TempFile=FS_NULL;


	if(!ReadArcInfo())
	{
		Close();
		return FS_FALSE;
	}	
	m_bHasChanged=FS_FALSE;
	m_bOpen=FS_TRUE;
	return FS_TRUE;
}

fs_dword CLArchive::DoAddFile(BDIO_FILE fin, fs_cstr szNameInArc, fs_dword Flags)
{	
	LPK_FILE_INFO_EX Entry;
	Entry.nSize=BDIO_GetSize(fin);
	Entry.nCmpSize=Entry.nSize;
	//Entry.nOffset=BDIO_Tell(m_File);
	Entry.nType=FS_CheckFlag(Flags, LPK_ADD_ZLIBCMP)?LPK_FILE_TYPE_ZLIBCMP:LPK_FILE_TYPE_NORMAL;
	Entry.nInternalPosition=LPK_FILE_POS_MAIN;
	
	//Just in case a file with the same name already exists, go ahead
	//and add something onto the end of it (we'll add an X).
	wcsncpy(Entry.szFilename, szNameInArc, FS_MAX_PATH);
	while(GetFileRef(Entry.szFilename)!=0xFFFFFFFF)
	{
		wcscat(Entry.szFilename, L"X");
	}
	
	fs_byte* pTemp=static_cast<fs_byte*>(FS_Malloc(sizeof(fs_byte)*Entry.nSize, LF_ALLOC_REASON_SCRATCH, "FS", __FILE__, __LINE__));
	if(!pTemp)
	{
		return 0;
	}
	if(BDIO_Read(fin, Entry.nSize, pTemp)!=Entry.nSize)
	{
		FS_Free(pTemp, LF_ALLOC_REASON_SCRATCH );
		return 0;
	}
	
	//Write the file to the end of the main file.
	BDIO_Seek(m_File, m_nMainFileWritePos, LF_SEEK_BEGIN);
	Entry.nOffset=BDIO_Tell(m_File);
	if(Entry.nType==LPK_FILE_TYPE_ZLIBCMP)
	{
		Entry.nCmpSize=BDIO_WriteCompressed(m_File, Entry.nSize, pTemp);
		fs_dword nCmp=(fs_dword)(100.0f-(float)Entry.nCmpSize/(float)Entry.nSize*100.0f);
		//If the compression did not occur, or if the compression
		//percentage was too low, we'll just write the file data.
		//By default the compression threshold is 20.
		if( (Entry.nCmpSize==0) || (nCmp<m_nCmpThreshold) )
		{
			BDIO_Seek(m_File, Entry.nOffset, LF_SEEK_BEGIN);
			Entry.nCmpSize=BDIO_Write(m_File, Entry.nSize, pTemp);
			Entry.nType=LPK_FILE_TYPE_NORMAL;
		}
	}
	else
	{
		Entry.nCmpSize=BDIO_Write(m_File, Entry.nSize, pTemp);
	}
	m_nMainFileWritePos=BDIO_Tell(m_File);
	
	FS_Free(pTemp,LF_ALLOC_REASON_SCRATCH);
	pTemp = FS_NULL;

	LPK_FILE_INFO_EX* pNew=static_cast<LPK_FILE_INFO_EX*>(FS_Malloc(sizeof(LPK_FILE_INFO_EX)*(m_nCount+1), LF_ALLOC_REASON_FILE, "FS", __FILE__, __LINE__));
	for(fs_dword i=0; i<m_nCount; i++)
	{
		pNew[i]=m_pFileList[i];
	}
	pNew[m_nCount]=Entry;
	m_nCount++;
	if(m_pFileList){FS_Free(m_pFileList,LF_ALLOC_REASON_FILE);}
	m_pFileList=pNew;
	
	m_bHasChanged=FS_TRUE;
	return Entry.nCmpSize;
	
}

fs_dword CLArchive::AddFile(fs_cstr szFilename, fs_cstr szNameInArc, fs_dword Flags)
{
	if(!m_bOpen || !m_bWriteable)
		return 0;
		
	BDIO_FILE fin=BDIO_Open(szFilename, LF_OPEN_EXISTING, LF_ACCESS_READ);
	if(!fin)
		return 0;
	
	fs_dword nSize=DoAddFile(fin, szNameInArc, Flags);
	BDIO_Close(fin);
	return nSize;	
}

fs_bool CLArchive::ReadArcInfo()
{
	//Read the header info.  It should be the last 16 bytes of
	//the file.
	BDIO_Seek(m_File, -16, LF_SEEK_END);
	BDIO_Read(m_File, 4, &m_nType);
	BDIO_Read(m_File, 4, &m_nVersion);
	BDIO_Read(m_File, 4, &m_nCount);
	BDIO_Read(m_File, 4, &m_nInfoOffset);
	
	if((m_nType!=LPK_TYPE) || (m_nVersion!=LPK_VERSION))
		return FS_FALSE;
	
	//Seek to the beginning of the file data.
	BDIO_Seek(m_File, m_nInfoOffset, LF_SEEK_BEGIN);
	if(BDIO_Tell(m_File)!=m_nInfoOffset)
		return FS_FALSE;
		
	//Save the info offset as the main file write position,
	//as this will be where new files are written to.
	m_nMainFileWritePos=m_nInfoOffset;
	
	//Allocate memory for file data.
	m_pFileList=static_cast<LPK_FILE_INFO_EX*>(FS_Malloc(sizeof(LPK_FILE_INFO_EX)*m_nCount, LF_ALLOC_REASON_FILE, "FS", __FILE__, __LINE__));
	if(!m_pFileList)
		return FS_FALSE;
		
	//Now read the data.
	for(fs_dword i=0; i<m_nCount; i++)
	{
		BDIO_Read(m_File, 256, &m_pFileList[i].szFilename);
		BDIO_Read(m_File, 4, &m_pFileList[i].nType);
		BDIO_Read(m_File, 4, &m_pFileList[i].nOffset);
		BDIO_Read(m_File, 4, &m_pFileList[i].nSize);
		BDIO_Read(m_File, 4, &m_pFileList[i].nCmpSize);
		m_pFileList[i].nInternalPosition=LPK_FILE_POS_MAIN;
	}	
	return FS_TRUE;
}


fs_bool CLArchive::Save()
{
	if(!m_bWriteable || !m_bOpen || !m_bHasChanged)
		return FS_FALSE;
		
	BDIO_Seek(m_File, m_nMainFileWritePos, LF_SEEK_BEGIN);
	BDIO_Seek(m_TempFile, 0, LF_SEEK_BEGIN);
	
	for(fs_dword i=0; i<m_nCount; i++)
	{
		if(m_pFileList[i].nInternalPosition==LPK_FILE_POS_TEMP)
		{
			BDIO_Seek(m_TempFile, m_pFileList[i].nOffset, LF_SEEK_BEGIN);
			m_pFileList[i].nOffset=BDIO_Tell(m_File);
			BDIO_CopyData(m_File, m_TempFile, m_pFileList[i].nCmpSize);
			
			m_pFileList[i].nInternalPosition=LPK_FILE_POS_MAIN;
		}
		else
		{
			//We don't need to do anything.
		}
	}
	m_nInfoOffset=BDIO_Tell(m_File);
	//Write the file info data.
	for(fs_dword i=0; i<m_nCount; i++)
	{
		BDIO_Write(m_File, 256, &m_pFileList[i].szFilename);
		BDIO_Write(m_File, 4, &m_pFileList[i].nType);
		BDIO_Write(m_File, 4, &m_pFileList[i].nOffset);
		BDIO_Write(m_File, 4, &m_pFileList[i].nSize);
		BDIO_Write(m_File, 4, &m_pFileList[i].nCmpSize);
	}
	
	BDIO_Seek(m_File, 0, LF_SEEK_END);
	//Write the header...
	BDIO_Write(m_File, 4, &m_nType);
	BDIO_Write(m_File, 4, &m_nVersion);
	BDIO_Write(m_File, 4, &m_nCount);
	BDIO_Write(m_File, 4, &m_nInfoOffset);
	
	BDIO_Seek(m_TempFile, 0, LF_SEEK_BEGIN);
	m_bHasChanged=FS_FALSE;
	return FS_TRUE;
}

void CLArchive::Close()
{		
	//If the file was writeable then everything will be written from the temp file.
	if(m_bWriteable && m_bOpen)
	{
		Save();
	}
	
	if(m_File)
		BDIO_Close(m_File);
		
	if(m_TempFile)
		BDIO_Close(m_TempFile);
		
	if(m_pFileList)
		FS_Free(m_pFileList, LF_ALLOC_REASON_FILE);
		
	m_File=m_TempFile=0;
	m_pFileList=FS_NULL;
		
	m_bOpen=FS_FALSE;
	m_nType=0;
	m_nVersion=0;
	m_nCount=0;
	m_nInfoOffset=0;
	m_bHasChanged=FS_FALSE;
}

fs_dword CLArchive::GetFileRef(const fs_path szName)
{
	for(fs_dword i=0; i<this->m_nCount; i++)
	{
		if(wcsicmp(szName, m_pFileList[i].szFilename)==0)
			return i;
	}
	return 0xFFFFFFFF;
}
fs_bool CLArchive::ExtractFile( fs_dword nRef , void* pOut , fs_dword OutSize )
{	
	if(nRef>=m_nCount)
		return FS_FALSE;
		
	BDIO_FILE fin=FS_NULL;
	if(m_pFileList[nRef].nInternalPosition==LPK_FILE_POS_TEMP)
		fin=m_TempFile;
	else
		fin=m_File;
		
	BDIO_Seek(fin, m_pFileList[nRef].nOffset, LF_SEEK_BEGIN);

	if( OutSize == m_pFileList[nRef].nSize )
	{
		fs_dword nRead=0;
		if(m_pFileList[nRef].nType==LPK_FILE_TYPE_ZLIBCMP)
		{
			nRead=BDIO_ReadCompressed(fin, m_pFileList[nRef].nSize, pOut);
		}
		else
		{
			nRead=BDIO_Read(fin, m_pFileList[nRef].nSize, pOut);
		}
	
		return nRead == m_pFileList[nRef].nSize && nRead == OutSize ? FS_TRUE : FS_FALSE;
	}

	return FS_FALSE;
}
