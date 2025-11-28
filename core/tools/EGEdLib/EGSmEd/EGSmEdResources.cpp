// (c) 2017 Beem Media

#include "EGSmEdResources.h"
#include "EGSmEdVarMgr.h"
#include "EGSmFile2.h"
#include "EGWndBase.h"
#include "../EGEdResLib/resource.h"

static HCURSOR g_CursorNormal = nullptr;
static HCURSOR g_CursorGrab = nullptr;
static HMENU g_MenuNewNode = nullptr;
static HMENU g_MenuNativeEvents = nullptr;

static struct egStateUiConfig
{
	const egsm_node_t Type;
	const COLORREF    StateColor;
	HBRUSH            Brush;
}
EGSmEdResources_UiConfig[] =
{
	#define DECL_NODE( _name_ , _color_ ) { egsm_node_t::_name_ , _color_ , nullptr },
	#include "EGSmNodes.items"
};

static egsmEdNodeFormatInfo EGSmEdResources_NodeInfos[] =
{
	#define EGDECL( _fname_ , _fstring_ , _ncolor_ ) { eg_crc(#_fname_) , _fstring_ , _ncolor_ , true , nullptr },
	#include "EGSmEdNode.items"
};

static egsmEdNodeFormatInfo EGSmEdResources_DefaultNodeInfo = { CT_Clear , "%s\n%s\n%s" , RGB(100,0,0) , false , nullptr };


void EGSmEdResources_Init( HINSTANCE hInst )
{
	unused( hInst );

	for( egStateUiConfig& Config : EGSmEdResources_UiConfig )
	{
		Config.Brush = EGWndBase::CreateSolidBrush( Config.StateColor );
	}

	for( egsmEdNodeFormatInfo& FormatInfo : EGSmEdResources_NodeInfos )
	{
		FormatInfo.Brush = EGWndBase::CreateSolidBrush( FormatInfo.Color );
	}
	EGSmEdResources_DefaultNodeInfo.Brush =  EGWndBase::CreateSolidBrush( EGSmEdResources_DefaultNodeInfo.Color );

	g_CursorNormal = LoadCursorW( nullptr , IDC_ARROW );
	g_CursorGrab = LoadCursorW( nullptr , IDC_SIZEALL );

	EGSmEdResources_RebuildContextMenus();
}

void EGSmEdResources_Deinit( HINSTANCE hInst )
{
	unused( hInst );

	DestroyMenu( g_MenuNewNode );
	g_MenuNewNode = nullptr;
	
	DeleteObject( g_CursorGrab );
	DeleteObject( g_CursorNormal );

	DeleteObject( EGSmEdResources_DefaultNodeInfo.Brush );
	for( egsmEdNodeFormatInfo& FormatInfo : EGSmEdResources_NodeInfos )
	{
		DeleteObject( FormatInfo.Brush );
	}
	
	for( egStateUiConfig& Config : EGSmEdResources_UiConfig )
	{
		DeleteObject( Config.Brush );
		Config.Brush = nullptr;
	}
}

const egsmEdNodeFormatInfo& EGSmEdResources_GetNodeInfo( eg_string_crc NativeEventName )
{
	for( const egsmEdNodeFormatInfo& NodeInfo : EGSmEdResources_NodeInfos )
	{
		if( NodeInfo.NativeEventName == NativeEventName )
		{
			return NodeInfo;
		}
	}
	return EGSmEdResources_DefaultNodeInfo;
}

