#include "ExBgComponent.h"
#include "EGUiLayout.h"
#include "ExGameTypes.h"

EG_CLASS_DECL( ExBgComponent )

static const eg_real EX_MENU_IN_ANIM_TIME = .1f;
static const eg_real EX_MENU_FULLSCREEN_IN_ANIM_TIME = .18f;

void ExBgComponent::InitWidget( ICb* Cb )
{
	m_Cb = Cb;
}

void ExBgComponent::PlayReveal( eg_uint NumChoices /*= 0 */ )
{
	m_AdditionalOffset = eg_vec2(0.f,0.f);

	if( m_Info )
	{
		switch_crc( m_Info->StartupEvent.Crc )
		{
			case_crc("FullScreen"): PlayFullScreenInAnim(); break;
			case_crc("DialogBox"): PlayDialogInAnim( NumChoices ); break;
			case_crc("KeyBindDialogBox"): PlayKeyBindDialogBoxInAnim(); break;
			case_crc("QuestLog"): PlayQuestLogInAnim(); break;
			case_crc("FullScreenSmall"): PlayFullScreenSmallInAnim(); break;
			default:
			{
				if( NumChoices > 0 )
				{
					PlayChoiceWidgetIntroAnim( NumChoices );
				}
				else
				{
					PlayFullScreenInAnim();
				}
			} break;
		}
	}
}

void ExBgComponent::PlayFullScreenInAnim()
{
	RunEvent( eg_crc("FullScreen") );
	m_AnimDuration = EX_MENU_FULLSCREEN_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2( 105.f, 0.f );
	m_HeightFromTo = eg_vec2( 140, 0.f );
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}

void ExBgComponent::PlayFullScreenSmallInAnim()
{
	RunEvent( eg_crc("FullScreenSmall") );
	m_AnimDuration = EX_MENU_FULLSCREEN_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2( 105.f, 0.f );
	m_HeightFromTo = eg_vec2( 140, 0.f );
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}

void ExBgComponent::PlayQuestLogInAnim()
{
	RunEvent( eg_crc( "QuestLog" ) );
	m_AnimDuration = EX_MENU_FULLSCREEN_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2( 105.f, 0.f );
	m_HeightFromTo = eg_vec2( 140, 0.f );
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}


void ExBgComponent::PlayDialogInAnim( eg_uint NumChoices )
{
	RunEvent( eg_crc( "DialogBox" ) );
	m_AnimDuration = EX_MENU_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2( 68.f, 0.f );
	m_HeightFromTo = eg_vec2( 65.f, 0.f + static_cast<eg_int>( NumChoices-1 )*2.625f*2.f*1.5f );
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}

void ExBgComponent::PlayChoiceWidgetIntroAnim( eg_int NumChoices )
{
	m_AnimDuration = EX_MENU_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2(10.f,14.f);
	static const eg_real ADD_BOTTOM_OFFSET = -35.f;
	m_HeightFromTo = eg_vec2(18.f, ADD_BOTTOM_OFFSET + EG_Min<eg_int>(NumChoices,EX_CHOICE_WIDGET_VISIBLE_CHOICES)*30.f*.3f);
	if( NumChoices > EX_CHOICE_WIDGET_VISIBLE_CHOICES )
	{
		static const eg_real SCROLL_BAR_ADJUST = 3.f;
		m_WidthFromTo.y += SCROLL_BAR_ADJUST;
		m_AdditionalOffset.x += SCROLL_BAR_ADJUST; //  * .5f;
	}
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}

void ExBgComponent::PlayKeyBindDialogBoxInAnim()
{
	RunEvent( eg_crc("KeyBindDialogBox") );
	m_AnimDuration = EX_MENU_IN_ANIM_TIME;
	m_WidthFromTo = eg_vec2( -10.f, 60.f );
	m_HeightFromTo = eg_vec2( -10.f, 20.f );
	m_AdditionalOffset = eg_vec2(0.f, 10.f);
	m_RevealTime = 0.f;
	m_bIsRevealing = true;
	if( !m_bAudioMuted )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::DIALOG_OPEN );
	}
}

void ExBgComponent::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	Super::Update( DeltaTime, AspectRatio );

	if( m_bIsRevealing )
	{
		AnimateReveal( DeltaTime );
	}
}

void ExBgComponent::AnimateReveal( eg_real DeltaTime )
{
	m_RevealTime += DeltaTime;

	if( m_RevealTime >= m_AnimDuration )
	{
		m_RevealTime = m_AnimDuration;
		m_bIsRevealing = false;
		if( m_Cb )
		{
			m_Cb->OnBgComponentAnimComplete( this );
		}
	}


	eg_real BlendPct = EGMath_CubicInterp( 0.f, 0.f, 1.f, 0.f, EGMath_GetMappedRangeValue( m_RevealTime, eg_vec2( 0.f, m_AnimDuration ), eg_vec2( 0.f, 1.f ) ) );
	BlendPct = EG_Clamp( BlendPct, 0.f, 1.f );

	eg_transform BasePose;
	eg_transform Tr;

	eg_real StartWidthAdj = m_WidthFromTo.x;
	eg_real WidthAdj = m_WidthFromTo.y;
	eg_real StartHeightAdj = m_HeightFromTo.x;
	eg_real HeightAdj = m_HeightFromTo.y;

	BasePose = eg_transform::BuildTranslation( StartWidthAdj + m_AdditionalOffset.x , m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::BuildTranslation( -WidthAdj + m_AdditionalOffset.x , m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::Lerp( BasePose , Tr, BlendPct );
	SetCBone( CT_Clear , eg_crc( "ul" ), Tr );

	BasePose = eg_transform::BuildTranslation( -StartWidthAdj + m_AdditionalOffset.x , m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::BuildTranslation( WidthAdj + m_AdditionalOffset.x , m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::Lerp( BasePose , Tr, BlendPct );
	SetCBone( CT_Clear , eg_crc( "ur" ), Tr );


	BasePose = eg_transform::BuildTranslation( StartWidthAdj + m_AdditionalOffset.x , StartHeightAdj + m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::BuildTranslation( -WidthAdj + m_AdditionalOffset.x , -HeightAdj + m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::Lerp( BasePose , Tr, BlendPct );
	SetCBone( CT_Clear , eg_crc( "ll" ), Tr );

	BasePose = eg_transform::BuildTranslation( -StartWidthAdj + m_AdditionalOffset.x , StartHeightAdj + m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::BuildTranslation( WidthAdj + m_AdditionalOffset.x , -HeightAdj + m_AdditionalOffset.y , 0.f );
	Tr = eg_transform::Lerp( BasePose , Tr, BlendPct );
	SetCBone( CT_Clear , eg_crc( "lr" ), Tr );
}
