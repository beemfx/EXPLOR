// (c) 2018 Beem Media

#include "EGCopyPasteDeserializer.h"

EGCopyPasteRflDeserializer::EGCopyPasteRflDeserializer( egRflEditor* PasteToProperty , const void* Buffer , eg_size_t BufferSize )
{
	if( PasteToProperty )
	{
		m_Deserializer.Init( *PasteToProperty, "Pasted" );
		m_Deserializer.SetCanPasteFirstProperty( true );

		XMLLoad( Buffer , BufferSize , "Pasted" );
	}
}

EGCopyPasteRflDeserializer::~EGCopyPasteRflDeserializer()
{
	m_Deserializer.Deinit();
}

void EGCopyPasteRflDeserializer::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	m_Deserializer.OnXmlTagBegin( Tag , AttGet );
}

void EGCopyPasteRflDeserializer::OnTagEnd( const eg_string_base& Tag )
{
	m_Deserializer.OnXmlTagEnd( Tag );
}

