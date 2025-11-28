// (c) 2017 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"

class EGDefEdSettingsPanel : public EGWndPanelPropEditor
{
	private: typedef EGWndPanelPropEditor Super;

public:

	EGDefEdSettingsPanel( EGWndPanel* Parent );

private:

	virtual void HandlePropChanged( const egRflEditor* Property ) override;
};
