// (c) 2017 Beem Media

#include "EGEntObj.h"
#include "EGTimeline.h"
#include "EGCrcDb.h"

void EGEntObj::Init( eg_string_crc EntId )
{
	assert( nullptr == m_EntDef ); //Ent object was already created?
	m_EntDef = EntDict_GetDef( EntId );
	if( nullptr != m_EntDef )
	{
		m_BaseBounds = m_EntDef->m_BaseBounds;
		m_ComponentTree.Init( m_EntDef->m_ComponentTree , nullptr , false , true , m_EntDef->m_DefaultRenderFilter );
	}
	else
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": Tried to create an invalid entity (0x%08X) (%s).", EntId , EGCrcDb::CrcToString(EntId).String() );
	}
}

void EGEntObj::Init( const EGEntDef* Def )
{
	assert( nullptr == m_EntDef ); //Ent object was already created?
	m_EntDef = Def;
	if( nullptr != m_EntDef )
	{
		m_BaseBounds = m_EntDef->m_BaseBounds;
		m_ComponentTree.Init( m_EntDef->m_ComponentTree , nullptr , false , true , m_EntDef->m_DefaultRenderFilter );
	}
	else
	{
		EGLogf( eg_log_t::Error, __FUNCTION__ ": Tried to create an invalid entity with null definition." );
		// assert( false ); //No such entity?
	}
}

void EGEntObj::InitClone( const EGEntObj& rhs )
{
	Deinit();

	m_EntDef = rhs.m_EntDef;
	if( nullptr != m_EntDef )
	{
		m_BaseBounds = m_EntDef->m_BaseBounds;
		m_ComponentTree.Init( m_EntDef->m_ComponentTree , nullptr , false , true , m_EntDef->m_DefaultRenderFilter );
	}
}

void EGEntObj::Deinit()
{
	if( m_EntDef )
	{
		m_EntDef = nullptr;
	}

	m_ComponentTree.Deinit();
}

void EGEntObj::SetDrawInfo( const eg_transform& Pose , const eg_vec4& ScaleVec , eg_bool bIsLit )
{
	SetPose( Pose );
	SetScaleVec( ScaleVec );
	SetIsLit( bIsLit );
}

void EGEntObj::Update( eg_real DeltaTime )
{
	m_ComponentTree.ActiveUpdate( DeltaTime );
	m_ComponentTree.RelevantUpdate( DeltaTime );
}

void EGEntObj::RunEvent( eg_string_crc EventCrc )
{
	if( m_EntDef )
	{
		for( const EGTimeline* Timeline : m_EntDef->m_Timelines )
		{
			if( Timeline && Timeline->GetId() == EventCrc )
			{
				m_ComponentTree.RunTimeline( Timeline );
				break;
			}
		}
	}
}

void EGEntObj::StopAllEvents()
{
	m_ComponentTree.StopAllTimelines();
}

void EGEntObj::ResetEvents()
{
	m_ComponentTree.ResetAnimations();
}

void EGEntObj::Draw()
{
	if( nullptr == m_EntDef )return;

	if( m_bIsLit )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_LIT );
	}
	else
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	}

	m_ComponentTree.Draw( m_Pose );

	MainDisplayList->PopDefaultShader();
}

void EGEntObj::DrawForTool()
{
	if( nullptr == m_EntDef )return;

	MainDisplayList->SetWorldTF( eg_mat::BuildTransformNoScale( m_Pose ) );
	MainDisplayList->DrawAABB( m_BaseBounds , eg_color(eg_color32(180,0,0)) );

	if( m_bIsLit )
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_LIT );
	}
	else
	{
		MainDisplayList->PushDefaultShader( eg_defaultshader_t::MESH_UNLIT );
	}

	m_ComponentTree.DrawForTool( m_Pose );

	MainDisplayList->PopDefaultShader();
}

void EGEntObj::SetCustomBone( eg_string_crc NodeId , eg_string_crc BoneId , const eg_transform& Transform )
{
	m_ComponentTree.SetBone( NodeId , BoneId , Transform );
}

void EGEntObj::ClearCustomBones()
{
	m_ComponentTree.ClearCustomBones();
}

void EGEntObj::SetTexture( eg_string_crc NodeId , eg_string_crc GroupId , eg_cpstr TexturePath )
{
	m_ComponentTree.SetTexture( NodeId , GroupId , TexturePath );
}

void EGEntObj::SetMaterial( eg_string_crc NodeId, eg_string_crc GroupId, egv_material Material )
{
	m_ComponentTree.SetMaterial( NodeId , GroupId , Material );
}

void EGEntObj::SetText( eg_string_crc TextNodeCrcId , const eg_loc_text& NewText )
{
	m_ComponentTree.SetText( TextNodeCrcId , NewText );
}

void EGEntObj::SetMuteAudio( eg_bool bMute )
{
	m_ComponentTree.SetMuteAudio( bMute );
}

void EGEntObj::SetPalette( const eg_vec4& Palette )
{
	m_ComponentTree.SetPalette( Palette );
}

void EGEntObj::SetAnimationNormalTime( eg_string_crc NodeId , eg_string_crc SkeletonId , eg_string_crc AnimationId , eg_real t)
{
	m_ComponentTree.SetAnimationNormalTime( NodeId , SkeletonId , AnimationId , t );
}

void EGEntObj::QueryTextNodes( EGArray<eg_d_string>& Out ) const
{
	m_ComponentTree.QueryTextNodes( Out );
}

void EGEntObj::QueryBones( EGArray<eg_d_string>& Out ) const
{
	m_ComponentTree.QueryBones( Out );
}

void EGEntObj::QueryEvents( EGArray<eg_d_string>& Out ) const
{
	if( m_EntDef )
	{
		m_EntDef->QueryEventList( Out );
	}
}

void EGEntObj::RefreshFromDefForTool( const class EGComponent* CompDef )
{
	if( m_EntDef )
	{
		m_BaseBounds = m_EntDef->m_BaseBounds;
	}
	m_ComponentTree.RefreshFromDefForTool( CompDef );
}
