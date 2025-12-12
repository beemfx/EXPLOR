// (c) 2016 Beem Media. All rights reserved.

#include "ExCharacterPortraitWidget.h"
#include "ExMenu.h"
#include "ExPortraits.h"
#include "ExClient.h"
#include "EGMeshComponent.h"
#include "ExCombatant.h"
#include "EGDebugText.h"

EG_CLASS_DECL( ExCharacterPortraitWidget2 )

const eg_real ExCharacterPortraitWidget2::BAR_MIN = -20.f*2.f;
const eg_real ExCharacterPortraitWidget2::BAR_MAX = 0.f;

void ExCharacterPortraitWidget2::InitPortrait( const ExFighter* Character, eg_uint PartyIndex, const ExGame* Game )
{
	const ExFighter* OldFighter = m_Character;

	m_Character = Character;
	m_PartyIndex = PartyIndex;

	if( m_Character == nullptr && Game != nullptr )
	{
		m_Character = Game->GetPartyMemberByIndex( m_PartyIndex );
	}
	
	if( m_Character != OldFighter )
	{
		if( m_Character )
		{
			m_HealthTween.SetValue( m_Character->GetHP() );
			m_MagicTween.SetValue( m_Character->GetMP() );
		}
		else
		{
			m_HealthTween.SetValue( 0 );
			m_MagicTween.SetValue( 0 );
		}
	}

	m_LevelUpIcon = m_EntObj.GetComponentTree().GetComponentById<EGMeshComponent>( eg_crc("LevelUpIcon") );
	m_BoostIcon = m_EntObj.GetComponentTree().GetComponentById<EGMeshComponent>( eg_crc("BoostIcon") );

	Refresh();
}

void ExCharacterPortraitWidget2::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime , AspectRatio );

	UpdateBars( DeltaTime );
}

void ExCharacterPortraitWidget2::Refresh()
{
	EGMeshComponent* StatusBG = m_EntObj.GetComponentTree().GetComponentById<EGMeshComponent>( eg_crc("StatusBG") );

	if( ExPortraits::IsInitialized() && m_Character )
	{
		SetText( eg_crc("NameText") , eg_loc_text(m_Character->LocName) );
		eg_string_crc StatusText = m_Character->GetStatusText();
		SetText( eg_crc("StatusText") , eg_loc_text(StatusText) );
		if( StatusBG )
		{
			StatusBG->SetHidden( StatusText.IsNull() );
		}
		if( m_LevelUpIcon )
		{
			m_LevelUpIcon->SetHidden( !m_Character->CanTrainToNextLevel() );
		}
		if( m_BoostIcon )
		{
			m_BoostIcon->SetHidden( !m_Character->HasCombatBoost() );
		}
		SetText( eg_crc("DebugText") , CT_Clear );
		SetTexture( eg_crc("Portrait") , eg_crc( "Image" ), *ExPortraits::Get().FindInfo( m_Character->PortraitId ).Filename.FullPath );
	}
	else
	{
		SetText( eg_crc("NameText") , CT_Clear );
		SetText( eg_crc("StatusText") , CT_Clear );
		SetText( eg_crc("DebugText") , CT_Clear );
		if( StatusBG )
		{
			StatusBG->SetHidden( true );
		}
		if( m_LevelUpIcon )
		{
			m_LevelUpIcon->SetHidden( true );
		}
		if( m_BoostIcon )
		{
			m_BoostIcon->SetHidden( true );
		}
		SetTexture( eg_crc("Portrait") , eg_crc( "Image" ) , GAME_DATA_PATH "/portraits/T_Portrait_Empty_d" );
	}

	SetOverrideSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_DEFAULT );

	UpdateBars( 0.f );
}

void ExCharacterPortraitWidget2::SetHighlighted( eg_bool bHighlighted )
{
	RunEvent( bHighlighted ? eg_crc( "Select" ) : eg_crc( "Deselect" ) );
}

