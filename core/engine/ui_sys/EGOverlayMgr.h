// EGOverlayMgr
// (c) 2015 Beem Software

#pragma once

#include "EGList.h"

class EGMenu;
class EGClient;

class EGOverlayMgr
{
public:
	EGOverlayMgr( EGClient* Cli ):m_OverlayList(1),m_AiClient(Cli),m_LastKnownAspectRatio(0){ }
	~EGOverlayMgr(){ Clear(); }
	EGMenu* AddOverlay( eg_string_crc MenuId , eg_real DrawPriority ); //Higher draw priorities appear on top of lower ones.
	void RemoveOverlay( EGMenu* Overlay );
	void Clear();
	eg_bool OwnsOverlay( EGMenu* Overlay ) const;

	void Update( eg_real DeltaTime , const struct egLockstepCmds* Input , eg_real AspectRatio );
	void Draw();

	EGClient* GetClient(){ return m_AiClient; }

private:

	void Update_HandleMouse( EGMenu* Menu, const struct egLockstepCmds* Input );

private:
	struct egOverlayItem: public IListable
	{
		class EGMenu* Overlay;
		eg_string_crc MenuId;
		eg_real       DrawPriority;
	};

private:
	EGList<egOverlayItem> m_OverlayList;
	EGClient*const        m_AiClient;
	eg_real               m_LastKnownAspectRatio;
	eg_vec2               m_v2LastMousePos;
	eg_bool               m_bHasInitialMousePos = false;
};