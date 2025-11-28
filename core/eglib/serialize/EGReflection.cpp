// (c) 2018 Beem Media

#include "EGReflection.h"
#include "EGFileData.h"
#include "EGCopyPasteDeserializer.h"

const eg_mem_pool egRflEditor::s_MemPool = eg_mem_pool::DefaultHi;
egRflStructType Rfl_CustomEditor( "Custom" );

void egRflEditor::Rebuild()
{
	if( Data && Type )
	{
		if( GetType() == eg_rfl_value_t::Struct )
		{
			for( egRflEditor& Child : Children )
			{
				Child.bMarkedAsFound = false;
			}

			auto FindCurrentChildStruct = [this]( eg_cpstr Name ) -> egRflEditor*
			{
				for( egRflEditor& Child : Children )
				{
					if( EGString_Equals( Child.GetVarName() , Name ) )
					{
						return &Child;
					}
				}

				return nullptr;
			};

			ChildVars.Clear( false );
			Type->GetChildren( ChildVars );
			for( const egRflVar& Var : ChildVars )
			{
				egRflEditor* CurrentChild = FindCurrentChildStruct( Var.GetName() );
				if( CurrentChild )
				{
					assert( CurrentChild->Type == Var.GetType() );
					CurrentChild->Data = Data + Var.GetOffset();
					CurrentChild->bMarkedAsFound = true;
					CurrentChild->Rebuild();
				}
				else
				{
					egRflEditor NewChild( Var , Data + Var.GetOffset() );
					Children.AppendMove( NewChild );
				}
			}

			// Really nothing should have changed about the structure, but just for future proofness.
			Children.DeleteAllByPredicate( []( egRflEditor& Child ) -> eg_bool { return !Child.bMarkedAsFound; } );
		}

		if( GetType() == eg_rfl_value_t::Array )
		{
			IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
			const eg_size_t PrevArraySize = Children.Len();
			const eg_size_t NewArraySize = AsArray->RflGetArrayLen();
			for( eg_size_t i=0; i<NewArraySize; i++ )
			{
				if( i < PrevArraySize )
				{
					assert( Children[i].Type == Type->GetArrayChildType() );
					Children[i].Data = reinterpret_cast<eg_byte*>(AsArray->RflGetArrayItemAt( i ));
					Children[i].Rebuild();
				}
				else
				{
					egRflEditor NewChild( AsArray->RflGetArrayItemAt( i ) , Type->GetArrayChildType() );
					Children.AppendMove( NewChild );
				}
			}

			// The array may have shrunk when deleting an item,
			// so clear out any excess.
			assert( Children.Len() >= NewArraySize );
			Children.Resize( NewArraySize );
		}
	}
}

void egRflEditor::HandleArrayChanged()
{
	Rebuild();
}

void egRflEditor::HandlePostLoad()
{
	HandleArrayChanged();
}

void egRflEditor::DebugPrint( eg_uint Level ) const
{
	eg_d_string Output;
	for( eg_uint i=0; i<Level; i++ )
	{
		Output.Append( "   " );
	}

	eg_bool bPrintChildren = false;

	switch( GetType() )
	{
		case eg_rfl_value_t::Unknown:
			Output.Append( "!-unknown-!" );
			break;
		case eg_rfl_value_t::Primitive:
			Output.Append( EGString_Format("%s = \"%s\"" , GetVarName() , *ToString() ) );
			break;
		case eg_rfl_value_t::Struct:
			Output.Append( EGString_Format("Struct %s" , GetVarName() ) );
			bPrintChildren = true;
			break;
		case eg_rfl_value_t::Array:
			Output.Append( EGString_Format("Array %s" , GetVarName() ) );
			bPrintChildren = true;
			break;
	}

	EGLogf( eg_log_t::General , "%s" , *Output );

	if( bPrintChildren )
	{
		for( const egRflEditor& Child : Children )
		{
			Child.DebugPrint( Level + 1 );
		}
	}
}

void egRflEditor::SetRawData( const void* RawData, eg_size_t RawDataSize )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Primitive && RawDataSize == Type->GetPrimitiveSize() )
	{
		EGMem_Copy( Data , RawData , RawDataSize );
	}
	else
	{
		assert( false ); // Cannot set raw data on this type.
	}
}

