#include "fs_sys2.h"

#if defined( __cplusplus )
extern "C" {
#endif

void* FS_Malloc(fs_size_t size, LF_ALLOC_REASON reason, const fs_char8*const type, const fs_char8*const file, const fs_uint line);
void  FS_Free(void* p, LF_ALLOC_REASON reason);

/* Path name parsing functions. */
fs_str FS_GetDirFromPath(fs_str szDir, fs_cstr szFullPath);
void FS_FixCase(fs_path szOut, fs_cstr szFullPath);
void FS_ErrPrint(fs_cstr szFormat, FS_ERR_LEVEL nErrLevel, ...);

#if defined( __cplusplus )
} //extern "C"
#endif

#if defined( __cplusplus )
class IFsMemObj
{
public:
	void* operator new(fs_size_t Size);
	void operator delete(void*);
};
#endif
