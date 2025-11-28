// (c) 2017 Beem Media
#pragma once

#include "EGWnd.h"
#include "EGSmTypes.h"

static const UINT EGSM_NEW_NODE_ID_OFFSET = 1500;
static const UINT EGSM_GENERIC_NATIVE_EVENT_ID_OFFSET = 2000;
static const UINT EGSM_NATIVE_EVENT_ID_OFFSET = 3000;
static const UINT EGSM_MAX_NATIVE_EVENT = 10000;

extern const eg_char16 EGSmEd_Title[];

enum class egsm_cursor_t
{
	NORMAL,
	GRAB,
};

enum class egsm_menu_t
{
	CONTEXT_NEW_NODE,
};

struct egsmEdNodeFormatInfo
{
	const eg_string_crc NativeEventName;
	eg_cpstr const      FormatString;
	const COLORREF      Color;
	const eg_bool       bIsIntrinsic;
	HBRUSH              Brush;
};


void EGSmEdResources_Init( HINSTANCE hInst );
void EGSmEdResources_Deinit( HINSTANCE hInst );
void EGSmEdResources_RebuildContextMenus();

static inline POINT EGSmEdResources_MakePoint( LPARAM lParam )
{
	POINT Point = { static_cast<SHORT>(LOWORD(lParam)) , static_cast<SHORT>(HIWORD(lParam)) };
	return Point;
}

const egsmEdNodeFormatInfo& EGSmEdResources_GetNodeInfo( eg_string_crc NativeEventName );
HBRUSH EGSmEdResources_GetHeaderBrush( egsm_node_t NodeType , eg_string_crc NativeEventType = CT_Clear );
COLORREF EGSmEdResources_GetHeaderColor( egsm_node_t NodeType , eg_string_crc NativeEventType = CT_Clear );
HCURSOR EGSmEdResources_GetCursor( egsm_cursor_t Type );
HMENU EGSmEdResources_GetMenu( egsm_menu_t Type );