// (c) 2017 Beem Media

#pragma once

#include "EGWndPanel.h"
#include "EGWeakPtr.h"

class EGDefEdTimelineSelector;
class EGDefEdTimelinePanel;
class EGDefEdKeyframePanel;
class EGTimeline;

class EGDefEdTimelineEditor : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	EGWeakPtr<EGTimeline> m_Timeline;
	EGDefEdTimelineSelector* m_TimelineSelector = nullptr;
	EGDefEdTimelinePanel* m_TimelinePanel = nullptr;
	EGDefEdKeyframePanel* m_KeyframePanel = nullptr;

public:

	EGDefEdTimelineEditor( EGWndPanel* Parent );

	void PreviewTimeline( eg_string_crc TimelineId );
	void ResetTimeline();
	void StopTimeline();
	void SetSelectedTimeline( eg_string_crc TimelineId );
	void SetSelectedKeyframe( eg_int KeyframeIndex );
	void NotifyTimelineNameChanged();
	void NotifyFrameTimeChanged();
	void NotifyScriptChanged();
	void Refresh();
};

