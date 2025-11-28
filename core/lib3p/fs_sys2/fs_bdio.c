/* fs_bdio.c - Basic Disk Input Output. These functions should be used
	internally only, and not by outside systems.
	
	Copyright (C) 2007 Blaine Myers
*/

#if defined( __WIN32__ ) || defined( __WIN64__ )

#include "fs_bdio.win32.h"

#else

#error Platform not set.

#endif