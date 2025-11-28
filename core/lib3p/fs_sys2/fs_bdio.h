#ifndef _FS_BDIO_H__
#define _FS_BDIO_H__
#include "fs_sys2.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/* Basic DIO stuff, should only be used internally only. */
static fs_bool FS_CheckFlag(fs_dword flag, fs_dword var){ return ((flag&var) != 0); }

static const fs_dword LF_ACCESS_BDIO_TEMP = (1 << 5);

typedef fs_intptr_t BDIO_FILE;
BDIO_FILE BDIO_Open(fs_cstr szFilename, LF_CREATE_MODE nMode, fs_dword nAccess);

fs_bool   BDIO_Close(BDIO_FILE File);
fs_dword  BDIO_Read(BDIO_FILE File, fs_dword nSize, void* pOutBuffer);
fs_dword  BDIO_Write(BDIO_FILE File, fs_dword nSize, const void* pInBuffer);
fs_dword  BDIO_Tell(BDIO_FILE File);
fs_dword  BDIO_Seek(BDIO_FILE File, fs_long nOffset, LF_SEEK_TYPE nOrigin);
fs_dword  BDIO_GetSize(BDIO_FILE File);
			 
fs_dword  BDIO_WriteCompressed(BDIO_FILE File, fs_dword nSize, void* pInBuffer);
fs_dword  BDIO_ReadCompressed(BDIO_FILE File, fs_dword nSize, void* pOutBuffer);
			 
fs_bool   BDIO_ChangeDir(fs_cstr szDir);
fs_dword  BDIO_GetCurrentDir(fs_str szDir, fs_dword nSize);

BDIO_FILE BDIO_OpenTempFile(LF_CREATE_MODE nMode, fs_dword nAccess);

fs_dword  BDIO_CopyData(BDIO_FILE DestFile, BDIO_FILE SourceFile, fs_dword nSize);

fs_dword  BDIO_GetFullPathName(fs_path szPath, const fs_path szFilename);

typedef void* BDIO_FIND;

typedef struct _BDIO_FIND_DATAW{
	fs_bool bDirectory;
	fs_bool bReadOnly;
	fs_bool bHidden;
	fs_bool bNormal;
	fs_large_integer nFileSize;
	fs_path szFilename;
}BDIO_FIND_DATAW;

BDIO_FIND BDIO_FindFirstFile(fs_cstr szFilename, BDIO_FIND_DATAW* pFindData);
fs_bool   BDIO_FindNextFile(BDIO_FIND hFindFile, BDIO_FIND_DATAW* pFindData);
fs_bool   BDIO_FindClose(BDIO_FIND hFindFile);
fs_char BDIO_GetOsPathSeparator();

#ifdef __cplusplus
}
#endif __cplusplus

#endif _FS_BDIO_H__