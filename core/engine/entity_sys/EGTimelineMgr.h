// (c) 2017 Beem Media

#pragma once

#include "EGTimelineTypes.h"

class EGTimeline;
class IEGTimelineHandler;

class EGTimelineMgr
{
private:

	struct egPlayingInfo
	{
		eg_uint             UniqueId;
		egTimelinePlayState PlateState;
		const EGTimeline*   Timeline;

		eg_bool operator==( const egPlayingInfo& rhs ) const
		{
			return UniqueId == rhs.UniqueId && Timeline == rhs.Timeline;
		}
	};
	
private:

	IEGTimelineHandler*const  m_Handler = nullptr;
	EGArray<egPlayingInfo>    m_PlayingTimelines;
	eg_uint                   m_NextUniqueId;

public:

	EGTimelineMgr( IEGTimelineHandler* Handler ): m_Handler( Handler ){ }

	void StartTimeline( const EGTimeline* Timeline );
	void StopTimeline( const EGTimeline* Timeline );
	void StopAll();
	void Update( eg_real DeltaTime );
	eg_bool IsTimelinePlaying( const EGTimeline* Timeline ) const;
};
