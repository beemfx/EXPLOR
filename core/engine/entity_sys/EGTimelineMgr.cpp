// (c) 2017 Beem Media

#include "EGTimelineMgr.h"
#include "EGTimeline.h"

void EGTimelineMgr::StartTimeline( const EGTimeline* Timeline )
{
	// The same timeline can be played multiple times simultaneously.
	m_NextUniqueId++;

	egPlayingInfo NewPlayingInfo;
	NewPlayingInfo.UniqueId = m_NextUniqueId;
	NewPlayingInfo.Timeline = Timeline;
	NewPlayingInfo.PlateState = egTimelinePlayState( CT_Clear );

	NewPlayingInfo.PlateState += EG_SMALL_NUMBER; // This will get anything with time 0 to run immediately (necessary for running timelines with audio muted on frame 1)
	NewPlayingInfo.Timeline->ProcessTimeline( NewPlayingInfo.PlateState , m_Handler );

	if( NewPlayingInfo.PlateState.IsDone() )
	{
		// Only had an event on time 0, no point in adding to manager.
	}
	else
	{
		m_PlayingTimelines.Append( NewPlayingInfo );
	}
}

void EGTimelineMgr::StopTimeline( const EGTimeline* Timeline )
{
	// Stop all timelines that match Timeline
	EGArray<egPlayingInfo> PlayersToCull;

	for( egPlayingInfo& PlayInfo : m_PlayingTimelines )
	{
		if( PlayInfo.Timeline == Timeline )
		{
			PlayersToCull.Append( PlayInfo );
		}
	}

	for( const egPlayingInfo& ToCull : PlayersToCull )
	{
		m_PlayingTimelines.DeleteByItem( ToCull );
	}
}

void EGTimelineMgr::StopAll()
{
	m_PlayingTimelines.Clear();
}

void EGTimelineMgr::Update( eg_real DeltaTime )
{
	EGArray<egPlayingInfo> PlayersToCull;

	for( egPlayingInfo& PlayInfo : m_PlayingTimelines )
	{
		PlayInfo.PlateState += DeltaTime;
		PlayInfo.Timeline->ProcessTimeline( PlayInfo.PlateState , m_Handler );

		if( PlayInfo.PlateState.IsDone() )
		{
			PlayersToCull.Append( PlayInfo );
		}
	}

	for( const egPlayingInfo& ToCull : PlayersToCull )
	{
		m_PlayingTimelines.DeleteByItem( ToCull );
	}
}

eg_bool EGTimelineMgr::IsTimelinePlaying( const EGTimeline* Timeline ) const
{
	for( const egPlayingInfo& PlayInfo : m_PlayingTimelines )
	{
		if( PlayInfo.Timeline == Timeline )
		{
			return true;
		}
	}
	return false;
}
