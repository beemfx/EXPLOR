// (c) 2018 Beem Media

#pragma once

struct egRflEditor;

class EGReflectionDeserializer
{
private:

	EGArray<egRflEditor*> m_PropReadStack;
	eg_d_string           m_RefFilename;
	eg_bool               m_bHasReadSelf = false;
	eg_bool               m_bCanPasteFirstProperty = false;

public:

	void Init( egRflEditor& BaseProperty , eg_cpstr RefFilename );
	void Deinit();
	void OnXmlTagBegin( const eg_string_base& Tag , const class EGXmlAttrGetter& AttGet );
	void OnXmlTagEnd( const eg_string_base& Tag );
	void SetCanPasteFirstProperty( eg_bool bNewValue ) { m_bCanPasteFirstProperty = bNewValue; }
};
