// (c) 2018 Beem Media

#include "EGWorldFile.h"
#include "EGFileData.h"
#include "EGReflectionDeserializer.h"
#include "EGWorldObject.h"
#include "EGEntWorld.h"

EG_CLASS_DECL( EGWorldFile )

void EGWorldFile::OnDestruct()
{
	Reset();

	Super::OnDestruct();
}

void EGWorldFile::Load( const EGFileData& MemFile , eg_cpstr RefFilename , eg_bool bForEditor )
{
	Reset();

	XMLLoad( MemFile.GetData() , MemFile.GetSize() , RefFilename );

	m_GlobalsEd.ExecutePostLoad( RefFilename , bForEditor );
	m_EditorConfigEd.ExecutePostLoad( RefFilename , bForEditor );
	EGArray<EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( EGWorldObject* Obj : AllObjs )
	{
		Obj->GetEditor()->ExecutePostLoad( RefFilename , bForEditor );
		if( bForEditor )
		{
			Obj->RefreshEditableProperties();
		}
	}

	if( !bForEditor )
	{
		ClearEditor();
	}

	assert( nullptr == m_Deserializer ); // Bad file?
	EG_SafeDelete( m_Deserializer );

	assert( m_DeserializerStack.IsEmpty() ); // Bad file?
	m_DeserializerStack.Clear();
}

void EGWorldFile::Save( EGFileData& MemFile )
{
	if( !m_GlobalsEd.IsValid() )
	{
		assert( m_GlobalsEd.GetData() == nullptr );
		InitGlobalsEditor();
	}

	if( !m_EditorConfigEd.IsValid() )
	{
		assert( m_EditorConfigEd.GetData() == nullptr );
		InitEditorConfigEditor();
	}

	MemFile.WriteStr8( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	MemFile.WriteStr8( "<egworld>\r\n" );

	MemFile.WriteStr8( "\t<globals>\r\n" );
	m_GlobalsEd.Serialize( eg_rfl_serialize_fmt::XML , 2 , MemFile );
	MemFile.WriteStr8( "\t</globals>\r\n" );

	MemFile.WriteStr8( "\t<editor_config>\r\n" );
	m_EditorConfigEd.Serialize( eg_rfl_serialize_fmt::XML , 2 , MemFile );
	MemFile.WriteStr8( "\t</editor_config>\r\n" );

	for( const EGWorldObject* Obj : m_WorldObjects )
	{
		Obj->Serialize( eg_rfl_serialize_fmt::XML , 1 , MemFile );
	}

	MemFile.WriteStr8( "</egworld>\r\n" );
}

void EGWorldFile::ClearEditor()
{
	m_GlobalsEd = CT_Clear;
	m_EditorConfigEd = CT_Clear;

	EGArray<EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( EGWorldObject* Obj : AllObjs )
	{
		Obj->ClearEditor();
	}
}

void EGWorldFile::ApplyToWorldPreLoad( EGEntWorld* World ) const
{
	World->SetGravity( m_Globals.GravityVector );

	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( const EGWorldObject* Obj : AllObjs )
	{
		Obj->ApplyToWorldPreload( World );
	}
}

void EGWorldFile::ApplyToWorldPostLoad(EGEntWorld* World) const
{
	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( const EGWorldObject* Obj : AllObjs )
	{
		Obj->ApplyToWorldPostLoad( World );
	}
}

eg_bool EGWorldFile::IsPreLoadComplete( const EGEntWorld* World ) const
{
	eg_bool bComplete = true;

	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( const EGWorldObject* Obj : AllObjs )
	{
		bComplete = bComplete && Obj->IsPreLoadComplete( World );
		if( !bComplete )
		{
			break;
		}
	}

	return bComplete;
}

void EGWorldFile::ApplyToWorldPreview( EGEntWorld* World ) const
{
	World->SetGravity( m_Globals.GravityVector );

	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( const EGWorldObject* Obj : AllObjs )
	{
		Obj->AddToWorldPreview( World );
	}
}

void EGWorldFile::ClearAllFromPreview( EGEntWorld* World ) const
{
	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );
	for( const EGWorldObject* Obj : AllObjs )
	{
		Obj->RemoveFromWorldPreview( World );
	}
}

void EGWorldFile::GetAllWorldObjects( EGArray<const EGWorldObject*>& Out ) const
{
	Out.Reserve( m_WorldObjects.Len() );
	for( const EGWorldObject* Obj : m_WorldObjects )
	{
		Obj->GetObjAndChildren( Out );
	}
}

void EGWorldFile::GetAllWorldObjects( EGArray<EGWorldObject*>& Out )
{
	Out.Reserve( m_WorldObjects.Len() );
	for( EGWorldObject* Obj : m_WorldObjects )
	{
		Obj->GetObjAndChildren( Out );
	}
}

