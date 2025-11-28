// (c) 2017 Beem Media

#pragma once

#include "EGWndScrollingPanel.h"
#include "EGCircleArray.h"
#include "EGLogDispatcher.h"

class EGWndDebugLogPanelListener : public EGObject
{
	EG_CLASS_BODY( EGWndDebugLogPanelListener , EGObject )

private:

	class EGWndDebugLogPanel* m_DebugPanel;

public:

	void SetDebugPanel( EGWndDebugLogPanel* DebugPanel ){ m_DebugPanel = DebugPanel; }

	void OnLog( eg_log_channel Channel , const eg_string_base& LogText );
};

class EGWndDebugLogPanel : public EGWndScrollingPanel
{
	typedef EGWndScrollingPanel Super;

private:

	struct egLogItem
	{
		eg_string_big Line = CT_Clear;
		eg_color32    Color = eg_color32( 255 , 255 , 255 );

		egLogItem() = default;
		egLogItem( eg_cpstr InLine , eg_color32 InColor ): Line( InLine ) , Color( InColor ) { }
	};

	static const eg_uint64 UPDATE_TIMER_ID = 1;

private:

	EGWndDebugLogPanelListener   m_Listener;
	EGLogDispatcher              m_LogDispatcher;
	EGCircleArray<egLogItem,200> m_Lines;
	eg_uint                      m_LinesAddedSinceUpdate = 0;

public:

	EGWndDebugLogPanel( EGWndPanel* InParent );
	virtual ~EGWndDebugLogPanel() override;

	// BEGIN EGWndPanel
	virtual eg_size_t GetNumItems() const override;
	virtual void OnPaint( HDC hdc ) override;
	virtual void OnWmTimer( eg_uint64 TimerId ) override;
	// END EGWndPanel

	void InsertLine( eg_log_channel LogChannel , eg_cpstr Line );
};
