// (c) 2016 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"

class EGLytEdPropPanel : public EGWndPanelPropEditor
{
private: typedef EGWndPanelPropEditor Super;

private:

	static EGLytEdPropPanel* s_PropPanel;

public:

	static EGLytEdPropPanel* GetPanel(){ return s_PropPanel; }

	EGLytEdPropPanel( EGWndPanel* Parent );
	~EGLytEdPropPanel();

	// BEGIN EGWndPanelPropEditor
	virtual void HandlePropChanged( const egRflEditor* Property ) override;
	// END EGWndPanelPropEditor

	void CommitChangesFromPreviewPanel();
};
