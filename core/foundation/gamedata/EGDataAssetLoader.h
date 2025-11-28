// (c) 2018 Beem Media

#pragma once

#include "EGXMLBase.h"

class EGReflectionDeserializer;
class EGDataAsset;
class EGFileData;
struct egRflEditor;

class EGDataAssetLoader : public IXmlBase
{
private:

	EGClass*                  m_DataAssetClass = nullptr;
	EGDataAsset*              m_DataAsset = nullptr;
	EGReflectionDeserializer* m_RflDeserializer = nullptr;
	egRflEditor&              m_RflEditor;
	eg_mem_pool               m_MemPool = eg_mem_pool::Default;

public:

	EGDataAssetLoader( eg_cpstr16 InFilename , eg_mem_pool InMemPool , eg_bool bForEditor , egRflEditor& RflEditor );
	EGDataAssetLoader( const EGFileData& MemFile , eg_cpstr16 InRefFilename , eg_mem_pool InMemPool , eg_bool bForEditor , egRflEditor& RflEditor );
	~EGDataAssetLoader();

	EGDataAsset* GetDataAsset() { return m_DataAsset; }

private:

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) override final;
	virtual void OnTagEnd( const eg_string_base& Tag ) override final;
	virtual eg_cpstr XMLObjName() const { return "EGDataAssetLoader"; }
};

