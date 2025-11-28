// (c) 2018 Beem Media

#include "EGReflectionDeserializer.h"
#include "EGReflection.h"
#include "EGXMLBase.h"

void EGReflectionDeserializer::Init( egRflEditor& BaseProperty , eg_cpstr RefFilename )
{
	m_RefFilename = RefFilename;
	m_PropReadStack.Clear();
	m_bHasReadSelf = false;
	if( BaseProperty.IsValid() )
	{
		m_PropReadStack.Push( &BaseProperty );
	}
	else
	{
		assert( false ); // Nothing to read if there is no base type.
	}
}

void EGReflectionDeserializer::Deinit()
{
	m_RefFilename.Clear();
	m_PropReadStack.Clear();
	m_bHasReadSelf = false;
}

void EGReflectionDeserializer::OnXmlTagBegin( const eg_string_base& Tag , const class EGXmlAttrGetter& AttGet )
{
	eg_string_big PropertyName = AttGet.GetString( "name" );

	auto PrintDidNotReadError = [&PropertyName,&Tag,this]() -> void
	{
		EGLogf( eg_log_t::Warning , "Bad property serialization: \"%s\" was not read from %s in %s." , *PropertyName , Tag.String() , *m_RefFilename );
	};

	if( m_PropReadStack.IsEmpty() )
	{
		PrintDidNotReadError();
		return;
	}

	egRflEditor* pCurProp = m_PropReadStack.Top();

	if( !m_bHasReadSelf )
	{
		egRflEditor& CurProp = *pCurProp;
		if( EGString_Equals( CurProp.GetVarName() , *PropertyName ) || m_bCanPasteFirstProperty )
		{
			if( CurProp.IsPrimitive() )
			{
				CurProp.SetFromString( AttGet.GetString( "value" ) );
			}
			else if( CurProp.GetType() == eg_rfl_value_t::Struct || CurProp.GetType() == eg_rfl_value_t::Array )
			{
				CurProp.SetExpandedInEditor( AttGet.GetBool( "ed_expanded" , true ) );
				if( CurProp.GetType() == eg_rfl_value_t::Array )
				{
					CurProp.ClearArray();
					if( AttGet.DoesAttributeExist("reserve_size") )
					{
						const eg_uint ReserveSize = AttGet.GetUInt("reserve_size");
						CurProp.ReserveArraySpace( ReserveSize );
					}
				}
			}

			m_bHasReadSelf = true;
		}
		else
		{
			PrintDidNotReadError();
		}

		return;
	}

	egRflEditor* pCurPropChild = nullptr;

	if( pCurProp && pCurProp->IsValid() )
	{
		switch( pCurProp->GetType() )
		{
			case eg_rfl_value_t::Unknown:
			case eg_rfl_value_t::Primitive:
				break;
			case eg_rfl_value_t::Array:
				pCurPropChild = pCurProp->InsertArrayChildPtr();
				break;
			case eg_rfl_value_t::Struct:
				pCurPropChild = pCurProp->GetChildPtr( *PropertyName );
				break;
		}
	}

	if( Tag == "array" || Tag == "struct" )
	{
		if( pCurPropChild && pCurPropChild->IsValid() && ( pCurPropChild->GetType() == eg_rfl_value_t::Array || pCurPropChild->GetType() == eg_rfl_value_t::Struct ) )
		{
			egRflEditor& CurPropChild = *pCurPropChild;
			CurPropChild.SetExpandedInEditor( AttGet.GetBool( "ed_expanded" , true ) );
			if( CurPropChild.GetType() == eg_rfl_value_t::Array )
			{
				CurPropChild.ClearArray();
				if( AttGet.DoesAttributeExist("reserve_size") )
				{
					const eg_uint ReserveSize = AttGet.GetUInt("reserve_size");
					CurPropChild.ReserveArraySpace( ReserveSize );
				}
			}
			m_PropReadStack.Push( pCurPropChild );
		}
		else
		{
			m_PropReadStack.Push( nullptr );
			PrintDidNotReadError();
		}
	}
	else if( Tag == "property")
	{
		// If it's a property it's a primitive so just read the value in
		// (either finding the struct child or insert a new array item.)

		if( pCurPropChild && pCurPropChild->IsValid() && pCurPropChild->IsPrimitive() )
		{
			pCurPropChild->SetFromString( AttGet.GetString( "value" ) );
		}
		else
		{
			PrintDidNotReadError();
		}
	}
	else
	{
		PrintDidNotReadError();
	}
}

void EGReflectionDeserializer::OnXmlTagEnd( const eg_string_base& Tag )
{
	if( Tag == "array" || Tag == "struct" )
	{
		if( m_PropReadStack.HasItems() && m_PropReadStack.Top() && m_PropReadStack.Top()->IsValid() )
		{
			egRflEditor& Top = *m_PropReadStack.Top();
			if( Top.GetType() == eg_rfl_value_t::Array )
			{
				Top.TrimArrayReserved();
			}
		}
		m_PropReadStack.Pop();
	}
	else if( Tag == "property" )
	{
		// Nothing to do...
	}
	else
	{
		assert( false ); // What is this?
	}
}
