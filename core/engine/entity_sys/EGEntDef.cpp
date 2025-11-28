/******************************************************************************
File: EntT.cpp
Class: EGEntDef
Purpose: See header.

(c) 2011 Beem Software
******************************************************************************/

#include "EGEnt.h"
#include "EGPhysBody2.h"
#include "EGParse.h"
#include "EGCrcDb.h"
#include "EGTimeline.h"
#include "EGFileData.h"
#include "EGPhysBodyComponent.h"
#include "EGReflectionDeserializer.h"
#include "EGEntComponentTree.h"
#include "EGComponent.h"

eg_cpstr ENT_RENDER_FILTER_WORLD = "World";
eg_cpstr ENT_RENDER_FILTER_PRE_FX = "PreFxPass";
eg_cpstr ENT_RENDER_FILTER_POST_FX = "PostFxPass";

EGEntDef::EGEntDef( eg_ctor_t Ct )
: m_NextNodeId( 0 )
, m_CurAnim( Ct )
, m_NextTransformIsBase64( false )
, m_NextTransformType( Ct )
{
	if( Ct == CT_Default )
	{
		m_DefaultRenderFilter = EGCrcDb::StringToCrc(ENT_RENDER_FILTER_PRE_FX);
	}
}

EGEntDef::EGEntDef( eg_cpstr Filename ) : m_NextNodeId( 0 )
, m_CurAnim( eg_crc( "" ) )
, m_NextTransformIsBase64( false )
, m_NextTransformType( CT_Clear )
{
	LoadForDict( Filename );
}

void EGEntDef::LoadForDict( eg_cpstr Filename )
{
	Unload();
	m_Editor = EGReflection_GetEditor( *this , "EntDef" );

	m_DefaultRenderFilter = EGCrcDb::StringToCrc(ENT_RENDER_FILTER_PRE_FX);

	if( nullptr != Filename && Filename[0] != 0 )
	{
		m_BaseBounds = eg_aabb( CT_Default );
		XMLLoad(Filename);
	}

	// We don't need editors at this point since this is for the dictionary
	FinalizeDictDef();

	PostLoad();
}

void EGEntDef::LoadForEditor( const class EGFileData& File , eg_cpstr RefName )
{
	Unload();
	m_Editor = EGReflection_GetEditor( *this , "EntDef" );
	
	m_DefaultRenderFilter = EGCrcDb::StringToCrc(ENT_RENDER_FILTER_PRE_FX);
	if( nullptr != RefName && File.GetSize() > 0 && RefName[0] != '\0' )
	{
		m_BaseBounds = eg_aabb( CT_Default );
		XMLLoad( File.GetData() , File.GetSize() , RefName );
	}

	RefreshVisibleProperties( m_Editor );

	PostLoad();
}

void EGEntDef::Unload( void )
{
	m_Editor = CT_Clear;

	m_EntityClassName.Class = nullptr;
	m_UiClassName.Class = nullptr;

	while( m_ComponentTree.HasItems() )
	{
		EGComponent* CompDef = m_ComponentTree.GetOne();
		m_ComponentTree.Remove( CompDef );
		EG_SafeRelease( CompDef );
	}

	for( EGTimeline* Timeline : m_Timelines )
	{
		EG_SafeRelease( Timeline );
	}
	m_Timelines.Clear();
}

void EGEntDef::SerializeToXml( class EGFileData& File ) const
{
	auto Write = [&File]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		File.WriteStr8( Buffer );
	};

	Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	
	Write( "<edef>\r\n" );

	Write( "\r\n" );

	File.WriteStr8( "\t<properties>\r\n" );
	m_Editor.Serialize( eg_rfl_serialize_fmt::XML , 2 , File );
	File.WriteStr8( "\t</properties>\r\n" );

	Write( "\r\n" );

	for( const EGComponent* CompDef : m_ComponentTree )
	{
		CompDef->SerializeToXml( File , "\t" );
		Write( "\r\n" );
	}

	for( const EGTimeline* Timeline : m_Timelines )
	{
		Timeline->SerializeToXml( File , "\t"  );
		Write( "\r\n" );
	}
	
	Write( "</edef>\r\n" );
}

