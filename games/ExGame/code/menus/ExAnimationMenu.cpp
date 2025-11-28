// (c) 2020 Beem Media. All Rights Reserved.

#include "ExAnimationMenu.h"
#include "EGLoader.h"
#include "EGMenuStack.h"
#include "EGUiImageWidget.h"
#include "ExTextRevealWidget.h"
#include "EGSoundScape.h"

EG_CLASS_DECL( ExAnimationMenu )
EG_CLASS_DECL( ExAnimationMenuTimeline )

void ExAnimationMenu::PlayMovie( EGClient* Client , eg_string_crc MovieType  )
{
	eg_string_crc MenuId = CT_Clear;
	eg_cpstr MoviePath = "";

	switch_crc( MovieType )
	{
	case_crc("Intro"):
		MenuId = eg_crc("IntroMovieMenu");
		MoviePath = GAME_DATA_PATH "/movies/IntroMovie/IntroMovie.egasset";
		break;
	case_crc("Ending"):
		MenuId = eg_crc("EndingMovieMenu");
		MoviePath = GAME_DATA_PATH "/movies/EndingMovie/EndingMovie.egasset";
		break;
	case_crc("ClassicIntro"):
		MenuId = eg_crc("ClassicIntroMovieMenu");
		MoviePath = GAME_DATA_PATH "/movies/ClassicIntroMovieAssets/ClassicIntroMovie.egasset";
		break;
	case_crc("IntroCameraSpline"):
		MenuId = eg_crc("IntroCameraSplineMovieMenu");
		MoviePath = GAME_DATA_PATH "/movies/IntroCameraSplineMovie/IntroCameraSplineMovie.egasset";
		break;
	}

	EGMenuStack* MenuStack = Client ? Client->SDK_GetMenuStack() : nullptr;
	if( MenuStack )
	{
		ExAnimationMenu* AnimationMenu = EGCast<ExAnimationMenu>( MenuStack->Push( MenuId ) );
		if( AnimationMenu )
		{
			AnimationMenu->PlayAnimation( MoviePath );
		}
	}
}

void ExAnimationMenu::PlayAnimation( eg_cpstr AssetPath )
{
	m_TimelineIndex = 0;
	m_CurrentDelayTime = 0.f;
	m_CurrentDelayDuration = 0.f;

	assert( IsActive() );
	if( IsActive() )
	{
		EG_SafeDelete( m_TimelineLoader );
		m_TimelineLoader = new ExAnimationMenuTimelineLoader( AssetPath );
	}
}

void ExAnimationMenu::OnInit()
{
	Super::OnInit();
}

void ExAnimationMenu::OnDeinit()
{
	EG_SafeDelete( m_TimelineLoader );
	if( m_bHasPlayedMusic )
	{
		EGClient* PrimaryClient = GetPrimaryClient();
		EGSoundScape* SoundScape = PrimaryClient ? PrimaryClient->SDK_GetSoundScape() : nullptr;
		if( SoundScape )
		{
			SoundScape->PopBgMusic();
		}
	}

	if( m_bIsPlayingCameraSpline )
	{
		m_bIsPlayingCameraSpline = false;
		RunServerEvent( egRemoteEvent( eg_crc("PlayCameraSpline") , eg_string_crc(CT_Clear) ) );
	}

	Super::OnDeinit();
}

void ExAnimationMenu::OnUpdate( eg_real DeltaTime , eg_real AspectRatio )
{
	Super::OnUpdate( DeltaTime , AspectRatio );

	if( m_CurrentDelayDuration > EG_SMALL_NUMBER )
	{
		m_CurrentDelayTime += DeltaTime;
	}

	if( m_CurrentDelayTime >= m_CurrentDelayDuration && m_TimelineLoader && m_TimelineLoader->IsLoaded() )
	{
		const EGArray<exAnimationMenuEvent>& Timeline = m_TimelineLoader->GetTimeline();

		if( Timeline.IsValidIndex( m_TimelineIndex ) )
		{
			ProcessTimeline( Timeline );
		}
		else
		{
			MenuStack_Pop();
		}
	}
}

