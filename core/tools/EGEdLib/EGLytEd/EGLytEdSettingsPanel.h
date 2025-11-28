// (c) 2017 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"

class EGLytEdSettingsPanel : public EGWndPanelPropEditor
{
private: typedef EGWndPanelPropEditor Super;

private:

	static EGLytEdSettingsPanel* GlobalPanel;

public:

	static EGLytEdSettingsPanel* GetPanel(){ return GlobalPanel; }

	EGLytEdSettingsPanel( EGWndPanel* Parent );
	virtual ~EGLytEdSettingsPanel() override;

	// BEGIN EGWndPanelPropEditor
	virtual void HandlePropChanged( const egRflEditor* Property ) override;
	// END EGWndPanelPropEditor
};