void egRflEditor::SetFromString( eg_cpstr Str )
{
	if( IsValid() )
	{
		Type->SetFromString( Data , Str );
	}
}

eg_d_string egRflEditor::ToString() const
{
	if( IsValid() )
	{
		return Type->ToString( Data );
	}
	return "";
}

egRflEditor egRflEditor::GetChild( eg_cpstr VarName )
{
	if( IsValid() )
	{
		for( egRflEditor& Child : Children )
		{
			if( EGString_Equals( Child.GetVarName() , VarName ) )
			{
				return Child;
			}
		}
	}
	return egRflEditor();
}

egRflEditor egRflEditor::GetArrayChild( eg_size_t Index )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array && Children.IsValidIndex( Index ) )
	{
		return Children[ Index ];
	}

	return egRflEditor();
}

egRflEditor egRflEditor::InsertArrayChild()
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflArrayAppendItem();

		HandleArrayChanged();

		return Children.Last();
	}
	else
	{
		assert( false ); // Not an array
	}

	return egRflEditor();
}

egRflEditor* egRflEditor::GetChildPtr( eg_cpstr VarName )
{
	if( IsValid() )
	{
		for( egRflEditor& Child : Children )
		{
			if( EGString_Equals( Child.GetVarName() , VarName ) )
			{
				return &Child;
			}
		}
	}
	return nullptr;
}

egRflEditor* egRflEditor::GetArrayChildPtr( eg_size_t Index )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array && Children.IsValidIndex( Index ) )
	{
		return &Children[Index];
	}

	return nullptr;
}

egRflEditor* egRflEditor::InsertArrayChildPtr()
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflArrayAppendItem();

		HandleArrayChanged();

		return &Children.Last();
	}
	else
	{
		assert( false ); // Not an array
	}

	return nullptr;
}


void egRflEditor::DeleteArrayChildAt( eg_size_t Index )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflArrayDeleteItemAt( Index );

		HandleArrayChanged();
	}
	else
	{
		assert( false ); // Not an array
	}
}

void egRflEditor::ClearArray()
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflClear();

		HandleArrayChanged();
	}
	else
	{
		assert( false ); // Not an array
	}
}

void egRflEditor::ReserveArraySpace( eg_size_t Size )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflReserveArraySpace( Size );

		HandleArrayChanged();
	}
	else
	{
		assert( false ); // Not an array
	}
}

void egRflEditor::TrimArrayReserved()
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflTrimArrayReserved();

		HandleArrayChanged();
	}
	else
	{
		assert( false ); // Not an array
	}
}


void egRflEditor::Serialize( eg_rfl_serialize_fmt Format , eg_int Depth , EGFileData& MemFile ) const
{
	if( Format != eg_rfl_serialize_fmt::XML )
	{
		return;
	}

	eg_string_small TabStr( CT_Clear );
	for( eg_int i=0; i<Depth; i++ )
	{
		TabStr.Append( '\t' );
	}

	auto Write = [&MemFile,&TabStr]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		MemFile.WriteStr8( TabStr );
		MemFile.WriteStr8( Buffer );
	};

	// Recursively serialize:

	if( bIsSerialized && Type )
	{
		switch( Type->GetType() )
		{
			case eg_rfl_value_t::Unknown:
				break;
			case eg_rfl_value_t::Primitive:
			{
				Write( "<property name=\"%s\" value=\"%s\" />\r\n" , GetVarName() , EGString_ToXmlFriendly(*ToString()).String() );
			} break;
			case eg_rfl_value_t::Struct:
			{
				Write( "<struct name=\"%s\" ed_expanded=\"%s\" >\r\n" , GetVarName() , bIsExpanded ? "true" : "false" );
				for( const egRflEditor& Item : Children )
				{
					Item.Serialize( Format , Depth+1 , MemFile );
				}
				Write( "</struct>\r\n" );
			} break;
			case eg_rfl_value_t::Array:
			{
				if( Children.Len() > 0 )
				{
					Write( "<array name=\"%s\" ed_expanded=\"%s\" reserve_size=\"%d\" >\r\n" , GetVarName() , bIsExpanded ? "true" : "false" , Children.LenAs<eg_int>() );
					for( const egRflEditor& Item : Children )
					{
						Item.Serialize( Format , Depth+1 , MemFile );
					}
					Write( "</array>\r\n" );
				}
			} break;
		}
	}
}

