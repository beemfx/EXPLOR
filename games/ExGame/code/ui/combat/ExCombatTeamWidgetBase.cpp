// (c) 2017 Beem Media

#include "ExCombatTeamWidgetBase.h"
#include "ExCombat.h"
#include "EGAudioList.h"
#include "EGTextNodeComponent.h"
#include "ExUiSounds.h"

void ExCombatTeamWidgetBase::InitCombatTeamWidgetBase()
{
	static const struct exAnimSetupInfo
	{ 
		eg_string_crc AnimObj;
		eg_string_crc StartAnim;
		eg_real       AnimDuration;
	}
	AnimInfos[] =
	{
		{ eg_crc("WeaponSlash")    , eg_crc("Play")    , .5f },
		{ eg_crc("DeathExplosion") , eg_crc("Explode") , .5f },
		{ eg_crc("SpellBonus")     , eg_crc("Play")    ,  1.f },
		{ eg_crc("SpellExplosion") , eg_crc("Play")    ,  1.f },
		{ eg_crc("ENT_UI_DamageNumber") , eg_crc("PlayDamage") , 2.f }, // Damage Number
		{ eg_crc("ENT_UI_DamageNumber") , eg_crc("PlayDamage") , 2.f }, // Burgled
		{ eg_crc("ENT_UI_DamageNumber") , eg_crc("PlayDamage") , 2.f }, // Resisted
		{ CT_Clear , CT_Clear , .5f }, // SpellFail
		{ eg_crc("ENT_UI_DamageNumber") , eg_crc("PlayDamage") , 2.f }, // Critical
	};

	m_AnimTemplates.Resize( enum_count(ex_combat_anim) );

	static_assert( countof(AnimInfos) == enum_count(ex_combat_anim) , "Missing Info" );

	for( eg_size_t i=0; i<m_AnimTemplates.Len(); i++ )
	{
		if( AnimInfos[i].AnimObj.IsNotNull() )
		{
			m_AnimTemplates[i].EntObj.Init( AnimInfos[i].AnimObj );
		}
		m_AnimTemplates[i].StartAnim = AnimInfos[i].StartAnim;
		m_AnimTemplates[i].AnimDuration = AnimInfos[i].AnimDuration;
	}
}

void ExCombatTeamWidgetBase::DeintCombatTeamWidgetBase()
{
	for( exAnimInfo& AnimInfo : m_AnimTemplates )
	{
		AnimInfo.EntObj.Deinit();
	}

	for( exPlayingAnimInfo* PlayingInfo : m_PlayingAnims )
	{
		PlayingInfo->EntObj.Deinit();
		delete PlayingInfo;
	}
	m_PlayingAnims.Clear();
}

void ExCombatTeamWidgetBase::PlayCombatTeamWidgetBaseAnimation( ex_combat_anim Anim , const eg_transform& Pose , eg_real Scale , const eg_color32& PaletteColor )
{
	if( Anim == ex_combat_anim::SpellFail )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
		return;
	}

	eg_size_t AnimIndex = static_cast<eg_size_t>(Anim);
	if( m_AnimTemplates.IsValidIndex( AnimIndex ) )
	{
		exPlayingAnimInfo* NewPlayingAnim = new exPlayingAnimInfo;
		if( NewPlayingAnim )
		{
			m_PlayingAnims.Append( NewPlayingAnim );
			NewPlayingAnim->EntObj.InitClone( m_AnimTemplates[AnimIndex].EntObj );
			NewPlayingAnim->AnimTimeLeft = m_AnimTemplates[AnimIndex].AnimDuration;
			NewPlayingAnim->Pose = Pose;
			NewPlayingAnim->ScaleVec = eg_vec4(Scale,Scale,Scale,1.f);
			NewPlayingAnim->EntObj.SetPalette( eg_color(PaletteColor).ToVec4() );
			NewPlayingAnim->EntObj.RunEvent( m_AnimTemplates[AnimIndex].StartAnim );
		}
	}
}

