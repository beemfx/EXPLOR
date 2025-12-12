// (c) 2017 Beem Media

#include "ExCombatPartyWidget.h"
#include "ExCombat.h"
#include "ExCharacterPortraitWidget.h"
#include "EGAudioList.h"
#include "EGAudio.h"
#include "ExRoster.h"
#include "ExUiSounds.h"

EG_CLASS_DECL( ExCombatPartyWidget )

static const eg_real EX_COMBAT_PARTY_WIDGET_SCALE_SHRINK = EX_PORTRAIT_SCALE/EX_PORTRAIT_SCALE_BIG;

void ExCombatPartyWidget::SetCombatantList( const ExCombatTeam& Team , const ExCombatantList& List )
{
	ExCombatTeamWidgetBase::SetCombatantList( Team , List );
	RefreshGridWidget( static_cast<eg_uint>(m_CombatantList.Len()) );
}

void ExCombatPartyWidget::SetScaledUp( eg_bool bNewValue )
{
	static const eg_real SCALE_TIME = .5f;

	if( bNewValue )
	{
		m_ScaleTarget.SetValue( EX_COMBAT_PARTY_WIDGET_SCALE_SHRINK );
		m_ScaleTarget.MoveTo( 1.f , SCALE_TIME , eg_tween_animation_t::Cubic );
	}
	else
	{
		m_ScaleTarget.SetValue( 1.f );
		m_ScaleTarget.MoveTo( EX_COMBAT_PARTY_WIDGET_SCALE_SHRINK , SCALE_TIME , eg_tween_animation_t::Cubic );
	}
}

void ExCombatPartyWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	m_ScaleTarget.SetValue( 1.f );

	CellChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
}

void ExCombatPartyWidget::SetHighlightedPortrait( eg_uint Index )
{
	m_HighlightedPortrait = Index;
}

void ExCombatPartyWidget::SetHighlightedFighter( ExCombatant* Combatant )
{
	assert( !m_bIsCastSpellInWorld );
	if( m_bIsCastSpellInWorld )
	{
		return;
	}

	m_HighlightedPortrait = countof(m_PortraitWidgets);

	if( Combatant == nullptr )
	{
		return;
	}

	for( eg_uint i=0; i<countof(m_PortraitWidgets); i++ )
	{
		if( m_PortraitWidgets[i] )
		{
			eg_bool bIsHighlighted = i == Combatant->GetServerPartyIndex() && Combatant->GetTeamIndex() == 0;// m_PortraitWidgets[i]->GetCharacter() == &Combatant->m_Fighter;
			if( bIsHighlighted )
			{
				m_HighlightedPortrait = i;
			}
		}
	}
}

void ExCombatPartyWidget::OnConstruct()
{
	Super::OnConstruct();

	InitCombatTeamWidgetBase();

	m_MaleGotHit.Append( EGAudio_CreateSound( "/combat/male_hit_01" ) );
	m_MaleGotHit.Append( EGAudio_CreateSound( "/combat/male_hit_02" ) );

	m_FemaleGotHit.Append( EGAudio_CreateSound( "/combat/female_hit_01" ) );
	m_FemaleGotHit.Append( EGAudio_CreateSound( "/combat/female_hit_02" ) );

	m_Rng.CreateSeed();
}

void ExCombatPartyWidget::OnDestruct()
{
	for( egs_sound Sound : m_MaleGotHit )
	{
		EGAudio_DestroySound( Sound );
	}
	m_MaleGotHit.Clear();

	for( egs_sound Sound : m_FemaleGotHit )
	{
		EGAudio_DestroySound( Sound );
	}
	m_FemaleGotHit.Clear();

	DeintCombatTeamWidgetBase();
	
	Super::OnDestruct();
}

