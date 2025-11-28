// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMonsterSpriteComponent.h"
#include "EGRenderer.h"
#include "EGDebugText.h"

EG_CLASS_DECL( ExMonsterSpriteComponent )

void ExMonsterSpriteComponent::SetAnimation( eg_int FirstFrame , eg_int LastFrame , eg_real Duration , ex_anim_t AnimType )
{
	m_FirstFrame = EG_Clamp<eg_int>( FirstFrame , 0 , NUM_FRAMES-1 );
	m_LastFrame = EG_Clamp<eg_int>( LastFrame , 0 , NUM_FRAMES-1 );
	if( m_FirstFrame > m_LastFrame )
	{
		EG_Swap( m_FirstFrame , m_LastFrame );
	}
	m_Duration = Duration;
	m_AnimType = AnimType;
	m_NumFrames = m_LastFrame - m_FirstFrame + 1;
	if( m_AnimType == ex_anim_t::LoopForwardBack )
	{
		if( m_NumFrames > 1 )
		{
			m_NumFrames = (m_NumFrames-1)*2;
		}
	}
	assert( m_NumFrames > 0 );
	m_FrameTime = m_Duration / m_NumFrames;
	m_TimeSinceLastFrame = 0.f;

	m_BaseFrame = 0;
	UpdateFrame( 0.f );
}

void ExMonsterSpriteComponent::OnConstruct()
{
	Super::OnConstruct();

	SetAnimation( 0 , NUM_FRAMES-1 , 20.f , ex_anim_t::LoopForward );
}

void ExMonsterSpriteComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	UpdateFrame( DeltaTime );
	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Animation Frame: {0} Base: {1}/{2}" , m_CurrentFrame , m_BaseFrame , m_NumFrames ) );
}

void ExMonsterSpriteComponent::Draw( const eg_transform& ParentPose ) const
{
	MainDisplayList->SetIVec4( eg_r_ivec4_t::EntInts0 , eg_ivec4(m_CurrentFrame,0,0,0) );
	Super::Draw( ParentPose );
}

void ExMonsterSpriteComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	auto AnimTypeParm = [&Action]( eg_size_t Index ) -> ex_anim_t
	{
		static const struct
		{
			eg_cpstr AnimName;
			ex_anim_t AnimType;
		}
		ConvTable[] =
		{
			{ "LoopForward" , ex_anim_t::LoopForward },
			{ "LoopBack" , ex_anim_t::LoopBack },
			{ "LoopForwardBack" , ex_anim_t::LoopForwardBack },
			{ "OnceForward" , ex_anim_t::OnceForward },
			{ "OnceBack" , ex_anim_t::OnceBack },
		};

		for( eg_size_t i=0; i<countof(ConvTable); i++ )
		{
			if( EGString_EqualsI( ConvTable[i].AnimName , Action.FnCall.Parms[Index] ) )
			{
				return ConvTable[i].AnimType;
			}
		}
		
		// assert( false ); // Animation type not found...
		EGLogf( eg_log_t::Warning , "ExMonsterSpriteComponent::ScriptExec tried to set an invalid animation type: %s" , Action.FnCall.Parms[Index] );
		return ex_anim_t::LoopForward;
	};

	switch_crc( ActionAsCrc )
	{
		case_crc("SetAnimation"):
		{
			if( Action.FnCall.NumParms >= 4 )
			{
				SetAnimation( Action.IntParm(0) , Action.IntParm(1) , Action.RealParm(2) , AnimTypeParm(3) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetAnimation." );
			}
		} break;

		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

void ExMonsterSpriteComponent::UpdateFrame( eg_real DeltaTime )
{
	m_TimeSinceLastFrame += DeltaTime;

	if( m_TimeSinceLastFrame > m_FrameTime )
	{
		m_TimeSinceLastFrame = 0.f;

		switch( m_AnimType )
		{
			case ex_anim_t::LoopForward:
			case ex_anim_t::LoopBack:
			case ex_anim_t::LoopForwardBack:
				m_BaseFrame = (m_BaseFrame + 1) % m_NumFrames;
				break;
			case ex_anim_t::OnceForward:
			case ex_anim_t::OnceBack:
				m_BaseFrame = EG_Clamp<eg_int>( m_BaseFrame + 1 , 0 , m_NumFrames-1 );
				break;
		}
	}

	switch( m_AnimType )
	{
		case ex_anim_t::LoopForward:
		case ex_anim_t::OnceForward:
			m_CurrentFrame = m_FirstFrame + m_BaseFrame;
			break;
		case ex_anim_t::LoopBack:
		case ex_anim_t::OnceBack:
			m_CurrentFrame = m_LastFrame - m_BaseFrame;
			break;
		case ex_anim_t::LoopForwardBack:
		{
			if( m_NumFrames > 1 )
			{
				if( m_BaseFrame <= (m_NumFrames/2) )
				{
					m_CurrentFrame = m_FirstFrame + m_BaseFrame;
				}
				else
				{
					m_CurrentFrame = m_LastFrame - (m_BaseFrame - (m_NumFrames/2));
				}
			}
			else
			{
				m_CurrentFrame = m_FirstFrame + m_BaseFrame;
			}
		} break;
	}

	assert( 0 <= m_CurrentFrame && m_CurrentFrame < NUM_FRAMES );
}