void egRflEditor::SetChildValue( eg_cpstr VarName, eg_cpstr NewValue )
{
	egRflEditor Child = GetChild( VarName );
	if( Child.IsValid() )
	{
		Child.SetFromString( NewValue );
	}
	else
	{
		assert( false ); // No such child.
	}
}

void egRflEditor::GetChildren( EGArray<egRflEditor>& Out )
{
	Out = Children;
}

void egRflEditor::GetChildrenPtrs( EGArray<egRflEditor*>& Out )
{
	Out.Reserve( Out.Len() + Children.Len() );

	for( egRflEditor& Child : Children )
	{
		Out.Append( &Child );
	}
}

void egRflEditor::DeleteArrayChild( egRflEditor* Child )
{
	for( eg_size_t i=0; i<Children.Len(); i++ )
	{
		if( Child ==  &Children[i] )
		{
			DeleteArrayChildAt( i );
			return;
		}
	}
}

void egRflEditor::InsertArrayChildAt( eg_size_t Index )
{
	if( IsValid() && GetType() == eg_rfl_value_t::Array )
	{
		IEGArrayRflI* AsArray = reinterpret_cast<IEGArrayRflI*>(Data);
		AsArray->RflArrayInsertItemAt( Index );

		HandleArrayChanged();

		for( eg_size_t i=0; i<Children.Len(); i++ )
		{
			// Just because inserting an array item doesn't actually move the
			// reflection info we'll reset all reflection properties
			// (serializing and editing should never change on array children
			// but collapsed state will so we'll just set all but the
			// newly added one to collapsed).
			Children[i].bIsExpanded = (i==Index);
		}
	}
	else
	{
		assert( false ); // Not an array
	}
}

void egRflEditor::InsertArrayChildBefore( egRflEditor* Child )
{
	for( eg_size_t i=0; i<Children.Len(); i++ )
	{
		if( Child ==  &Children[i] )
		{
			InsertArrayChildAt( i );
			return;
		}
	}
}

eg_d_string egRflEditor::GetPreviewValue() const
{
	if( GetType() == eg_rfl_value_t::Array )
	{
		return EGString_Format( "%d Items" , Children.LenAs<eg_int>() ).String();
	}
	else if( GetType() == eg_rfl_value_t::Struct )
	{
		if( Children.IsValidIndex( 0 ) )
		{
			return Children[0].GetPreviewValue();
		}
		return "No Properties";
	}
	return EGString_Format( "%s: %s" , GetDisplayName() , *ToString() ).String();
}

void egRflEditor::GetComboChoices( EGArray<eg_d_string>& Out, eg_bool& bManualEditOut ) const
{
	if( Type )
	{
		Type->GetComboChoices( Data , Out , bManualEditOut );
	}
}

egRflEditor::egRflEditor()
: Children( s_MemPool )
{

}

egRflEditor::egRflEditor( eg_ctor_t Ct ) 
: Children( s_MemPool )
{
	if( Ct == CT_Clear || Ct == CT_Default )
	{
		*this = egRflEditor();
	}
	else
	{
		assert( false );
	}
}

egRflEditor::egRflEditor( eg_cpstr InName, void* InData, const egRflValueType* InType )
: Children( s_MemPool )
, NameAsDynamicString( InName )
, NameAsStaticString( nullptr )
, NameAsCrc( InName )
, Data( reinterpret_cast<eg_byte*>( InData ) )
, Type( InType )
{
	Rebuild();
}

egRflEditor::egRflEditor( const egRflVar& Var, void* InData )
: Children( s_MemPool )
, NameAsDynamicString( CT_Clear )
, NameAsStaticString( Var.GetName() )
, NameAsCrc( Var.GetName() )
, Data( reinterpret_cast<eg_byte*>(InData) )
, Type( Var.GetType() )
{
	Rebuild();
}

egRflEditor::egRflEditor( void* InData, const egRflValueType* InType )
: Children( s_MemPool )
, NameAsDynamicString( CT_Clear )
, NameAsStaticString( "-" )
, NameAsCrc( "-" )
, Data( reinterpret_cast<eg_byte*>( InData ) )
, Type( InType )
{
	Rebuild();
}

