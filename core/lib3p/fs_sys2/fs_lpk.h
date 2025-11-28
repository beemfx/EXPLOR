#ifndef __FS_ARC_H__
#define __FS_ARC_H__

#include "fs_sys2.h"

typedef fs_intptr_t BDIO_FILE;

static const fs_dword LPK_VERSION = 103;
static const fs_dword LPK_TYPE    = 0x314B504C; //(*(fs_dword*)"LPK1")

static const fs_dword LPK_FILE_TYPE_NORMAL   = (1<<0);
static const fs_dword  LPK_FILE_TYPE_ZLIBCMP = (1<<1);

static const fs_dword LPK_OPEN_ENABLEWRITE = (1<<0);
static const fs_dword LPK_ADD_ZLIBCMP      = (1<<1);

typedef struct _LPK_FILE_INFO
{
	fs_path szFilename;
	fs_dword nType;
	fs_dword nOffset;
	fs_dword nSize;
	fs_dword nCmpSize;
}LPK_FILE_INFO;

static const fs_dword LPK_FILE_POS_TEMP = (1<<3);
static const fs_dword LPK_FILE_POS_MAIN = (1<<4);

typedef struct _LPK_FILE_INFO_EX: public LPK_FILE_INFO
{
	fs_dword nInternalPosition;
}LPK_FILE_INFO_EX;

class FS_SYS2_EXPORTS CLArchive
{
public:
	CLArchive();
	~CLArchive();
	
	fs_bool CreateNew(fs_cstr szFilename);
	fs_bool Open(fs_cstr szFilename, fs_dword Flags=0);
	fs_bool Open( const void* pData , fs_dword DataSize , fs_dword Flags = 0 );
	void Close();
	fs_bool Save();
	
	fs_dword AddFile(fs_cstr szFilename, fs_cstr szNameInArc, fs_dword Flags);
	fs_dword GetNumFiles();
	fs_bool GetFileInfo(fs_dword nRef, LPK_FILE_INFO* pInfo);
	const LPK_FILE_INFO* GetFileInfo(fs_dword nRef);
	fs_bool IsOpen();
	fs_dword GetFileRef(const fs_path szName);
	fs_bool ExtractFile( fs_dword nRef , void* pOut , fs_dword OutSize );

private:
	fs_dword m_nType;
	fs_dword m_nVersion;
	fs_dword m_nCount;
	fs_dword m_nInfoOffset;
	LPK_FILE_INFO_EX* m_pFileList;
	
	BDIO_FILE m_File;
	BDIO_FILE m_TempFile;
	
	fs_bool m_bOpen;
	fs_bool m_bWriteable;
	fs_dword m_nMainFileWritePos;
	fs_bool m_bHasChanged;
	fs_dword m_nCmpThreshold;
	fs_bool ReadArcInfo();
	
	fs_dword DoAddFile(BDIO_FILE fin, fs_cstr szNameInArc, fs_dword dwFlags);
};

#endif __FS_ARC_H__