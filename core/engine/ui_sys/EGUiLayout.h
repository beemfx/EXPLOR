// (c) 2017 Beem Media

#pragma once

#include "EGXMLBase.h"
#include "EGList.h"
#include "EGStringMap.h"
#include "EGEngineTemplates.h"
#include "EGRendererTypes.h"
#include "EGEngineSerializeTypes.h"
#include "EGClassName.h"
#include "EGUiLayout.reflection.h"

struct egUiWidgetInfo;
class EGReflectionDeserializer;
class EGFileData;

extern EGClass& EGMenu_GetClass();

egreflect class EGUiLayout : public IXmlBase
{
	
public:

	egprop eg_d_string   m_Id        = "";
	egprop eg_class_name m_ClassName = &EGMenu_GetClass();
	egprop eg_bool       m_NoInput   = false;

	egRflEditor m_Editor;
	eg_bool     m_IsInTool = false;

	EGList<egUiWidgetInfo> m_Objects = EGList<egUiWidgetInfo>::DEFAULT_ID;

public:

	EGUiLayout();
	~EGUiLayout(){ Clear(); }
	void Load( eg_cpstr Filename );
	void Load( const void* Data , eg_size_t DataSize , eg_cpstr RefName , eg_bool IsInTool );
	void LoadForResave( EGFileData& FileData , eg_cpstr RefName );
	void SaveTo( EGFileData& FileOut );
	void Clear();

	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts) override final;
	virtual void OnTagEnd(const eg_string_base& Tag ) override final;
	virtual eg_cpstr XMLObjName()const;

	//
	// EGLayout Editor Functions
	//
	void CreateNewForTool();
	void InsertObject( eg_cpstr StrDef , eg_real x , eg_real y );
	void InsertImage( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertClearZ( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertText( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertLight( struct egUiWidgetInfo* ObjToInsertBefore );
	void InsertCamera( struct egUiWidgetInfo* ObjToInsertBefore );
	eg_bool MoveObject( egUiWidgetInfo* ObjToMove , egUiWidgetInfo* ObjToMoveBefore );
	void DeleteObject( egUiWidgetInfo* ObjToDelete );
	void OnToolFullRebuild();
	egRflEditor* GetEditor() { return &m_Editor; }

	eg_cpstr GetFilename()const{ return GetXmlFilename(); }

private:

	EGReflectionDeserializer* m_Deserializer = nullptr;

private:

	void OnTag_emenu( const EGXmlAttrGetter& AttGet );

	void FinalizeLoad( eg_bool bForToolRebuild = false );
	egUiWidgetInfo* FindObjById( eg_string_crc Id );
};

class EGUiLayoutMgr
{
public:
	static EGUiLayoutMgr* Get(){ return &s_LayoutMgr; }

	void Init();
	void Deinit();

	const EGUiLayout* GetLayout( eg_string_crc MenuId );
private:
	EGUiLayoutMgr(){ }
	~EGUiLayoutMgr(){ }
	void Init_AddLayout( eg_cpstr Filename );
	static eg_string_crc FilenameToMenuCrc( eg_cpstr Filename );

private:

	EGSysMemItemMap<EGUiLayout*> m_LayoutMap;

	EGUiLayout m_DefaultLayout;
	static EGUiLayoutMgr s_LayoutMgr;
};