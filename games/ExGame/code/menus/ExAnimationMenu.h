// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "ExMenu.h"
#include "EGDataAsset.h"
#include "ExAnimationMenu.reflection.h"

class ExAnimationMenuTimelineLoader;
struct exAnimationMenuEvent;
class EGUiImageWidget;
class ExTextRevealWidget;
struct exAnimationMenuImage;

class ExAnimationMenu : public ExMenu
{
	EG_CLASS_BODY( ExAnimationMenu , ExMenu )

protected:

	ExAnimationMenuTimelineLoader* m_TimelineLoader = nullptr;

	eg_int m_TimelineIndex = 0;
	eg_real m_CurrentDelayTime = 0.f;
	eg_real m_CurrentDelayDuration = 0.f;
	eg_bool m_bHasPlayedMusic = false;
	eg_bool m_bIsPlayingCameraSpline = false;

public:

	static void PlayMovie( EGClient* Client , eg_string_crc MovieType );

protected:

	void PlayAnimation( eg_cpstr AssetPath );

	virtual void OnInit() override;
	virtual void OnDeinit() override;
	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override;
	virtual eg_bool OnInput( eg_menuinput_t InputType ) override;

	void ProcessTimeline( const EGArray<exAnimationMenuEvent>& Timeline );
};

egreflect enum class ex_animation_menu_event_t
{
	None ,
	Delay ,
	RunWidgetEvent ,
	PlayMusic ,
	PlayCameraSpline ,
	StopCameraSpline ,
};

egreflect struct exAnimationMenuRunWidgetEvent
{
	egprop eg_string_crc WidgetId = CT_Clear;
	egprop eg_string_crc EventName = CT_Clear;
};

egreflect struct exAnimationMenuPlayMusicEvent
{
	egprop eg_asset_path MusicTrack = eg_asset_path_special_t::Sound;
};

egreflect struct exAnimationMenuEvent
{
	egprop eg_d_string Label;
	egprop ex_animation_menu_event_t EventType = ex_animation_menu_event_t::None;
	egprop eg_real DelayTime = 0.f;
	egprop exAnimationMenuRunWidgetEvent RunWidgetEventInfo;
	egprop exAnimationMenuPlayMusicEvent PlayMusicInfo;
	egprop eg_string_crc CameraSplineId = CT_Clear;
	egprop eg_bool bCanSkipToThis = false;
};

egreflect class ExAnimationMenuTimeline : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExAnimationMenuTimeline , EGDataAsset )
	EG_FRIEND_RFL( ExAnimationMenuTimeline )

public:

	egprop EGArray<exAnimationMenuEvent> m_TimelineEvents;
};

class ExAnimationMenuTimelineLoader : public ILoadable
{
private:

	EGArray<exAnimationMenuEvent> m_TimelineEvents;
	eg_bool m_bIsLoading = false;
	eg_bool m_bIsLoaded = false;

public:

	ExAnimationMenuTimelineLoader( eg_cpstr AssetPath );
	~ExAnimationMenuTimelineLoader();

	eg_bool IsLoaded() const { return m_bIsLoaded; }

	const EGArray<exAnimationMenuEvent>& GetTimeline() const { return m_TimelineEvents; }

private:

	virtual void DoLoad( eg_cpstr strFile , const eg_byte*const  pMem , const eg_size_t Size ) override;
	virtual void OnLoadComplete( eg_cpstr strFile ) override;
};
