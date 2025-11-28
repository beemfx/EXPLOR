#include <string.h>
#include <ctype.h>
#include "fs_sys2.h"

/***************************************************
	Pathname getting functions, retrieve filename
	path, or short filename from a full path
***************************************************/

fs_str FS_GetFileNameFromPath(fs_str szFileName, fs_cstr szFullPath)
{
	fs_dword dwLen=wcslen(szFullPath);
	fs_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szFileName[j]=szFullPath[i];
	}
	return szFileName;
}

fs_str FS_GetDirFromPath(fs_str szDir, fs_cstr szFullPath)
{
	fs_dword dwLen=wcslen(szFullPath);
	fs_long i=0;
	for(i=(fs_long)dwLen-1; i>=0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
			break;
	}
	wcsncpy(szDir, szFullPath, i+1);
	szDir[i+1]=0;
	return szDir;
}

void FS_SYS2_EXPORTS FS_FixCase(fs_path szOut, fs_cstr szFullPath)
{
	fs_long i = 0;
	for(i=0; i<FS_MAX_PATH; i++)
	{
		szOut[i] = tolower(szFullPath[i]);
		if(0 == szOut[i])break;
	}
	szOut[FS_MAX_PATH] = 0;
}