void EGEntDef::QueryEventList( EGArray<eg_d_string>& Out ) const
{
	for( const EGTimeline* Timeline : m_Timelines )
	{
		if( Timeline )
		{
			Out.Append( EGCrcDb::CrcToString(Timeline->GetId()).String() );
		}
	}
}

void EGEntDef::GetComponents(EGArray<EGComponent*>& DefsOut)
{
	assert( DefsOut.Len() == 0 );

	for( EGComponent* RootDef : m_ComponentTree )
	{
		GetComponentsRecursive( RootDef , DefsOut );
	}
}

void EGEntDef::CreateAssets() const
{
	if( m_AssetHolderCount == 0 )
	{
		assert( nullptr == m_AssetHolder );
		static_assert( sizeof(m_AssetHolderMem) >= sizeof(EGEntComponentTree) , "Need more mem for asset holder" );
		m_AssetHolder = new ( m_AssetHolderMem ) EGEntComponentTree( CT_Clear );
		m_AssetHolder->Init( m_ComponentTree , false , false , true , m_DefaultRenderFilter );
	}
	m_AssetHolderCount++;
}

void EGEntDef::DestroyAssets() const
{
	assert( m_AssetHolderCount > 0 );
	m_AssetHolderCount--;

	if( m_AssetHolderCount == 0 )
	{
		assert( m_AssetHolder );
		m_AssetHolder->Deinit();
		m_AssetHolder->~EGEntComponentTree();
		m_AssetHolder = nullptr;
	}
}

void EGEntDef::PostLoad()
{
	assert( nullptr == m_ConfigDeserializer );
	EG_SafeDelete( m_ConfigDeserializer );
}

void EGEntDef::FinalizeDictDef()
{
	for( EGComponent* Comp : m_ComponentTree )
	{
		EGArray<EGComponent*> CompStack;
		CompStack.Push( Comp );
		while( CompStack.HasItems() )
		{
			EGComponent* CurItem = CompStack.Top();
			CompStack.Pop();

			CurItem->FinalizeDictDef();
			for( EGComponent* Child : CurItem->GetChildren() )
			{
				CompStack.Push( Child );
			}
		}
	}

	m_Editor = CT_Clear;
}

void EGEntDef::GetComponentsRecursive( EGComponent* Parent, EGArray<EGComponent*>& DefsOut )
{
	if( Parent )
	{
		DefsOut.Append( Parent );
		for( EGComponent* ChildDef : Parent->GetChildren() )
		{
			GetComponentsRecursive( ChildDef , DefsOut );
		}
	}
}

void EGEntDef::RefreshVisibleProperties( egRflEditor& ThisEditor )
{
	assert( ThisEditor.GetData() == this );

	auto SetPropEditable = [this, &ThisEditor]( eg_cpstr Name, eg_bool bValue ) -> void
	{
		egRflEditor* VarEd = ThisEditor.GetChildPtr( Name );
		if( VarEd )
		{
			VarEd->SetEditable( bValue );
		}
	};

	SetPropEditable( "m_EditorConfig" , false );
	SetPropEditable( "m_EntityClassName" , m_ClassType == eg_ent_class_t::Game || m_ClassType == eg_ent_class_t::GameAndUI );
	SetPropEditable( "m_UiClassName" , m_ClassType == eg_ent_class_t::UI || m_ClassType == eg_ent_class_t::GameAndUI );

	egRflEditor* EditorConfigEd = ThisEditor.GetChildPtr( "m_EditorConfig" );
	if( EditorConfigEd )
	{
		m_EditorConfig.RefreshVisibleProperties( *EditorConfigEd );
	}
}

void EGEntDef::OnEditPropertyChanged( const egRflEditor* ChangedProperty , egEntDefEditNeeds& NeedsOut  )
{
	if( EGString_Equals( ChangedProperty->GetVarName() , "m_ClassType" ) )
	{
		NeedsOut.bRefreshEditor = true;
		RefreshVisibleProperties( m_Editor );
	}
	else if( EGString_Equals( ChangedProperty->GetVarName() , "Type" ) || EGString_Equals( ChangedProperty->GetVarName() , "Lights" ) )
	{
		NeedsOut.bRefreshEditor = true;
		RefreshVisibleProperties( m_Editor );
	}
}