void ExCharacterPortraitWidget2::UpdateCombatantView( eg_real DeltaTime , const ExCombatant* Combatant )
{	
	ex_combat_s CombatantState = ex_combat_s::OUT_OF_COMBAT;
	if( Combatant )
	{
		CombatantState = Combatant->GetCombatState();
	}

	const eg_real CombatStatusOffset = GetCombatStatusOffset();

	if( CombatantState != m_CombatState )
	{
 		m_CombatState = CombatantState;
		SetTargetOffset( CombatantState == ex_combat_s::FIRST_ROW ? EX_COMBAT_PORTRAIT_FIRST_ROW_OFFSET : 0.f , CombatStatusOffset );
	}

	eg_transform StatusTransform = CT_Default;
	StatusTransform.TranslateThis( 0.f , CombatStatusOffset , 0.f );

	if( Combatant )
	{
		// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "{0}: Offset: {1} ({2}-{3}) {4}" , *eg_d_string8(Combatant->GetFighter()->LocName) , CombatStatusOffset , m_CombatStatusOffsetStart , m_CombatStatusOffsetEnd , m_CombatStatusOffsetTime ) );
	}

	EGVisualComponent* BaseComp = m_EntObj.GetComponentTree().GetComponentById<EGVisualComponent>( eg_crc("Base") );
	if( BaseComp )
	{
		BaseComp->SetPose( StatusTransform );
	}

	m_CombatStatusOffsetTime += DeltaTime;
}

eg_real ExCharacterPortraitWidget2::GetCombatStatusOffset() const
{
	static const eg_real AnimationTime = .5f;
	if( m_CombatStatusOffsetTime < AnimationTime )
	{
		const eg_real NormalizedTime = EG_Clamp( m_CombatStatusOffsetTime/AnimationTime , 0.f , 1.f );
		const eg_real t = EG_Clamp( EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , NormalizedTime ) , 0.f , 1.f );
		const eg_real CombatStatusOffset = EGMath_GetMappedRangeValue( t , eg_vec2(0.f,1.f) , eg_vec2(m_CombatStatusOffsetStart,m_CombatStatusOffsetEnd) );
		return CombatStatusOffset;
	}

	return m_CombatStatusOffsetEnd;
}

void ExCharacterPortraitWidget2::UpdateBars( eg_real DeltaTime )
{
	eg_transform Joint;

	if( m_Character )
	{
		// Health
		ex_attr_value Health = m_Character->GetHP();
		ex_attr_value MaxHealth = m_Character->GetAttrValue( ex_attr_t::HP );
		if( m_bAnimateBars )
		{
			m_HealthTween.Update( DeltaTime , Health , MaxHealth );
		}
		else
		{
			m_HealthTween.SetValue( Health );
		}
		Joint = eg_transform::BuildTranslation( eg_vec3(EGMath_GetMappedRangeValue( m_HealthTween.GetCurrentValue() , eg_vec2(0.f,static_cast<eg_real>(MaxHealth)) , eg_vec2(BAR_MIN,BAR_MAX)) , 0 , 0 ) );
		SetCBone( CT_Clear , eg_crc( "HealthJoint" ) , Joint );

		// Mana
		ex_attr_value Mana = m_Character->GetMP();
		ex_attr_value MaxMana = m_Character->GetAttrValue( ex_attr_t::MP );
		if( m_bAnimateBars )
		{
			m_MagicTween.Update( DeltaTime , Mana , MaxMana );
		}
		else
		{
			m_MagicTween.SetValue( Mana );
		}
		Joint = eg_transform::BuildTranslation( eg_vec3(EGMath_GetMappedRangeValue( m_MagicTween.GetCurrentValue() , eg_vec2(0.f,static_cast<eg_real>(MaxMana)) , eg_vec2(BAR_MIN,BAR_MAX)) , 0 , 0 ) );
		SetCBone( CT_Clear , eg_crc( "MagicJoint" ) , Joint );

		SetText( eg_crc("DebugText") , CT_Clear );//EGFormat( L"{0}" , m_Character->GetAttrValue( ex_attr_t::AGR ) ) );
	}
	else
	{
		Joint = eg_transform::BuildTranslation( eg_vec3(BAR_MIN,0.f,0.f) );
		SetCBone( CT_Clear , eg_crc( "HealthJoint" ) , Joint );
		SetCBone( CT_Clear , eg_crc( "MagicJoint" ) , Joint );

		SetText( eg_crc("DebugText") , CT_Clear );
	}
}

void ExCharacterPortraitWidget2::SetTargetOffset( eg_real NewOffset , eg_real PrevOffset )
{
	m_CombatStatusOffsetTime = 0.f;
	m_CombatStatusOffsetStart = PrevOffset;
	m_CombatStatusOffsetEnd = NewOffset;
}