void ExCombatPartyWidget::Update( float DeltaTime , float AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	m_ScaleTarget.Update( DeltaTime );

	m_GridWidgetScale = m_ScaleTarget.GetCurrentValue();

	const eg_real MOVE_DIST = 24.57f; // We should do a calculation for this number, but this is what was taken from the UI editor when calculating the offset of the big portratis vs small ones.

	m_AdditionalOffset.x = EGMath_GetMappedRangeValue( m_GridWidgetScale , eg_vec2(EX_COMBAT_PARTY_WIDGET_SCALE_SHRINK,1.f) , eg_vec2(MOVE_DIST,0.f) );

	if( !m_bIsCastSpellInWorld )
	{
		for( eg_size_t i=0; i<countof(m_PortraitWidgets); i++ )
		{
			ExCharacterPortraitWidget2* PortraitWidget = m_PortraitWidgets[i];
			if( PortraitWidget )
			{
				eg_bool bIsHighlighted = i == m_HighlightedPortrait;
				PortraitWidget->SetHighlighted( bIsHighlighted );
				PortraitWidget->UpdateCombatantView( DeltaTime , m_CombatantList.IsValidIndex(i) ? m_CombatantList[i] : nullptr );
			}
		}
	}

	UpdateCombatTeamWidgetBase( DeltaTime );
}

void ExCombatPartyWidget::Draw( eg_real AspectRatio )
{
	Super::Draw( AspectRatio );

	eg_transform BasePose( CT_Default );

	DrawCombatTeamWidgetBase( BasePose );
}

void ExCombatPartyWidget::OnGridWidgetItemChanged( egUiGridWidgetCellChangedInfo2& CellInfo )
{
	if( m_bIsCastSpellInWorld )
	{
		assert( false ); // Should be overridden
		return;
	}

	if( m_CombatantList.IsValidIndex(CellInfo.GridIndex) )
	{
		const ExCombatant* ThisFighter = m_CombatantList[CellInfo.GridIndex];
	
		if( EG_IsBetween<eg_size_t>( CellInfo.GridIndex , 0 , countof(m_PortraitWidgets)-1 ) )
		{
			m_PortraitWidgets[CellInfo.GridIndex] = EGCast<ExCharacterPortraitWidget2>(CellInfo.GridItem);
			if( m_PortraitWidgets[CellInfo.GridIndex] )
			{
				m_PortraitWidgets[CellInfo.GridIndex]->SetAnimateBars( true );
				m_PortraitWidgets[CellInfo.GridIndex]->InitPortrait( ThisFighter ? &ThisFighter->GetFighter() : nullptr , 0 , nullptr );
			}
		}
	}

	if( CellInfo.IsNewlySelected() )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
	}
}

void ExCombatPartyWidget::OnGridWidgetItemChangedFromCastSpellInWorld( egUiGridWidgetCellChangedInfo2& CellInfo , ExGame* Game )
{
	if( EG_IsBetween<eg_size_t>( CellInfo.GridIndex , 0 , countof(m_PortraitWidgets)-1 ) )
	{
		m_PortraitWidgets[CellInfo.GridIndex] = EGCast<ExCharacterPortraitWidget2>( CellInfo.GridItem );
		if( m_PortraitWidgets[CellInfo.GridIndex] )
		{
			m_PortraitWidgets[CellInfo.GridIndex]->SetAnimateBars( true );
			m_PortraitWidgets[CellInfo.GridIndex]->InitPortrait( nullptr , CellInfo.GridIndex , Game );
		}

		if( CellInfo.IsNewlySelected() )
		{
			CellInfo.GridItem->RunEvent( eg_crc("Select") );
			if( IsEnabled() )
			{
				ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
			}
		}
		if( CellInfo.IsNewlyDeselected() )
		{
			CellInfo.GridItem->RunEvent( eg_crc("Deselect") );
		}
	}
}

