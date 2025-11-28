#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "fs_sys2.h"

FS_ERR_LEVEL g_nErrLevel=ERR_LEVEL_NOTICE;
void (__cdecl*g_pfnErrPrint)(fs_cstr szFormat)=FS_NULL;

void FS_SetErrLevel(FS_ERR_LEVEL nLevel)
{
	g_nErrLevel=nLevel;
}

void FS_SYS2_EXPORTS FS_SetErrPrintFn(void (__cdecl*pfn)(fs_cstr szFormat))
{
	g_pfnErrPrint=pfn;
}

#define FS_ERR_MAX_OUTPUT 511

void FS_ErrPrint(fs_cstr szFormat, FS_ERR_LEVEL nErrLevel, ...)
{
	
	fs_char szOutput[FS_ERR_MAX_OUTPUT+1];
	va_list arglist=FS_NULL;
	//Don't print the message if the current error level
	//is greater than the error level of the message.
	if((nErrLevel<g_nErrLevel) || !g_pfnErrPrint)
		return;
		
	va_start(arglist, nErrLevel);
	//_vsnwprintf(szOutput, 1022, szFormat, arglist);
	_vsnwprintf_s(szOutput, FS_ERR_MAX_OUTPUT, FS_ERR_MAX_OUTPUT, szFormat, arglist);
	va_end(arglist);
	
	szOutput[wcslen(szOutput)]=0;
	g_pfnErrPrint(szOutput);	
}