eg_bool ExAnimationMenu::OnInput( eg_menuinput_t InputType )
{
	if( InputType == eg_menuinput_t::BUTTON_BACK )
	{
		eg_bool bSkipped = false;

		if( EX_CHEATS_ENABLED )
		{
			if( m_TimelineLoader && m_TimelineLoader->IsLoaded() )
			{
				const EGArray<exAnimationMenuEvent>& Timeline = m_TimelineLoader->GetTimeline();
				for( eg_int i=m_TimelineIndex; i<Timeline.Len(); i++ )
				{
					if( Timeline.IsValidIndex( i ) && Timeline[i].bCanSkipToThis )
					{
						m_TimelineIndex = i;
						m_CurrentDelayDuration = 0.f;
						m_CurrentDelayTime = 1.f;
						bSkipped = true;
						ProcessTimeline( Timeline );
						break;
					}
				}
			}
		}

		if( !bSkipped )
		{
			MenuStack_Pop();
		}
		return true;
	}

	return Super::OnInput( InputType );
}

void ExAnimationMenu::ProcessTimeline( const EGArray<exAnimationMenuEvent>& Timeline )
{
	// Process events until m_CurrentDelayDuration > 0
	m_CurrentDelayTime = 0.f;
	m_CurrentDelayDuration = 0.f;

	while( Timeline.IsValidIndex( m_TimelineIndex ) && m_CurrentDelayTime >= m_CurrentDelayDuration )
	{
		const exAnimationMenuEvent& CurrentEvent = Timeline[m_TimelineIndex];
		m_TimelineIndex++;

		m_CurrentDelayDuration = CurrentEvent.DelayTime;

		switch( CurrentEvent.EventType )
		{
			case ex_animation_menu_event_t::None:
			case ex_animation_menu_event_t::Delay:
			{
				// Only add delay.				
			} break;

			case ex_animation_menu_event_t::RunWidgetEvent:
			{
				const exAnimationMenuRunWidgetEvent& RunWidgetEventInfo = CurrentEvent.RunWidgetEventInfo;

				EGUiWidget* Widget = GetWidget<EGUiWidget>( RunWidgetEventInfo.WidgetId );
				if( Widget )
				{
					Widget->RunEvent( RunWidgetEventInfo.EventName );
				}

			} break;

			case ex_animation_menu_event_t::PlayMusic:
			{
				EGClient* PrimaryClient = GetPrimaryClient();
				EGSoundScape* SoundScape = PrimaryClient ? PrimaryClient->SDK_GetSoundScape() : nullptr;
				if( SoundScape )
				{
					if( m_bHasPlayedMusic )
					{
						SoundScape->SwitchBgMusic( *CurrentEvent.PlayMusicInfo.MusicTrack.Path );
					}
					else
					{
						m_bHasPlayedMusic = true;
						SoundScape->PushBgMusic( *CurrentEvent.PlayMusicInfo.MusicTrack.Path );
						SoundScape->ResetBgMusicTrack();
					}
				}
			} break;

			case ex_animation_menu_event_t::PlayCameraSpline:
			{
				m_bIsPlayingCameraSpline = true;
				RunServerEvent( egRemoteEvent( eg_crc("PlayCameraSpline") , CurrentEvent.CameraSplineId ) );
			} break;

			case ex_animation_menu_event_t::StopCameraSpline:
			{
				m_bIsPlayingCameraSpline = false;
				RunServerEvent( egRemoteEvent( eg_crc("PlayCameraSpline") , eg_string_crc(CT_Clear) ) );
			} break;
		}
	}
}

//
// ExAnimationMenuTimelineLoader
//

ExAnimationMenuTimelineLoader::ExAnimationMenuTimelineLoader( eg_cpstr AssetPath )
{
	m_bIsLoading = true;
	MainLoader->BeginLoad( AssetPath , this , EGLoader::LOAD_THREAD_MAIN );
}

ExAnimationMenuTimelineLoader::~ExAnimationMenuTimelineLoader()
{
	if( m_bIsLoading )
	{
		MainLoader->CancelLoad( this );
		m_bIsLoading = false;
	}
}

void ExAnimationMenuTimelineLoader::DoLoad( eg_cpstr strFile , const eg_byte* const pMem , const eg_size_t Size )
{
	EGFileData Data( eg_file_data_init_t::SetableUserPointer );
	Data.SetData( pMem , Size );
	ExAnimationMenuTimeline* AnimationData = EGDataAsset::LoadDataAsset<ExAnimationMenuTimeline>( Data , EGString_ToWide(strFile) );
	if( AnimationData )
	{
		m_TimelineEvents = std::move( AnimationData->m_TimelineEvents );
		EGDeleteObject( AnimationData );
	}
}

void ExAnimationMenuTimelineLoader::OnLoadComplete( eg_cpstr strFile )
{
	unused( strFile );

	m_bIsLoading = false;
	m_bIsLoaded = true;
}
