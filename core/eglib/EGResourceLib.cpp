// (c) 2017 Beem Media

#include "EGResourceLib.h"
#include "EGWindowsAPI.h"

static HMODULE EGEditor_ToolResLib = nullptr;

void EGResourceLib_Init()
{
	EGEditor_ToolResLib = LoadLibraryW( L"EGEdResLib.dll" );
	if( EGEditor_ToolResLib )
	{

	}
	else
	{
		EGLogf( eg_log_t::Error , "Unable to load EGEdResLib.dll some tools may not work correctly." );
		MessageBoxW( nullptr , L"Unable to load EGEdResLib.dll some tools may not work correctly." , L"EG Editor" , MB_OK );
	}
}

void EGResourceLib_Deinit()
{
	if( EGEditor_ToolResLib )
	{
		FreeLibrary( EGEditor_ToolResLib );
		EGEditor_ToolResLib = nullptr;
	}

}

HMODULE EGResourceLib_GetLibrary()
{
	if( nullptr == EGEditor_ToolResLib )
	{
		return GetModuleHandleW( nullptr );
	}

	return EGEditor_ToolResLib;
}