eg_cpstr EGEntDef::XMLObjName()const
{
	return ("\"EGEntDef\"");
}

void EGEntDef::OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts)
{
	if( !Tag.EqualsI("component") && ( m_CurReadComponent ) )
	{
		if( m_CurReadComponent )
		{
			m_CurReadComponent->OnXmlTag( Tag , atts, GetXmlFilename() );
		}
		return;
	}

	if( !Tag.EqualsI("properties") && m_ConfigDeserializer )
	{
		m_ConfigDeserializer->OnXmlTagBegin( Tag , atts );
		return;
	}

	//Note that currently with all the physics tags should
	//be calling the isValid methods to make sure that the
	//object's created won't cause an exception.
	if( Tag == "edef" )
	{

	}
	else if( Tag == "properties" )
	{
		assert( m_ConfigDeserializer == nullptr );
		m_ConfigDeserializer = new EGReflectionDeserializer;
		if( m_ConfigDeserializer )
		{
			m_ConfigDeserializer->Init( m_Editor , GetXmlFilename() );
		}
	}
	else if( Tag == "timeline" ) OnTag_timeline(Tag,atts);
	else if( Tag == "keyframe" ) OnTag_keyframe(Tag,atts);
	else if( Tag == "component" ) OnTag_component(atts);
	else assert( false ); //Not a tag.
}

void EGEntDef::OnTagEnd( const eg_string_base& Tag )
{
	if( Tag == "timeline" || Tag == "keyframe" )
	{
		if( m_Timelines.HasItems() && m_Timelines.Top() )
		{
			m_Timelines.Top()->OnTagEnd( Tag );
		}
	}
	else if( Tag == "component" )
	{
		assert( m_CurReadComponent != nullptr );
		if( m_CurReadComponent )
		{
			m_CurReadComponent->EndXmlComponentTag();
			m_CurReadComponent = m_CurReadComponent->GetParent();
		}
	}
	else if( m_CurReadComponent )
	{
		m_CurReadComponent->OnXmlTagEnd( Tag );
	}
	else if( Tag == "properties" )
	{
		assert( m_ConfigDeserializer );
		EG_SafeDelete( m_ConfigDeserializer );
	}
	else if( m_ConfigDeserializer )
	{
		m_ConfigDeserializer->OnXmlTagEnd( Tag );
	}
}

void EGEntDef::OnData(eg_cpstr strData, eg_uint nLen)
{
	const eg_string_base& Tag = GetXmlTagUp(0);

	if("keyframe" == Tag )
	{
		if( m_Timelines.HasItems() && m_Timelines.Top() )
		{
			m_Timelines.Top()->OnData( strData , nLen );
		}
	}
}

void EGEntDef::OnTag_component( const EGXmlAttrGetter& Getter )
{
	EGClass* Class = EGClass::FindClassWithBackup( Getter.GetString("class") , EGComponent::GetStaticClass() );
	if( Class )
	{
		EGComponent* NewComp = EGNewObject<EGComponent>( Class , eg_mem_pool::System );
		if( NewComp )
		{
			NewComp->InitEditor();
			if( nullptr == m_CurReadComponent )
			{
				m_ComponentTree.InsertLast( NewComp );
			}
			NewComp->BeginXmlComponentTag( m_CurReadComponent , Getter , GetXmlFilename() );

			m_CurReadComponent = NewComp;
		}
		else
		{
			assert( false ); // Invalid component
		}
	}
}

void EGEntDef::OnTag_timeline( const eg_string_base& Tag , const EGXmlAttrGetter& Getter )
{
	unused( Tag );

	EGTimeline* NewTimeline = EGNewObject<EGTimeline>( eg_mem_pool::System );
	if( NewTimeline )
	{
		m_Timelines.Append( NewTimeline );
		NewTimeline->OnTag( Tag , Getter );
	}
}

void EGEntDef::OnTag_keyframe( const eg_string_base& Tag , const EGXmlAttrGetter& Getter )
{
	if( GetXmlTagUp(1) == "timeline" && !m_Timelines.IsEmpty() && m_Timelines.Top() )
	{
		m_Timelines.Top()->OnTag( Tag , Getter );
	}
}
