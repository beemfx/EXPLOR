// (c) 2016 Beem Media

#pragma once

#include "EGUiLayout.h"

class EGMenu;
struct egUiWidgetInfo;

class EGLytEdFile
{
private:
	EGMenu*      m_Menu = nullptr;
	eg_real      m_PreviewRatio = 16.f/9.f;
	eg_vec2      m_LastMousePos = eg_vec2(0.f,0.f);
	eg_vec2      m_NextMousePos = eg_vec2(0.f,0.f);
	EGUiLayout   m_MenuLayout;
	eg_bool      m_bShowTextOutlines = false;

	static EGLytEdFile GlobalLayoutFile;

public:
	static EGLytEdFile& Get(){ return GlobalLayoutFile; }

	void Init();
	void Deinit();
	void RefreshSettingsPanel();
	void SetPreviewAspectRatio( eg_real NewRatio );
	eg_real GetPreviewAspectRatio() const { return m_PreviewRatio; }
	void SetMousePos( eg_real x , eg_real y ){ m_NextMousePos = eg_vec2(x,y); }
	egUiWidgetInfo* GetObjByMousePos( eg_real x, eg_real y );
	void Update( eg_real DeltaTime );
	void Draw();
	void Open( eg_cpstr16 Filename );
	eg_bool Save( eg_cpstr16 Filename );
	void InitMenu();
	void DeinitMenu();
	void InsertObject( eg_cpstr StrDef, eg_real x, eg_real y );
	void InsertImage( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertClearZ( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertText( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertLight( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertCamera( struct egUiWidgetInfo* ObjToInsertBefore );
	void MoveObject( struct egUiWidgetInfo* ObjToMove, struct egUiWidgetInfo* ObjToMoveBefore );
	void DeleteObject( struct egUiWidgetInfo* ObjToMove );
	void QueryEvents( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	void QueryTextNodes( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	void QueryBones( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	void QueryPossibleScrollbars( const egUiWidgetInfo* ObjToQuery , EGArray<eg_d_string>& Out );
	eg_size_t GetNumObjects();
	struct egUiWidgetInfo* GetObjectInfoByIndex( eg_size_t Index );
	void OnWasChanged( egUiWidgetInfo* Item , eg_bool bFullRebuild );
	void FullRebuild();
	void SetAllTextToBWNoShadow();
	void ToggleShowTextOutlines(){ m_bShowTextOutlines = !m_bShowTextOutlines; }

	static void StaticQueryEvents( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	static void StaticQueryTextNodes( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	static void StaticQueryBones( const egUiWidgetInfo* ObjToQuery, EGArray<eg_d_string>& Out );
	static void StaticQueryPossibleScrollbars( const egUiWidgetInfo* ObjToQuery , EGArray<eg_d_string>& Out );
};