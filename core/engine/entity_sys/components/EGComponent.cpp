// (c) 2018 Beem Media

#include "EGComponent.h"
#include "EGXMLBase.h"
#include "EGReflectionDeserializer.h"
#include "EGFileData.h"
#include "EGCrcDb.h"
#include "EGTimelineTypes.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGComponent )

egComponentStringQuery EGComponent::StaticQueryForSubMeshes = nullptr;

void EGComponent::OnConstruct()
{
	Super::OnConstruct();
}

void EGComponent::OnDestruct()
{
	assert( nullptr == m_Deserializer ); // This entity definition was bad.
	EG_SafeDelete( m_Deserializer );
	m_Editor = CT_Clear;

	if( m_bIsDefinition )
	{
		// A definition is responsible for it's children.
		while( m_Children.HasItems() )
		{
			EGComponent* ChildDef = m_Children.GetOne();
			m_Children.Remove( ChildDef );
			EG_SafeRelease( ChildDef );
		}
	}

	Super::OnDestruct();
}


void EGComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	assert( !m_bIsDefinition );

	m_InitData = InitData;
	m_Parent = InitData.Parent;

	EGComponent::RefreshFromDef(); // Only refresh from Def for the base component.
		
	if( m_Parent )
	{
		m_Parent->m_Children.InsertLast( this );
	}
}

void EGComponent::ActiveUpdate( eg_real DeltaTime )
{
	unused( DeltaTime );

	assert( !m_bIsDefinition );
}

void EGComponent::RelevantUpdate( eg_real DeltaTime )
{
	unused( DeltaTime );

	assert( !m_bIsDefinition );

	m_Pose.Update( DeltaTime );
	m_CachedScale.Update( DeltaTime );
}

void EGComponent::Draw( const eg_transform& ParentPose ) const
{
	unused( ParentPose );

	assert( !m_bIsDefinition );
}

void EGComponent::DrawForTool( const eg_transform& ParentPose ) const
{
	assert( !m_bIsDefinition );

	Draw( ParentPose );
}

void EGComponent::AddToSceneGraph( const eg_transform& ParentPose, EGWorldSceneGraph* SceneGraph ) const
{
	unused( ParentPose , SceneGraph );
}

void EGComponent::ScriptExec( const struct egTimelineAction& Action )
{
	assert( !m_bIsDefinition );

	EGLogf( eg_log_t::Error , "EGComponent: Invalid script call \"%s\" in \"%s\"." , Action.FnCall.FunctionName , *GetName() );
}

eg_transform EGComponent::GetPose( eg_string_crc JointName ) const
{
	assert( !m_bIsDefinition );

	eg_transform AdjustedPose = m_Pose.GetCurrentValue();
	AdjustedPose.ScaleTranslationOfThisNonUniformly( m_GlobalScale.ToVec3() );
	return JointName.IsNull() ? AdjustedPose : GetJointPose( JointName ) * AdjustedPose;
}

const eg_transform EGComponent::ComputeLocalPose() const
{
	assert( !m_bIsDefinition );

	eg_transform Out = CT_Default;

	eg_string_crc ParentJoint = m_ParentJointId;
	for( const EGComponent* ParentChain = GetParent(); nullptr != ParentChain; ParentChain = ParentChain->GetParent() )
	{
		Out = Out * ParentChain->GetPose( ParentJoint );
		ParentJoint = ParentChain->m_ParentJointId;
	}

	return Out;
}

const eg_transform EGComponent::GetWorldPose( const eg_transform& OwnerPose ) const
{
	return ComputeLocalPose() * GetPose( CT_Clear ) * OwnerPose;
}

void EGComponent::SetMuteAudio( eg_bool bMute )
{
	assert( !m_bIsDefinition );

	if( bMute )
	{
		m_MuteCount++;
	}
	else
	{
		m_MuteCount--;
	}
}

void EGComponent::SetScaleVec( const eg_vec4& ScaleVec )
{
	assert( !m_bIsDefinition );

	m_GlobalScale = ScaleVec;
}

