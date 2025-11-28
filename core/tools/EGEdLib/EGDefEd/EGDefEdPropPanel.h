// (c) 2017 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"

class EGDefEdPropPanel : public EGWndPanelPropEditor
{
private: typedef EGWndPanelPropEditor Super;

private:

	static EGDefEdPropPanel* s_PropPanel;

public:

	static EGDefEdPropPanel* GetPanel(){ return s_PropPanel; }

	EGDefEdPropPanel( EGWndPanel* Parent );
	~EGDefEdPropPanel();

	virtual void HandlePropChanged( const egRflEditor* Editor ) override;

	void CommitChangesFromPreviewPanel();
};
