/* Memory allocation. */
#include "fs_sys2.h"
#include "fs_internal.h"

static FS_ALLOC_FUNC g_pMalloc=0;
static FS_FREE_FUNC  g_pFree=0;

extern "C" void FS_SetMemFuncs(FS_ALLOC_FUNC pAlloc, FS_FREE_FUNC pFree)
{
	g_pMalloc=pAlloc;
	g_pFree=pFree;
}

extern "C" void* FS_Malloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line)
{
	if(g_pMalloc)
	{
		return g_pMalloc(size, reason, type, file, line);
	}
	else
	{
		__debugbreak();
		return FS_NULL;
	}
}

extern "C" void FS_Free(void* p, LF_ALLOC_REASON reason)
{
	if(g_pFree)
	{
		return g_pFree(p, reason);
	}
	else
	{
		__debugbreak();
	}
}

void* IFsMemObj::operator new(fs_size_t Size)
{
	return FS_Malloc(Size, LF_ALLOC_REASON_SYSTEM, "FS", __FILE__, __LINE__);
}

void IFsMemObj::operator delete(void* pData)
{
	FS_Free(pData, LF_ALLOC_REASON_SYSTEM);
}