EGWorldObject* EGWorldFile::InsertNewWorldObject( EGClass* ObjectClass )
{
	eg_d_string DesiredName = GenerateUniqueName( ObjectClass );

	EGWorldObject* NewObject = EGNewObject<EGWorldObject>( ObjectClass );
	if( NewObject )
	{
		NewObject->EditorInitNew( *DesiredName );
		m_WorldObjects.Append( NewObject );
	}

	return NewObject;
}

void EGWorldFile::DeleteWorldObject( EGWorldObject* WorldObject )
{
	if( m_WorldObjects.Contains( WorldObject ) )
	{
		m_WorldObjects.DeleteByItem( WorldObject );
	}

	WorldObject->EditorDeleteThis();
}

void EGWorldFile::MoveWorldObject( EGWorldObject* ObjToMove, EGWorldObject* ObjToMoveBefore, EGWorldObject* ObjParent )
{
	if( nullptr == ObjToMove )
	{
		return;
	}

	// Make sure the ObjToMoveBefore is not a child of ObjToMove
	for( EGWorldObject* MoveBeforeParent = ObjToMoveBefore; MoveBeforeParent != nullptr; MoveBeforeParent = MoveBeforeParent->GetParent() )
	{
		if( MoveBeforeParent == ObjToMove )
		{
			return;
		}
	}

	// Make sure the object to set as parent is not a child of the object
	for( EGWorldObject* ParentChild = ObjParent; ParentChild != nullptr; ParentChild = ParentChild->GetParent() )
	{
		if( ParentChild == ObjToMove )
		{
			return;
		}
	}

	if( ObjToMove->GetParent() )
	{
		EGWorldObject::UnbindParentAndChild( ObjToMove->GetParent() , ObjToMove );
	}
	else
	{
		m_WorldObjects.DeleteByItem( ObjToMove );
	}

	if( ObjParent )
	{
		EGWorldObject::BindParentAndChild( ObjParent , ObjToMove , nullptr );
	}
	else
	{
		if( ObjToMoveBefore )
		{
			if( ObjToMoveBefore->GetParent() )
			{
				EGWorldObject::BindParentAndChild( ObjToMoveBefore->GetParent() , ObjToMove , ObjToMoveBefore );
			}
			else
			{
				eg_size_t InsertIndex = m_WorldObjects.GetIndexOf( ObjToMoveBefore );
				assert( m_WorldObjects.IsValidIndex( InsertIndex ) );
				m_WorldObjects.InsertAt( InsertIndex , ObjToMove );
			}
		}
		else
		{
			m_WorldObjects.Append( ObjToMove );
		}
	}
}

void EGWorldFile::InitGlobalsEditor()
{
	assert( !m_GlobalsEd.IsValid() );
	m_GlobalsEd = EGReflection_GetEditor( m_Globals , "Globals" );
}

void EGWorldFile::InitEditorConfigEditor()
{
	assert( !m_EditorConfigEd.IsValid() );
	m_EditorConfigEd = EGReflection_GetEditor( m_EditorConfig , "EditorConfig" );
}

void EGWorldFile::Reset()
{
	for( EGWorldObject* Obj : m_WorldObjects )
	{
		EGDeleteObject( Obj );
	}
	m_WorldObjects.Clear();

	m_GlobalsEd = CT_Clear;
	m_Globals = egWorldFileGlobals();

	m_EditorConfigEd = CT_Clear;
	m_EditorConfig = egWorldFileEditorConfig();

	InitGlobalsEditor();
	InitEditorConfigEditor();
}

eg_d_string EGWorldFile::GenerateUniqueName( EGClass* ObjClass )
{
	eg_d_string DesiredName = EGWorldObject::GetShortNameFromClass( ObjClass );
	eg_int SuffixInt = 0;

	EGArray<const EGWorldObject*> AllObjs;
	GetAllWorldObjects( AllObjs );

	auto DoesNameExist = [&AllObjs]( eg_cpstr NameToTest ) -> eg_bool
	{
		eg_string_crc NameAsCrc = eg_string_crc(NameToTest);

		for( const EGWorldObject* Obj : AllObjs )
		{
			if( Obj->GetId() == NameAsCrc )
			{
				return true;
			}
		}

		return false;
	};

	if( DoesNameExist( *DesiredName ) )
	{
		for( eg_int i=0; i<100000; i++ )
		{
			eg_d_string NameWithSuffix = EGString_Format( "%s%d" , *DesiredName , i );
			if( !DoesNameExist( *NameWithSuffix ) )
			{
				DesiredName = NameWithSuffix;
				break;
			}
		}
	}

	return DesiredName;
}

