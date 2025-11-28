// (c) 2018 Beem Media

#pragma once

#include "EGWndPanel.h"

class EGThread;
class EGEdConfigSyncProc;

class EGEdConfigSyncPanel : public EGWndPanel
{
	EG_DECL_SUPER( EGWndPanel )

private:

	static const eg_uint64 POLL_SYNC_DONE_TIMER_ID = 56;

	EGThread*           m_WorkerThread = nullptr;
	EGEdConfigSyncProc* m_WorkerProc = nullptr;
	EGWndAppWindow&     m_AppOwner;
	eg_bool             m_bIsSyncing = false;
	eg_int              m_BuildElipses = 0;
	eg_s_string_sml8    m_DisplayText = CT_Clear;

public:

	EGEdConfigSyncPanel( EGWndPanel* Parent , EGWndAppWindow* InAppOwner );
	virtual ~EGEdConfigSyncPanel() override;

	void BeginSync( eg_cpstr SyncHost , eg_cpstr OutDir , eg_bool bBinariesOnly );

	virtual void OnWmTimer( eg_uint64 TimerId ) override;
	virtual void OnDrawBg( HDC hdc ) override;
	virtual void OnPaint( HDC hdc ) override;
};
