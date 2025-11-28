/******************************************************************************
File: XMLBase.cpp
Class: IXmlBase
Purpose: See header file.

(c) 2011 Beem Software
******************************************************************************/

#include "EGXMLBase.h"
#include "EGLibExtern.h"
#include "EGFileData.h"

#define XML_STATIC
#include <expat/expat.h>

struct egXMLState
{
	typedef EGFixedArray<eg_string_big,10> EGTagStack;

	eg_char*   m_pData;
	eg_uint    m_nDataSize;
	EGTagStack m_TagStack;
	XML_Parser m_parser;
};

void IXmlBase::XMLLoad(eg_cpstr strFile)
{
	EGFileData fileScript( eg_file_data_init_t::HasOwnMemory );
	EGLibExtern_LoadNowTo(strFile, fileScript);

	if( fileScript.GetSize() > 0 )
	{
		XMLLoad( fileScript.GetData() , fileScript.GetSize() , strFile );
	}
	else
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ ": No such file \"%s\"." , strFile );
	}
}

void IXmlBase::XMLLoad( const void* Data , eg_size_t DataSize , eg_cpstr RefName )
{
	XML_Memory_Handling_Suite MemHandler;
	MemHandler.malloc_fcn  = EXPAT_Malloc;
	MemHandler.realloc_fcn = EXPAT_Realloc;
	MemHandler.free_fcn    = EXPAT_Free;
	if( nullptr != RefName )
	{
		EGString_Copy( m_strFilename , RefName , countof(m_strFilename) );
	}
	else
	{
		m_strFilename[0] = '\0';
	}

	egXMLState State;
	m_pXML = &State;
	m_pXML->m_nDataSize=0;
	m_pXML->m_pData=nullptr;
	m_pXML->m_parser = XML_ParserCreate_MM(nullptr, &MemHandler, nullptr);
	
	m_pXML->m_TagStack.Clear();
	XML_ParserReset(m_pXML->m_parser, nullptr);
	XML_SetUserData(m_pXML->m_parser, this);
	XML_SetElementHandler(m_pXML->m_parser, EXPAT_Start, EXPAT_End);
	XML_SetCharacterDataHandler(m_pXML->m_parser, EXPAT_CharData);
	

	//Parse:
	XML_Status nRes=XML_Parse(	m_pXML->m_parser,	static_cast<const char*>(Data), static_cast<int>(DataSize), 1);

	if( nRes != XML_STATUS_OK )
	{
		EGLogf( eg_log_t::Error , __FUNCTION__ " %s.XMLLoad ERROR: Line %u: \"%s\"." , XMLObjName() , XML_GetCurrentLineNumber(m_pXML->m_parser) , XML_ErrorString(XML_GetErrorCode(m_pXML->m_parser)) );
	}

	
	XML_ParserReset(m_pXML->m_parser, nullptr);
	XML_ParserFree(m_pXML->m_parser);

	//Delete the char data in case it was never deleted.
	FreeChars(m_pXML->m_pData);

	//Delete the XML state
	m_pXML = nullptr;

	return;
}

const eg_string_base& IXmlBase::GetXmlTagUp( eg_uint ParentLevels )const
{
	assert( ParentLevels < m_pXML->m_TagStack.Len() );
	eg_int Index = static_cast<eg_int>(m_pXML->m_TagStack.Len()-1)-ParentLevels;
	if( ParentLevels >= m_pXML->m_TagStack.Len() )
	{
		Index = 0; //This won't be returning the correct value in this case.
	}
	return m_pXML->m_TagStack[Index];
}

eg_uint IXmlBase::GetXmlTagLevels()const
{
	return static_cast<eg_uint>(m_pXML->m_TagStack.Len());
}

void IXmlBase::EXPAT_Start(void *userData, const eg_char *name, const eg_char **atts)
{
	IXmlBase* p = (IXmlBase*)userData;
	//At the beginning of Start and End we dump any data that might
	//be in the data buffer.
	EXPAT_DumpData(p);
	
	//Get tag, push it onto the stack, and then send it to the parent class.
	eg_string_big Tag=name;
	p->m_pXML->m_TagStack.Push(Tag);
	EGXmlAttrGetter AttGet( atts );
	p->OnTag( Tag , AttGet );
}

void IXmlBase::EXPAT_End(void *userData, const eg_char *name)
{
	IXmlBase* p = (IXmlBase*)userData;
	EXPAT_DumpData(p);
	
	eg_string_big Tag = name;
	//Since XML tags must be properly nested the tag on top
	//should be the tag to be removed from the stack. Expat
	//makes sure that is the case so it isn't necessary to check now.
	p->OnTagEnd(Tag);
	p->m_pXML->m_TagStack.Pop();
}

void IXmlBase::EXPAT_CharData(void *userData, const eg_char *s, int len)
{
	IXmlBase* p = (IXmlBase*)userData;

	//Because character data can be handled in chunks, a data buffer is
	//used to store all data until the next tag is found (whehther that
	//tag is the opening tag of a new tag, or the closing tag), at that
	//point the data is dumped to the parent class.
	eg_uint nOldSize=p->m_pXML->m_nDataSize;
	XML_Char* pOldData=p->m_pXML->m_pData;
	p->m_pXML->m_pData = nullptr;
	
	if(!nOldSize)
	{
		p->m_pXML->m_pData = AllocChars(len+1);
		assert_pointer(p->m_pXML->m_pData);
		
		EGMem_Copy(p->m_pXML->m_pData, s, len*sizeof(XML_Char));
		p->m_pXML->m_pData[len]=0;
		p->m_pXML->m_nDataSize=len;
	}
	else
	{
		p->m_pXML->m_pData = AllocChars(nOldSize+len+1);
		assert_pointer(p->m_pXML->m_pData);
		EGMem_Copy(p->m_pXML->m_pData, pOldData, nOldSize*sizeof(XML_Char));
		EGMem_Copy(p->m_pXML->m_pData+nOldSize, s, len*sizeof(XML_Char));
		p->m_pXML->m_pData[nOldSize+len]=0;
		p->m_pXML->m_nDataSize=nOldSize+len;	
	}

	FreeChars(pOldData);
}

eg_char* IXmlBase::AllocChars(eg_uint NumChars)
{
	return (XML_Char*)EXPAT_Malloc(NumChars*sizeof(XML_Char));

}

void IXmlBase::FreeChars(eg_char*& p)
{
	if(p)
	{
		EXPAT_Free(p);
	}
	p = nullptr;
}

void IXmlBase::EXPAT_DumpData(IXmlBase* p)
{
	if(p->m_pXML->m_nDataSize)
	{
		p->OnData(p->m_pXML->m_pData, p->m_pXML->m_nDataSize);	
	}

	p->m_pXML->m_nDataSize = 0;
	FreeChars(p->m_pXML->m_pData);
}


void* IXmlBase::EXPAT_Malloc(eg_size_t Size)
{
	return new eg_byte[Size];
}

void* IXmlBase::EXPAT_Realloc(void* p, eg_size_t Size)
{
	EXPAT_Free(p);
	return EXPAT_Malloc(Size);
}

void IXmlBase::EXPAT_Free(void* p)
{
	if(p)
	{
		delete[]p;
	}
}