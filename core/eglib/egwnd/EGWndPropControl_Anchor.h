// (c) 2017 Beem Media

#pragma once

#include "EGWndPropControl.h"

class EGWndPropControl_Anchor : public EGWndPropControl
{
private: typedef EGWndPropControl Super;
class EGInternalControl;

public:

	EGWndPropControl_Anchor( IWndPropControlOwner* Parent , const egPropControlInfo& Info );
	~EGWndPropControl_Anchor();

	virtual void CommitChangesToSourceData( HWND CommitControl ) override;
	virtual void RefreshFromSourceData() override;

private:

	POINT GetAnchorAsIndexPoint();
	void SetAnchorFromIndexPoint( const POINT& Point );

protected:

	EGInternalControl* m_AnchorControl;
	eg_anchor           m_AnchorPos;
};
