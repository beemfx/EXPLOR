// (c) 2017 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"

class EGDefEdEditorConfigPanel : public EGWndPanelPropEditor
{
private: typedef EGWndPanelPropEditor Super;

public:

	EGDefEdEditorConfigPanel( EGWndPanel* Parent );

private:

	virtual void HandlePropChanged( const egRflEditor* Property ) override;
};

