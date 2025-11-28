// (c) 2017 Beem Media

#pragma once

#define DISABLE_W 4530 4820 4668
#pragma warning(push)
#pragma warning(disable:DISABLE_W)
#if !defined( WIN32_LEAN_AND_MEAN )
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <WinUser.h>
#include <shellapi.h>
#include <MMSystem.h>
#include <ShlObj.h>
#include <commdlg.h>
#include <Richedit.h>
#include <TextServ.h>

#pragma warning(pop)
#undef DISABLE_W

#undef CreateWindow
#undef CreateWindowEx
#undef RegisterClass
#undef RegisterClassEx
#undef SetWindowText
#undef DrawText
#undef DrawTextEx
#undef TextOut
#undef LoadMenu
#undef LoadImage
#undef LoadLibrary
#undef CreateFile
#undef OpenFile
#undef GetSaveFileName
#undef GetOpenFileName
#undef SHBrowseForFolder
#undef ChooseColor
#undef DefWindowProc
#undef DefDlgProc
#undef FindWindow
#undef FindWindowEx
#undef WNDCLASS
#undef WNDCLASSEX
#undef GetTextExtentPoint32
#undef ExtTextOut
#undef PeekMessage
#undef GetMessage
#undef DispatchMessage
#undef MessageBox
#undef LoadIcon
#undef LoadCursor
#undef GetModuleHandle
#undef UnregisterClass
#undef FindFirstFile
#undef FindFirstFileEx
#undef WIN32_FIND_DATA
#undef SystemParametersInfo
#undef GetClassName
#undef GetObject
#undef InsertMenuItem
#undef MENUITEMINFO