void ExCombatTeamWidgetBase::PlayDamageNumber( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale , eg_bool bIsHeal )
{
	eg_size_t AnimIndex = static_cast<eg_size_t>(ex_combat_anim::DamageNumber);
	if( m_AnimTemplates.IsValidIndex( AnimIndex ) )
	{
		exPlayingAnimInfo* NewPlayingAnim = new exPlayingAnimInfo;
		if( NewPlayingAnim )
		{
			m_PlayingAnims.Append( NewPlayingAnim );
			NewPlayingAnim->EntObj.InitClone( m_AnimTemplates[AnimIndex].EntObj );
			NewPlayingAnim->AnimTimeLeft = m_AnimTemplates[AnimIndex].AnimDuration;
			NewPlayingAnim->Pose = Pose;
			NewPlayingAnim->ScaleVec = eg_vec4(Scale,Scale,Scale,1.f);
			EGTextNodeComponent* DamageText = NewPlayingAnim->EntObj.GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("DamageNumber") );
			if( DamageText )
			{
				if( bIsHeal )
				{
					DamageText->SetText( EGFormat( L"|SC(EX_HEAL)|{0}|RC()|" , DmgAmount ) );
				}
				else
				{
					DamageText->SetText( EGFormat( L"{0}" , DmgAmount ) );
				}
			}
			NewPlayingAnim->EntObj.RunEvent( m_AnimTemplates[AnimIndex].StartAnim );
		}
	}
}

void ExCombatTeamWidgetBase::PlayBurgled( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale )
{
	eg_size_t AnimIndex = static_cast<eg_size_t>(ex_combat_anim::Burgled);
	if( m_AnimTemplates.IsValidIndex( AnimIndex ) )
	{
		exPlayingAnimInfo* NewPlayingAnim = new exPlayingAnimInfo;
		if( NewPlayingAnim )
		{
			m_PlayingAnims.Append( NewPlayingAnim );
			NewPlayingAnim->EntObj.InitClone( m_AnimTemplates[AnimIndex].EntObj );
			NewPlayingAnim->AnimTimeLeft = m_AnimTemplates[AnimIndex].AnimDuration;
			NewPlayingAnim->Pose = Pose;
			NewPlayingAnim->ScaleVec = eg_vec4(Scale,Scale,Scale,1.f);
			EGTextNodeComponent* DamageText = NewPlayingAnim->EntObj.GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("DamageNumber") );
			if( DamageText )
			{
				DamageText->SetText( EGFormat( eg_loc("CombatBurgleDamageNumber","|SC(EX_GOLD)|+{0} gold|RC()|") , DmgAmount ) );
			}
			NewPlayingAnim->EntObj.RunEvent( m_AnimTemplates[AnimIndex].StartAnim );
		}
	}
}

void ExCombatTeamWidgetBase::PlayCriticalHit( ex_attr_value DmgAmount , const eg_transform& Pose , eg_real Scale )
{
	eg_size_t AnimIndex = static_cast<eg_size_t>(ex_combat_anim::CriticalHit);
	if( m_AnimTemplates.IsValidIndex( AnimIndex ) )
	{
		exPlayingAnimInfo* NewPlayingAnim = new exPlayingAnimInfo;
		if( NewPlayingAnim )
		{
			m_PlayingAnims.Append( NewPlayingAnim );
			NewPlayingAnim->EntObj.InitClone( m_AnimTemplates[AnimIndex].EntObj );
			NewPlayingAnim->AnimTimeLeft = m_AnimTemplates[AnimIndex].AnimDuration;
			NewPlayingAnim->Pose = Pose;
			NewPlayingAnim->ScaleVec = eg_vec4(Scale,Scale,Scale,1.f);
			EGTextNodeComponent* DamageText = NewPlayingAnim->EntObj.GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("DamageNumber") );
			if( DamageText )
			{
				DamageText->SetText( EGFormat( eg_loc("CombatCriticalHitDamageNumber","|SC(EX_HURT)|CRITICAL|RC()|") , DmgAmount ) );
			}
			NewPlayingAnim->EntObj.RunEvent( m_AnimTemplates[AnimIndex].StartAnim );
		}
	}
}

