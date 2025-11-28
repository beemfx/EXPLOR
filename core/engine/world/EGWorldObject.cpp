// (c) 2018 Beem Media

#include "EGWorldObject.h"
#include "EGCrcDb.h"
#include "EGFileData.h"

EG_CLASS_DECL( EGWorldObject )

void EGWorldObject::OnDestruct()
{
	for( EGWorldObject* Child : m_Children )
	{
		EGDeleteObject( Child );
	}
	m_Children.Clear();
}

void EGWorldObject::RefreshInWorldPreview( EGEntWorld* World, const egRflEditor& ChangedProperty ) const
{
	unused( World, ChangedProperty );

	for( EGWorldObject* Child : m_Children )
	{
		if( Child )
		{
			Child->RefreshInWorldPreview( World , ChangedProperty );
		}
	}
}

void EGWorldObject::SetId( eg_cpstr IdIn )
{
	m_Id = EGCrcDb::StringToCrc( IdIn );
}

void EGWorldObject::SetWorldPose( const eg_transform& NewWorldPose )
{
	// Setting the world pose must account for all the parent poses, so we get
	// the parent's world pose and multiply the new pose by the inverse.
	const eg_transform CurrentPose = GetWorldPose();
	const eg_transform ParentPose = m_Pose.GetInverse() * CurrentPose;
	m_Pose = ParentPose.GetInverse() * NewWorldPose;
	m_Pose.NormalizeRotationOfThis();
}

eg_transform EGWorldObject::GetWorldPose() const
{
	eg_transform Out = m_Pose;

	for( const EGWorldObject* ParentChain = m_Parent; nullptr != ParentChain; ParentChain = ParentChain->m_Parent )
	{
		Out = Out * ParentChain->m_Pose;
	}

	return Out;
}

void EGWorldObject::EditorInitNew( eg_cpstr InId )
{
	m_Id = EGCrcDb::StringToCrc( InId );
	GetEditor(); // Force creation of editor.
	RefreshEditableProperties();
}

void EGWorldObject::RefreshEditableProperties()
{
	egRflEditor* IsLockedInToolEd = m_Editor.GetChildPtr( "m_bIsLockedInTool" );
	if( IsLockedInToolEd )
	{
		IsLockedInToolEd->SetEditable( false );
	}
}

void EGWorldObject::EditorDeleteThis()
{
	if( m_Parent )
	{
		// Ass the children of this to the parent of this.
		for( EGWorldObject* Child : m_Children )
		{
			Child->m_Parent = m_Parent;
			m_Parent->m_Children.Append( Child );
		}
		m_Children.Clear();
		m_Parent->m_Children.DeleteByItem( this );
	}

	EGDeleteObject( this );
}

egRflEditor* EGWorldObject::GetEditor()
{
	if( !m_Editor.IsValid() )
	{
		m_Editor = EGReflection_GetEditorForClass( GetObjectClass()->GetName() , this , "WorldObject" );
	}

	assert( m_Editor.GetData() == this );
	return &m_Editor;
}

const egRflEditor* EGWorldObject::GetEditor() const
{
	return const_cast<EGWorldObject*>(this)->GetEditor();
}

eg_int EGWorldObject::GetDepth() const
{
	eg_int Depth = 0;
	for( EGWorldObject* Parent = m_Parent; Parent != nullptr; Parent = Parent->m_Parent )
	{
		Depth++;
	}
	return Depth;
}

eg_d_string EGWorldObject::GetToolDesc() const
{
	return EGString_Format( "%s (%s)" , EGCrcDb::CrcToString( m_Id ).String() , *GetShortNameFromClass( GetObjectClass() ) ).String();
}

void EGWorldObject::GetObjAndChildren( EGArray<EGWorldObject*>& Out )
{
	Out.Append( this );
	for( EGWorldObject* Child : m_Children )
	{
		Child->GetObjAndChildren( Out );
	}
}

void EGWorldObject::GetObjAndChildren( EGArray<const EGWorldObject*>& Out ) const
{
	Out.Append( this );
	for( const EGWorldObject* Child : m_Children )
	{
		Child->GetObjAndChildren( Out );
	}
}

void EGWorldObject::Serialize( eg_rfl_serialize_fmt Format , eg_int Depth , EGFileData& MemFile ) const
{
	assert( Format == eg_rfl_serialize_fmt::XML );

	eg_string_small TabStr( CT_Clear );
	for( eg_int i=0; i<Depth; i++ )
	{
		TabStr.Append( '\t' );
	}

	const egRflEditor* Ed = GetEditor();
	MemFile.WriteStr8( EGString_Format( "%s<worldobject class=\"%s\">\r\n" , TabStr.String() , GetObjectClass()->GetName() ) );
	Ed->Serialize( Format , Depth+1 , MemFile );
	for( const EGWorldObject* Child : m_Children )
	{
		Child->Serialize( Format , Depth+1 , MemFile );
	}
	const egRflEditor* PropEd = GetPropertiesEditor();
	if( PropEd && PropEd->IsValid() )
	{
		// MemFile.WriteStr8( EGString_Format( "%s\t<woldobjectproperties>\r\n" , TabStr.String() ) );
		// PropEd->Serialize( Format , Depth+2 , MemFile );
		// MemFile.WriteStr8( EGString_Format( "%s\t</woldobjectproperties>\r\n" , TabStr.String() ) );
	}
	MemFile.WriteStr8( EGString_Format( "%s</worldobject>\r\n" , TabStr.String() ) );
}

void EGWorldObject::BindParentAndChild( EGWorldObject* Parent, EGWorldObject* Child , EGWorldObject* BindBefore )
{
	assert( Child->m_Parent == nullptr );
	Child->m_Parent = Parent;
	assert( !Parent->m_Children.Contains( Child ) );
	if( BindBefore )
	{
		assert( Parent->m_Children.Contains( BindBefore ) );
		eg_size_t InsertIndex = Parent->m_Children.GetIndexOf( BindBefore );
		Parent->m_Children.InsertAt( InsertIndex , Child );
	}
	else
	{
		Parent->m_Children.Append( Child );
	}
}

void EGWorldObject::UnbindParentAndChild( EGWorldObject* Parent, EGWorldObject* Child )
{
	assert( Parent && Child );
	assert( Parent->m_Children.Contains( Child ) && Child->GetParent() == Parent );

	Child->m_Parent = nullptr;
	Parent->m_Children.DeleteByItem( Child );
}

eg_d_string EGWorldObject::GetShortNameFromClass( EGClass* Class )
{
	if( Class == &EGWorldObject::GetStaticClass() )
	{
		return "WorldObject";
	}
	eg_string FullName = Class ? Class->GetName() : "";
	eg_string ShortName = FullName.Len() >= (2+11) ? &FullName.String()[2+11] : "WorldObject-Unknown-";
	return ShortName.String();
}