void egRflEditor::ExecutePostLoad( eg_cpstr8 Filename , eg_bool bForEditor )
{
	if( Type )
	{
		Type->PostLoad( Data , Filename , bForEditor );
	}
	
	for( egRflEditor& Child : Children )
	{
		Child.ExecutePostLoad( Filename , bForEditor );
	}
}

void egRflEditor::AddExplicitChild( const egRflEditor& ChildEditor )
{
	Children.Append( ChildEditor );
}

eg_cpstr egRflEditor::GetDisplayName() const
{
	if( DisplayName.Len() == 0 )
	{
		DisplayName = BeautifyVarNameAsDisplayName( GetVarName() );
	}
	return *DisplayName;
}

eg_string_big egRflEditor::BeautifyVarNameAsDisplayName( eg_cpstr InStr )
{
	eg_string_big Modified = InStr;

	auto IsCapital = []( eg_char Char ) -> eg_bool
	{
		return ('A' <= Char && Char <= 'Z') || ('0' <= Char && Char <= '9');
	};

	// Get rid of hidden array names:
	Modified.Replace( ".-", "" );

	// Use only the last part of the name (after last '.'):
	eg_size_t Len = Modified.Len();
	for( eg_size_t i=0; i<Len; i++ )
	{
		if( Modified[Len-1-i] == '.' )
		{
			eg_string Short = &Modified[Len-i];
			Modified = Short;
			break;
		}
	}

	// Clamp off leading "m_":
	if( EGString_EqualsCount( Modified , "m_" , 2 ) )
	{
		eg_string Shorter = &Modified[2u];
		Modified = Shorter;
	}

	// Clamp off leading "b" for bools
	if( Modified.Len() >= 2 && Modified[0u] == 'b' && IsCapital( Modified[1u] ) )
	{
		eg_string Shorter = &Modified[1u];
		Modified = Shorter;
	}

	// Put spaces between words:
	eg_string WordsSeparated( CT_Clear );
	Len = Modified.Len();
	eg_bool bLastWasCapital = true;
	for( eg_size_t i=0; i<Len; i++ )
	{
		eg_char c = Modified[i];
		eg_bool bCurIsCapital = IsCapital( c );
		if( !bLastWasCapital && bCurIsCapital )
		{
			WordsSeparated.Append( ' ' );
		}
		WordsSeparated.Append( c );
		bLastWasCapital = bCurIsCapital;
	}
	Modified = WordsSeparated;

	return Modified;
}

void egRflEditor::CopyFrom( const egRflEditor& SourceEditor )
{
	EGFileData SourceData( eg_file_data_init_t::HasOwnMemory );
	SourceEditor.Serialize( eg_rfl_serialize_fmt::XML , 0 , SourceData );
	EGCopyPasteRflDeserializer DestDeserizer( this , SourceData.GetData() , SourceData.GetSize() );
}

egRflEditor EGReflection_GetEditorForClass( eg_cpstr ClassName , void* Data , eg_cpstr Name )
{
	return egRflEditor( Name , Data , egRflStructType::FindStructTypeByName( ClassName ) );
}

egRflStructType::EGRflStructList* egRflStructType::s_AllTypes = nullptr;
eg_byte egRflStructType::s_AllTypesMem[sizeof(egRflStructType::EGRflStructList)];

const egRflStructType* egRflStructType::FindStructTypeByName( eg_cpstr InName )
{
	if( s_AllTypes )
	{
		for( const egRflStructType* Type : *s_AllTypes )
		{
			if( EGString_Equals( Type->m_ClassName , InName ) )
			{
				return Type;
			}
		}
	}
	return nullptr;
}

egRflStructType::egRflStructType( eg_cpstr InClassName )
: m_ClassName( InClassName )
{
	if( nullptr == s_AllTypes )
	{
		s_AllTypes = new ( s_AllTypesMem ) EGRflStructList( EGRflStructList::DEFAULT_ID );
	}

	if( s_AllTypes )
	{
		s_AllTypes->InsertLast( this );
	}
}

egRflStructType::~egRflStructType()
{
	if( s_AllTypes )
	{
		s_AllTypes->Remove( this );	

		if( s_AllTypes->Len() == 0 )
		{
			s_AllTypes->~EGRflStructList();
			s_AllTypes = nullptr;
		}
	}
}