void EGSmEdResources_RebuildContextMenus()
{
	if( g_MenuNewNode )
	{
		DestroyMenu( g_MenuNewNode );
		g_MenuNewNode = nullptr;
	}

	if( g_MenuNativeEvents )
	{
		DestroyMenu( g_MenuNativeEvents );
		g_MenuNativeEvents = nullptr;
	}

	UINT InsertPos = 0;
	auto InsertMenuOption = [&InsertPos]( HMENU PopupMenu , eg_cpstr16 Text , UINT Id , UINT Offset ) -> void
	{
		eg_char16 TempText[eg_string::STR_SIZE];
		EGString_Copy( TempText , Text , countof(TempText) );

		MENUITEMINFOW mif;
		zero( &mif );
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_STRING|MIIM_ID;
		mif.wID = Id+Offset;
		mif.fType = MFT_STRING;
		mif.dwTypeData = TempText;
		mif.fState = MFS_ENABLED;
		InsertMenuItemW( PopupMenu , InsertPos , TRUE , &mif );
		InsertPos++;
	};

	auto InsertSubMenu = [&InsertPos]( HMENU PopupMenu , HMENU SubMenu , eg_cpstr16 Text , UINT Id , UINT Offset ) -> void
	{
		eg_char16 TempText[eg_string::STR_SIZE];
		EGString_Copy( TempText , Text , countof(TempText) );

		MENUITEMINFOW mif;
		zero( &mif );
		mif.cbSize = sizeof(mif);
		mif.fMask = MIIM_STRING|MIIM_ID|MIIM_SUBMENU;
		mif.wID = Id+Offset;
		mif.fType = MFT_STRING;
		mif.dwTypeData = TempText;
		mif.fState = MFS_ENABLED;
		mif.hSubMenu = SubMenu;
		InsertMenuItemW( PopupMenu , InsertPos , TRUE , &mif );
		InsertPos++;
	};

	// New Event Menu
	{
		g_MenuNativeEvents = CreatePopupMenu();
		InsertPos = 0;
		EGArray<egsmVarDeclScr> Functions;
		EGSmEdVarMgr_GetFunctions( Functions );
		InsertMenuOption( g_MenuNativeEvents , L"Generic Event" , 0 , EGSM_GENERIC_NATIVE_EVENT_ID_OFFSET );
		for( UINT i=0; i<Functions.Len(); i++ )
		{
			const egsmVarDeclScr& VarDecl = Functions[i];
			InsertMenuOption( g_MenuNativeEvents , EGString_ToWide( VarDecl.Name ) , i , EGSM_NATIVE_EVENT_ID_OFFSET );
		}
	}

	// New Node Menu
	{
		g_MenuNewNode = CreatePopupMenu();
		InsertPos = 0;

		InsertSubMenu( g_MenuNewNode , g_MenuNativeEvents , L"New Event" , static_cast<UINT>( egsm_node_t::NATIVE_EVENT ) , EGSM_NEW_NODE_ID_OFFSET );

		auto CanCreateNodeType = []( egsm_node_t Type ) -> eg_bool
		{
			if( Type == egsm_node_t::UNKNOWN )
			{
				return false;
			}

			if( Type == egsm_node_t::NATIVE_EVENT )
			{
				return false; // Special type.
			}

			return true;
		};

		InsertMenuOption( g_MenuNewNode , L"Paste" , EGCE_SCRIPT_PASTE , 0 );

		#define DECL_NODE( _name_ , _color_ ) if( CanCreateNodeType( egsm_node_t::_name_ ) ){ InsertMenuOption( g_MenuNewNode , EGString_ToWide( EGString_Format( "New %s" , EGSmFile2::NodeTypeToDisplayString( egsm_node_t::_name_ ).String() ) ) , static_cast<UINT>( egsm_node_t::_name_ ) , EGSM_NEW_NODE_ID_OFFSET ); }
		#include "EGSmNodes.items"
	}
}

HBRUSH EGSmEdResources_GetHeaderBrush( egsm_node_t NodeType , eg_string_crc NativeEventType )
{
	if( NodeType == egsm_node_t::NATIVE_EVENT )
	{
		return EGSmEdResources_GetNodeInfo( NativeEventType ).Brush;
	}

	for( const egStateUiConfig& Config : EGSmEdResources_UiConfig )
	{
		if( Config.Type == NodeType )
		{
			return Config.Brush;
		}
	}

	return reinterpret_cast<HBRUSH>(GetStockObject( WHITE_BRUSH ));
}

COLORREF EGSmEdResources_GetHeaderColor( egsm_node_t NodeType , eg_string_crc NativeEventType )
{
	if( NodeType == egsm_node_t::NATIVE_EVENT )
	{
		return EGSmEdResources_GetNodeInfo( NativeEventType ).Color;
	}

	for( const egStateUiConfig& Config : EGSmEdResources_UiConfig )
	{
		if( Config.Type == NodeType )
		{
			return Config.StateColor;
		}
	}

	return RGB(255,255,255);
}

HCURSOR EGSmEdResources_GetCursor( egsm_cursor_t Type )
{
	switch( Type )
	{
	case egsm_cursor_t::NORMAL: return g_CursorNormal;
	case egsm_cursor_t::GRAB: return g_CursorGrab;
	}

	return g_CursorNormal;
}

HMENU EGSmEdResources_GetMenu( egsm_menu_t Type )
{
	switch( Type )
	{
	case egsm_menu_t::CONTEXT_NEW_NODE: return g_MenuNewNode;
	}

	return nullptr;
}