void ExCombatPartyWidget::PlayAnimation( const exCombatAnimInfo& AnimInfo )
{	
	auto GetCombatantWidgetIndex = [this]( ExCombatant* Combatant ) -> eg_uint
	{
		return Combatant && Combatant->GetTeamIndex() == 0 ? Combatant->GetServerPartyIndex() : countof(m_PortraitWidgets);
	};
		
	auto GetCombatantWidget = [this,&GetCombatantWidgetIndex]( ExCombatant* Combatant ) -> ExCharacterPortraitWidget2*
	{
		eg_uint Index = GetCombatantWidgetIndex( Combatant );

		if( EG_IsBetween<eg_uint>( Index , 0 , countof(m_PortraitWidgets)-1 ) )
		{
			return m_PortraitWidgets[Index];
		}

		return nullptr;
	};

	ExCharacterPortraitWidget2* DefenderWidget = GetCombatantWidget( AnimInfo.Defender );
	eg_uint PortraitIndex = GetCombatantWidgetIndex( AnimInfo.Defender );
	const eg_real StatusOffset = DefenderWidget ? DefenderWidget->GetCombatStatusOffset() : 0.f;
	const eg_transform BasePose = GetAnimationOffset( PortraitIndex , StatusOffset , m_bBigPortraits );

	if( AnimInfo.Type == ex_combat_anim::MeleeAttack )
	{
		if( DefenderWidget && AnimInfo.Defender )
		{
			ex_gender_t Gender = AnimInfo.Defender->GetFighter().GetGender();

			egs_sound GotHitSound = egs_sound::Null;

			switch( Gender )
			{
			case ex_gender_t::Unknown:
			case ex_gender_t::Male:
			{
				eg_int RandNumber = m_Rng.GetRandomRangeI(0,m_MaleGotHit.LenAsInt()-1);
				GotHitSound = m_MaleGotHit[RandNumber];
			} break;
			case ex_gender_t::Female:
				eg_int RandNumber = m_Rng.GetRandomRangeI(0,m_FemaleGotHit.LenAsInt()-1);
				GotHitSound = m_FemaleGotHit[RandNumber];
				break;
			}

			if( AnimInfo.Damage > 0 )
			{
				MainAudioList->PlaySound( GotHitSound );
			}

			PlayCombatTeamWidgetBaseAnimation( AnimInfo.Type , BasePose , .5f , AnimInfo.Damage > 0 ? eg_color32(181,26,0) : eg_color32(255,255,255) );
		}
	}
	else if( AnimInfo.Type == ex_combat_anim::SpellBonus )
	{
		if( DefenderWidget && AnimInfo.Defender )
		{
			PlayCombatTeamWidgetBaseAnimation( AnimInfo.Type , BasePose , m_bBigPortraits ? .5f : .3f , eg_color32(0,162,232) );
		}
	}
	else if( AnimInfo.Type == ex_combat_anim::SpellAttack )
	{
		if( DefenderWidget && AnimInfo.Defender )
		{
			PlayCombatTeamWidgetBaseAnimation( AnimInfo.Type , BasePose , m_bBigPortraits ? .5f : .3f , eg_color32(181,26,0) );
		}
	}
	else if( AnimInfo.Type == ex_combat_anim::SpellFail )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::NOT_ALLOWED );
	}

	const eg_bool bPlayDamageNumber = AnimInfo.IsDamageNumberType() && AnimInfo.Damage != 0;
	if( DefenderWidget && AnimInfo.Defender && bPlayDamageNumber )
	{
		PlayDamageNumber( AnimInfo.Damage , BasePose , m_bBigPortraits ? 1.f : .7f , AnimInfo.Type == ex_combat_anim::SpellBonus );
	}
}

eg_transform ExCombatPartyWidget::GetAnimationOffset( eg_int PortraitIndex , eg_real StatusOffset , eg_bool bBigPortraits )
{
	if( bBigPortraits )
	{
		static const eg_real PORTRAIT_WIDTH = EX_PORTRAIT_WIDTH*EX_PORTRAIT_SCALE_BIG; // Size and scaling in this menu...
		return eg_transform::BuildTranslation( eg_vec3( -2.5f * PORTRAIT_WIDTH  + PortraitIndex*PORTRAIT_WIDTH , -70.f + StatusOffset*EX_PORTRAIT_SCALE_BIG , 0.f ) );
	}
	else
	{
		static const eg_real PORTRAIT_WIDTH = EX_PORTRAIT_WIDTH*EX_PORTRAIT_SCALE; // Size and scaling in this menu...
		return eg_transform::BuildTranslation( eg_vec3( -2.5f * PORTRAIT_WIDTH  + PortraitIndex*PORTRAIT_WIDTH , -77.f + StatusOffset*EX_PORTRAIT_SCALE , 0.f ) );
	}

	return CT_Default;
}