void EGWorldFile::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	unused( AttGet );

	eg_string_crc TagAsCrc = eg_string_crc(Tag);
	switch_crc( TagAsCrc )
	{
		case_crc("egworld"):
		{
			
		} break;

		case_crc("globals"):
		{
			assert( m_Deserializer == nullptr );
			assert( m_GlobalsEd.GetData() == &m_Globals );
			m_Deserializer = new EGReflectionDeserializer;
			m_Deserializer->Init( m_GlobalsEd , GetXmlFilename() );
		} break;

		case_crc("editor_config"):
		{
			assert( m_Deserializer == nullptr );
			assert( m_EditorConfigEd.GetData() == &m_EditorConfig );
			m_Deserializer = new EGReflectionDeserializer;
			m_Deserializer->Init( m_EditorConfigEd , GetXmlFilename() );
		} break;

		case_crc("worldobject"):
		{
			eg_string ClassName = AttGet.GetString( "class" );
			EGClass* ObjectClass = EGClass::FindClassWithBackup( ClassName , EGWorldObject::GetStaticClass() );
			EGWorldObject* ParentObject = m_DeserializerStack.HasItems() ? m_DeserializerStack.Top() : nullptr;
			EGWorldObject* NewWorldObject = EGNewObject<EGWorldObject>( ObjectClass );
			if( ParentObject )
			{
				EGWorldObject::BindParentAndChild( ParentObject , NewWorldObject , nullptr );
				assert( m_Deserializer != nullptr );
			}
			else
			{
				m_WorldObjects.Append( NewWorldObject );
			}
			m_DeserializerStack.Push( NewWorldObject );

			if( nullptr == m_Deserializer )
			{
				m_Deserializer = new EGReflectionDeserializer;
			}
			m_Deserializer->Init( *NewWorldObject->GetEditor() , GetXmlFilename() );

		} break;

		/*
		case_crc("woldobjectproperties"):
		{
			assert( nullptr == m_PropertiesDeserializer && m_DeserializerStack.HasItems() );
			if( nullptr == m_PropertiesDeserializer && m_DeserializerStack.HasItems() )
			{
				egRflEditor* PropEd = m_DeserializerStack.Top()->GetPropertiesEditor();
				if( PropEd )
				{
					m_PropertiesDeserializer = new EGReflectionDeserializer;
					m_PropertiesDeserializer->Init( *PropEd , GetXmlFilename() );
				}
				else
				{
					EGLogf( eg_log_t::Warning , "Tried to deserialize properties to an object that didn't have them." );
				}
			}
		} break;
		*/

		default:
		{
			if( m_PropertiesDeserializer )
			{
				m_PropertiesDeserializer->OnXmlTagBegin( Tag , AttGet );
			}
			else if( m_Deserializer )
			{
				m_Deserializer->OnXmlTagBegin( Tag , AttGet );
			}
		} break;
	}
}

void EGWorldFile::OnTagEnd( const eg_string_base& Tag )
{
	eg_string_crc TagAsCrc = eg_string_crc(Tag);
	switch_crc( TagAsCrc )
	{
		case_crc("egworld"):
		{

		} break;

		case_crc("globals"):
		{
			assert( m_Deserializer != nullptr );
			m_Deserializer->Deinit();
			EG_SafeDelete( m_Deserializer );
		} break;

		case_crc("editor_config"):
		{
			assert( m_Deserializer != nullptr );
			m_Deserializer->Deinit();
			EG_SafeDelete( m_Deserializer );
		} break;

		case_crc("worldobject"):
		{
			assert( m_Deserializer != nullptr );
			assert( m_DeserializerStack.HasItems() );
			m_DeserializerStack.Pop();
			if( m_DeserializerStack.HasItems() )
			{
				m_Deserializer->Init( *m_DeserializerStack.Top()->GetEditor() , GetXmlFilename() );
			}
			else
			{
				m_Deserializer->Deinit();
				EG_SafeDelete( m_Deserializer );
			}
		} break;

		/*
		case_crc("woldobjectproperties"):
		{
			assert( m_PropertiesDeserializer != nullptr );
			if( m_PropertiesDeserializer )
			{
				delete m_PropertiesDeserializer;
				m_PropertiesDeserializer = nullptr;
			}
		} break;
		*/

		default:
		{
			if( m_PropertiesDeserializer )
			{
				m_PropertiesDeserializer->OnXmlTagEnd( Tag );
			}
			else if( m_Deserializer )
			{
				m_Deserializer->OnXmlTagEnd( Tag );
			}
		} break;
	}
}
