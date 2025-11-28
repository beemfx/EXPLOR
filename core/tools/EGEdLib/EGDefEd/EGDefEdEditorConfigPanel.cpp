// (c) 2017 Beem Media

#include "EGDefEdEditorConfigPanel.h"
#include "EGDefEd.h"
#include "EGDefEdFile.h"

EGDefEdEditorConfigPanel::EGDefEdEditorConfigPanel( EGWndPanel* Parent )
: Super( Parent )
{

}

void EGDefEdEditorConfigPanel::HandlePropChanged( const egRflEditor* Property )
{
	Super::HandlePropChanged( Property );

	egEntDefEditNeeds Needs;
	EGDefEdFile::Get().ApplySettingChanges( Property , Needs );
	if( Needs.bRefreshEditor )
	{
		PropEditorRebuild();
	}

	EGDefEd_SetDirty();
}
