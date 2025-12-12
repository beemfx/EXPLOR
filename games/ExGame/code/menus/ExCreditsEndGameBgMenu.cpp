// (c) 2020 Beem Media. All Rights Reserved.

#include "ExMenu.h"
#include "EGSoundScape.h"

class ExCreditsEndGameBgMenu : public ExMenu
{
	EG_CLASS_BODY( ExCreditsEndGameBgMenu , ExMenu )

protected:

	EGUiWidget* m_Planet;
	EGUiWidget* m_PlanetLight;
	EGUiWidget* m_Skybox;

	eg_transform m_PlanetRotation = CT_Default;
	eg_transform m_PlanetLightRotation = CT_Default;
	eg_transform m_SkyboxRotation = CT_Default;
	eg_real m_SunRiseTime = 0.f;

	eg_real m_TimeToNextEvent = 0.f;
	eg_real m_bHasPushedCredits = false;
	eg_real m_bPopMenuOnActivate = false;

protected:

	virtual void OnInit() override
	{
		Super::OnInit();

		m_Planet = GetWidget( eg_crc( "Planet" ) );
		m_PlanetLight = GetWidget( eg_crc( "PlanetLight" ) );
		m_Skybox = GetWidget( eg_crc( "Skybox" ) );

		EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
		if( SoundScape )
		{
			// SoundScape->PushBgMusic( "/Music/Credits" );
		}
	}

	virtual void OnDeinit() override
	{
		EGSoundScape* SoundScape = GetPrimaryClient() ? GetPrimaryClient()->SDK_GetSoundScape() : nullptr;
		if( SoundScape )
		{
			// SoundScape->PopBgMusic();
		}

		Super::OnDeinit();
	}

	virtual void OnActivate() override
	{
		Super::OnActivate();

		// We want the sun to start behind the planet and make it's way to
		// about the front. (IN the menu layout the sun starts on the left
		// size)
		m_PlanetLightRotation = eg_transform::BuildRotationY( EG_Rad(.3 * EG_PI / 2.f) );
		m_PlanetLightRotation.NormalizeThis();
		m_SunRiseTime = 0.f;

		if( m_bPopMenuOnActivate )
		{
			MenuStack_Pop();
		}
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
	{
		Super::OnUpdate( DeltaTime , AspectRatio );

		static const eg_real TIME_TO_START_CREDITS = .5f;// 3.f + 1.f; // Delay time (so bg assets can load) + fade time
		static const eg_real TIME_TO_START_SUNRIZE = 10.f;

		m_TimeToNextEvent += DeltaTime;

		if( !m_bHasPushedCredits && m_TimeToNextEvent >= TIME_TO_START_CREDITS )
		{
			m_bHasPushedCredits = true;
			m_bPopMenuOnActivate = true;
			MenuStack_PushTo( eg_crc("CreditsMenu") );
		}

		if( m_TimeToNextEvent <= TIME_TO_START_SUNRIZE )
		{
			m_SunRiseTime = 0.f;
		}

		//
		// Planet animation
		//

		const eg_real PLANET_ROTATION_SECONDS = 180.f;
		const eg_real PLANET_LIGHT_ROT_SECONDS = -240.f;
		const eg_real SKYBOX_ROT_SECONDS = 440.f;

		m_PlanetRotation.RotateYThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / PLANET_ROTATION_SECONDS)) );
		m_PlanetRotation.NormalizeThis();
		m_SkyboxRotation.RotateZThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / SKYBOX_ROT_SECONDS)) );
		m_SkyboxRotation.NormalizeThis();

		if( m_Planet )
		{
			m_Planet->SetOffset( EGUiWidget::eg_offset_t::PRE , m_PlanetRotation );
		}

		if( m_Skybox )
		{
			m_Skybox->SetOffset( EGUiWidget::eg_offset_t::POST , m_SkyboxRotation );
		}

		m_SunRiseTime += DeltaTime;
		if( m_SunRiseTime < EG_Abs( .4f * (PLANET_LIGHT_ROT_SECONDS / 2.f) ) )
		{
			m_PlanetLightRotation.RotateYThis( EG_Rad(DeltaTime * ((EG_PI * 2.f) / PLANET_LIGHT_ROT_SECONDS)) );
			m_PlanetLightRotation.NormalizeThis();

			if( m_PlanetLight )
			{
				// Post offset will preserve distance from planet!
				m_PlanetLight->SetOffset( EGUiWidget::eg_offset_t::POST , m_PlanetLightRotation );
			}
		}
	}
};

EG_CLASS_DECL( ExCreditsEndGameBgMenu )