void ExCombatTeamWidgetBase::PlayResisted( ex_attr_t ResistType , const eg_transform& Pose , eg_real Scale )
{
	eg_size_t AnimIndex = static_cast<eg_size_t>(ex_combat_anim::Resist);
	if( m_AnimTemplates.IsValidIndex( AnimIndex ) )
	{
		exPlayingAnimInfo* NewPlayingAnim = new exPlayingAnimInfo;
		if( NewPlayingAnim )
		{
			m_PlayingAnims.Append( NewPlayingAnim );
			NewPlayingAnim->EntObj.InitClone( m_AnimTemplates[AnimIndex].EntObj );
			NewPlayingAnim->AnimTimeLeft = m_AnimTemplates[AnimIndex].AnimDuration;
			NewPlayingAnim->Pose = Pose;
			NewPlayingAnim->ScaleVec = eg_vec4(Scale,Scale,Scale,1.f);
			EGTextNodeComponent* DamageText = NewPlayingAnim->EntObj.GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("DamageNumber") );
			if( DamageText )
			{
				DamageText->SetText( EGFormat( eg_loc("CombatResistDamageNumber","|SC({1})|RESISTED!|RC()|") , ExCore_GetAttributeName( ResistType ) , ExCore_GetAttributeNameFormatColor( ResistType ) ) );
			}
			NewPlayingAnim->EntObj.RunEvent( m_AnimTemplates[AnimIndex].StartAnim );
		}
	}
}

void ExCombatTeamWidgetBase::UpdateCombatTeamWidgetBase( eg_real DeltaTime )
{
	EGArray<exPlayingAnimInfo*> CompletedAnims;

	for( exPlayingAnimInfo* Anim : m_PlayingAnims )
	{
		Anim->EntObj.Update( DeltaTime );
		Anim->AnimTimeLeft -= DeltaTime;
		if( Anim->AnimTimeLeft < 0.f )
		{
			CompletedAnims.Append( Anim );
		}
	}

	for( exPlayingAnimInfo* DeleteInfo : CompletedAnims )
	{
		m_PlayingAnims.DeleteByItem( DeleteInfo );
		DeleteInfo->EntObj.Deinit();
		delete DeleteInfo;
	}
}

void ExCombatTeamWidgetBase::DrawCombatTeamWidgetBase( const eg_transform& BasePose )
{
	// Currently some animations depend on the z-buffer to draw correctly but 
	// this makes them stomp on other animations. So we have to clear the 
	// z-buffer between each draw. Hopefully we can make no animations depend
	// on the z-buffer and then just disable z drawing for the animations.
	eg_bool bUseNoZDrawMethod = false;

	if( bUseNoZDrawMethod )
	{
		MainDisplayList->PushDepthStencilState( eg_depthstencil_s::ZTEST_NONE_ZWRITE_OFF );
	}

	for( exPlayingAnimInfo* AnimInfo : m_PlayingAnims )
	{
		if( !bUseNoZDrawMethod )
		{
			MainDisplayList->ClearDS( 1.f , 0 );
		}

		eg_transform Pose = BasePose * AnimInfo->Pose;
		AnimInfo->EntObj.SetDrawInfo( Pose , AnimInfo->ScaleVec , false );
		AnimInfo->EntObj.Draw();
	}

	if( bUseNoZDrawMethod )
	{
		MainDisplayList->PopDepthStencilState();
	}
}

void ExCombatTeamWidgetBase::InitCombat( class ExCombat* Combat )
{
	m_Combat = Combat;
	if( m_Combat )
	{
		m_Combat->AddAnimHandler( this );
	}
}

void ExCombatTeamWidgetBase::SetCombatantList( const ExCombatTeam& Team ,  const ExCombatantList& List )
{
	unused( Team );
	
	m_CombatantList = List;
}