void EGComponent::RefreshFromDef()
{
	assert( !m_bIsDefinition );
	if( m_InitData.Def )
	{
		assert( m_InitData.Def->m_bIsDefinition );
		m_CachedId       = EGCrcDb::StringToCrc(*m_InitData.Def->m_Name);
		m_ParentJointId  = m_InitData.Def->m_ParentJointId;
		m_ScaleVector    = m_InitData.Def->m_ScaleVector;
		m_BasePose       = m_InitData.Def->m_BasePose;
		m_Pose.SetValue( m_InitData.Def->m_BasePose );
		m_CachedScale.SetValue( eg_vec4(m_InitData.Def->m_ScaleVector,1.f) );
	}
}

void EGComponent::Reset()
{
	assert( !m_bIsDefinition );
	RefreshFromDef();
}


void EGComponent::InitNewInEditor( eg_cpstr ComponentName )
{
	m_Name = ComponentName;
	InitEditor();
}

const eg_d_string EGComponent::GetEntOwnerDefId() const
{
	const EGEnt* OwnerAsEnt = GetOwner<EGEnt>();
	if( OwnerAsEnt )
	{
		return OwnerAsEnt->GetDefId();
	}

	return "-no ent owner-";
}

void EGComponent::InitEditor()
{
	m_bIsDefinition = true;
	if( m_Editor.GetData() != this )
	{
		m_Editor = EGReflection_GetEditorForClass( GetObjectClass()->GetName() , this , "ComponentDef" );
	}

	RefreshEditableProperties();
}

void EGComponent::ClearEditor()
{
	m_Editor = CT_Clear;
}

void EGComponent::FinalizeDictDef()
{
	assert( m_bIsDefinition );
	assert( nullptr == m_Deserializer );
	EG_SafeDelete( m_Deserializer );
	ClearEditor();
}

void EGComponent::RefreshEditableProperties()
{
	assert( m_bIsDefinition );

	egRflEditor* VarEd = nullptr;
	
	// Locked in tool is not editable.
	VarEd = m_Editor.GetChildPtr( "m_bIsLockedInTool" );
	if( VarEd )
	{
		VarEd->SetEditable( false );
	}

	// Name is not serialized.
	VarEd = m_Editor.GetChildPtr( "m_Name" );
	if( VarEd )
	{
		VarEd->SetSerialized( false );
	}
}

void EGComponent::BeginXmlComponentTag( EGComponent* Parent , const EGXmlAttrGetter& AttGet , eg_cpstr XmlFilename )
{
	assert( m_bIsDefinition );

	m_OwnerFilename = XmlFilename;

	m_Name = AttGet.GetString("id").String();
	if( !EGString_Equals( GetObjectClass()->GetName() , AttGet.GetString("class") ) )
	{
		EGLogf( eg_log_t::Error , "\"%s\" had an invalid component (%s)." , XmlFilename , AttGet.GetString("class").String() );
	}
	m_Parent = Parent;
	if( m_Parent )
	{
		m_Parent->m_Children.InsertLast( this );
	}
}

void EGComponent::OnXmlTag( const eg_string_base& Tag , const EGXmlAttrGetter& AttGet , eg_cpstr XmlFilename )
{
	assert( m_bIsDefinition );

	if( Tag == "properties" )
	{
		m_Editor = egRflEditor();
		InitEditor();
		assert( nullptr == m_Deserializer );
		m_Deserializer = new EGReflectionDeserializer;
		if( m_Deserializer )
		{
			m_Deserializer->Init( m_Editor , XmlFilename );
		}
	}
	else if( m_Deserializer )
	{
		m_Deserializer->OnXmlTagBegin( Tag , AttGet );
	}
}

