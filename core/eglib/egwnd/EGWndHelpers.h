// (c) 2017 Beem Media

#pragma once

#include "EGWindowsAPI.h"

static inline eg_bool EGWndHelper_GetFile( HWND hwndOwner , eg_char8* Out , eg_size_t OutSize , eg_bool bIsSave = false , eg_cpstr8 Filter = "All\0*.*\0Text\0*.TXT\0" , eg_cpstr8 DefExt = nullptr )
{
	assert( OutSize >= 256 ); // Probably need a bigger out size.
	for( eg_size_t i=0; i<OutSize;i++ ) { if( Out[i] == '\0' ) break; if( Out[i] == '/' ) { Out[i] = '\\'; } }
	OPENFILENAMEA  of;
	zero( &of );

	of.lStructSize = sizeof(of);
	of.hwndOwner = hwndOwner;
	of.lpstrFile = Out;
	of.nMaxFile = static_cast<DWORD>(OutSize);
	of.lpstrFilter = Filter;
	of.nFilterIndex = 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrDefExt = DefExt;
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if( bIsSave )
	{
		of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_CREATEPROMPT;
	}

	return TRUE == (bIsSave ? GetSaveFileNameA( &of) : GetOpenFileNameA( &of ));
}

static inline eg_bool EGWndHelper_GetFile( HWND hwndOwner , eg_char16* Out , eg_size_t OutSize , eg_bool bIsSave = false , eg_cpstr16 Filter = L"All\0*.*\0Text\0*.TXT\0" , eg_cpstr16 DefExt = nullptr )
{
	assert( OutSize >= 256 ); // Probably need a bigger out size.
	for( eg_size_t i=0; i<OutSize;i++ ) { if( Out[i] == '\0' ) break; if( Out[i] == '/' ) { Out[i] = '\\'; } }
	OPENFILENAMEW  of;
	zero( &of );

	of.lStructSize = sizeof(of);
	of.hwndOwner = hwndOwner;
	of.lpstrFile = Out;
	of.nMaxFile = static_cast<DWORD>(OutSize);
	of.lpstrFilter = Filter;
	of.nFilterIndex = 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.lpstrDefExt = DefExt;
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if( bIsSave )
	{
		of.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_CREATEPROMPT;
	}

	return TRUE == (bIsSave ? GetSaveFileNameW( &of) : GetOpenFileNameW( &of ));
}

static inline eg_bool EGWndHelper_GetFolder( HWND hwndOwner , eg_char8* Out , eg_size_t OutSize )
{
	for( eg_size_t i=0; i<OutSize;i++ ) { if( Out[i] == '\0' ) break; if( Out[i] == '/' ) { Out[i] = '\\'; } }
	BROWSEINFOA bi;
	zero( &bi );

	bi.hwndOwner = hwndOwner;
	bi.lpszTitle  = "Select a folder...";
	bi.ulFlags    = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_USENEWUI;

	LPITEMIDLIST pidl = SHBrowseForFolderA( &bi );

	if( 0 != pidl )
	{
		//get the name of the folder and put it in path
		eg_char16 Path[4096];
		SHGetPathFromIDListEx( pidl , Path , countof(Path) , 0 );
		eg_string_big NewOutput( Path );
		NewOutput.CopyTo( Out , OutSize );

		//free memory used
		IMalloc* imalloc = nullptr;
		if (SUCCEEDED( SHGetMalloc ( &imalloc )) )
		{
			imalloc->Free( pidl );
			imalloc->Release( );
		}

		return true;
	}

	return false;
}

static inline eg_bool WGWndHelper_ChooseColor( HWND Owner , eg_color& OutColor , COLORREF ChooseColors[16] )
{
	CHOOSECOLORW ColorInfo;
	zero( &ColorInfo );
	ColorInfo.lStructSize = sizeof(ColorInfo);
	ColorInfo.hwndOwner = Owner;
	eg_real Alpha = OutColor.a;
	eg_color32 AsColor32 = eg_color32(OutColor);
	ColorInfo.rgbResult = RGB( AsColor32.R , AsColor32.G , AsColor32.B );
	ColorInfo.lpCustColors = ChooseColors;
	ColorInfo.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;
	if( TRUE == ChooseColorW( &ColorInfo ) )
	{
		AsColor32 = eg_color32( GetRValue(ColorInfo.rgbResult) , GetGValue(ColorInfo.rgbResult) , GetBValue(ColorInfo.rgbResult) );
		OutColor = eg_color(AsColor32);
		OutColor.a = Alpha;
		return true;
	}
	return false;
}