// (c) 2018 Beem Media

#pragma once

#include "EGReflectionDeserializer.h"
#include "EGXMLBase.h"

class EGCopyPasteRflDeserializer : public IXmlBase
{
private:

	EGReflectionDeserializer m_Deserializer;

public:

	EGCopyPasteRflDeserializer( egRflEditor* PasteToProperty , const void* Buffer , eg_size_t BufferSize );
	~EGCopyPasteRflDeserializer();

	virtual void OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet ) override;
	virtual void OnTagEnd(const eg_string_base& Tag ) override;
	virtual eg_cpstr XMLObjName()const override { return "EGCopyPasteReflectionDeserializer"; }
};