void EGComponent::OnXmlTagEnd( const eg_string_base& Tag )
{
	assert( m_bIsDefinition );

	if( Tag == "properties" )
	{
		assert( m_Deserializer );
		if( m_Deserializer )
		{
			m_Deserializer->Deinit();
			delete m_Deserializer;
			m_Deserializer = nullptr;
		}
	}
	else if( m_Deserializer )
	{
		m_Deserializer->OnXmlTagEnd( Tag );
	}
}

void EGComponent::EndXmlComponentTag()
{
	assert( m_bIsDefinition );

	PostXmlLoad();
}

eg_string_big EGComponent::GetToolDesc() const
{
	assert( m_bIsDefinition );

	return EGString_Format( "%s%s (%s)" 
		, m_bIsLockedInTool ? "(L) " : ""
		, *m_Name
		, GetShortNameFromClass( GetObjectClass() ).String() );
}

eg_uint EGComponent::GetDepth() const
{
	eg_uint Depth = 0;
	for( EGComponent* Level = m_Parent; nullptr != Level; Level = Level->m_Parent )
	{
		Depth++;
	}
	return Depth;
}

void EGComponent::SerializeToXml( class EGFileData& File , eg_cpstr PrefixTabs ) const
{
	assert( m_bIsDefinition );

	auto Write = [&File]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		File.WriteStr8( Buffer );
	};

	auto GetComponentAttributes = [this]() -> eg_string_big
	{
		eg_string_big CompAttrs;

		auto AddCompAttr = [&CompAttrs]( eg_cpstr Tag , eg_cpstr Value ) -> void
		{
			CompAttrs.Append( EGString_Format( " %s=\"%s\"" , Tag , EGString_ToXmlFriendly( Value ).String() ) );
		};

		AddCompAttr( "id" , *m_Name );
		AddCompAttr( "class" , GetObjectClass()->GetName() );

		return CompAttrs;
	};

	Write( "%s<component%s>\r\n" , PrefixTabs , GetComponentAttributes().String() );


	File.WriteStr8( EGString_Format( "%s\t<properties>\r\n" , PrefixTabs ) );
	m_Editor.Serialize( eg_rfl_serialize_fmt::XML , static_cast<eg_uint>(EGString_StrLen(PrefixTabs)) + 2 , File );
	File.WriteStr8( EGString_Format( "%s\t</properties>\r\n" , PrefixTabs ) );

	for( const EGComponent* Child : m_Children )
	{
		Child->SerializeToXml( File , EGString_Format( "%s\t" , PrefixTabs ) );
	}

	Write( "%s</component>\r\n" , PrefixTabs );
}

eg_string EGComponent::GetShortNameFromClass(EGClass* Class)
{
	if( Class == &EGComponent::GetStaticClass() )
	{
		return "Component";
	}
	eg_string FullName = Class ? Class->GetName() : "";
	eg_string ShortName = FullName.Len() >= (2+9) ? &FullName.String()[2] : "-UnknownComponent-Component";
	ShortName.ClampEnd( 9 );
	return ShortName;
}

eg_ent_role EGComponent::GetEntRole() const
{
	EGEnt* OwnerAsEnt = EGCast<EGEnt>(m_InitData.Owner);
	return OwnerAsEnt ? OwnerAsEnt->GetRole() : eg_ent_role::Unknown;
}

eg_ent_world_role EGComponent::GetWorldRole() const
{
	EGEnt* OwnerAsEnt = EGCast<EGEnt>(m_InitData.Owner);
	return OwnerAsEnt ? OwnerAsEnt->GetWorldRole() : eg_ent_world_role::Unknown;
}

eg_bool EGComponent::IsServer() const
{
	return m_InitData.bIsServer;
}

eg_bool EGComponent::IsClient() const
{
	return m_InitData.bIsClient;
}

void EGComponent::SetPropEditable( eg_cpstr Name , eg_bool bEditable )
{
	assert( m_bIsDefinition );
	egRflEditor* VarEd = m_Editor.GetChildPtr( Name );
	if( VarEd )
	{
		VarEd->SetEditable( bEditable );
	}
}
