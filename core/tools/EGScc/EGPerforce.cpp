// (c) 2018 Beem Media

#include "EGPerforce.h"
#include "EGWindowsAPI.h"
#include "EGToolsHelper.h"

// EX-Release: Originally this was code to connect to Perforce source control
// for source control operations. If there is a need to hook this up to source
// control again the implementation might go here.

eg_bool EGPerforce_CheckoutFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn )
{
	unused(Filename, LogFn);

	return false;
}

eg_bool EGPerforce_RevertFile( eg_cpstr16 Filename, eg_bool bOnlyIfUnchanged, EGPerforceLogFn& LogFn )
{
	unused(Filename, bOnlyIfUnchanged, LogFn);
	return false;
}

eg_bool EGPerforce_AddFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn )
{
	unused(Filename, LogFn);
	return false;
}

eg_bool EGPerforce_DeleteFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn )
{
	unused(Filename, LogFn);
	return false;
}

#include "EGDynamicString.cpp"
#include "EGToolsHelper.cpp"
#include "EGstring.cpp"
#include "EGStringEx.cpp"
#include "EGPath2.cpp"
#include "EGLog.cpp"
#include "EGLogDispatcher.cpp"
#include "EGMutex.cpp"
#include "EGBase64.cpp"
#include "EGWeakPtrBase.cpp"
#include "EGObject.cpp"
#include "EGLibFile.cpp"
#include "EGMemPool.cpp"
#include "EGFileData.cpp"
#include "EGStringFormat.cpp"
#include "EGMath.cpp"
#include "EGMatrix.cpp"
#include "EGPlane.cpp"
#include "EGOsFile.cpp"

#include <malloc.h>

void* operator new ( eg_size_t Size , eg_mem_pool MemPool )
{
	unused( MemPool );
	return malloc( Size );
}

void* operator new [] ( eg_size_t Size , eg_mem_pool MemPool )
{
	unused( MemPool );
	return malloc( Size );
}

void operator delete ( void* p , eg_mem_pool MemPool )
{
	unused( MemPool );

	free( p );
}

void operator delete [] ( void* p , eg_mem_pool MemPool )
{
	unused( MemPool );

	free( p );
}
