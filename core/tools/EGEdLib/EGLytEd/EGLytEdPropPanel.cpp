// (c) 2016 Beem Media

#include "EGLytEdPropPanel.h"
#include "EGLytEdFile.h"
#include "EGUiWidgetInfo.h"
#include "EGLytEd.h"

EGLytEdPropPanel* EGLytEdPropPanel::s_PropPanel = nullptr;

EGLytEdPropPanel::EGLytEdPropPanel( EGWndPanel* Parent )
: Super( Parent )
{
	assert( s_PropPanel == nullptr ); // Can have only one properties panel
	s_PropPanel = this;

	SetNoPropsMessage( GetNoPropsMessage( true , false ) );
}

EGLytEdPropPanel::~EGLytEdPropPanel()
{
	s_PropPanel = nullptr;
}

void EGLytEdPropPanel::HandlePropChanged( const egRflEditor* Property )
{
	unused( Property );

	if( m_CurEditor )
	{
		const_cast<egRflEditor*>(Property)->ExecutePostLoad( EGLytEd_GetCurrentGameFilename() , true );

		egUiWidgetInfo* ChangedProp = static_cast<egUiWidgetInfo*>(m_CurEditor->GetData());
		if( ChangedProp )
		{
			egUiWidgetInfoEditNeeds Needs = ChangedProp->OnEditPropertyChanged( Property );
			if( Needs.bRebuildEditor )
			{
				PropEditorRebuild();
			}
			EGLytEdFile::Get().OnWasChanged( ChangedProp , Needs.bRebuildPreview );
		}
	}
}

void EGLytEdPropPanel::CommitChangesFromPreviewPanel()
{
	if( m_CurEditor )
	{
		EGLytEdFile::Get().OnWasChanged( static_cast<egUiWidgetInfo*>(m_CurEditor->GetData()) , false );
	}

	for( egProp& Prop : m_Props )
	{
		if( Prop.Control && Prop.Info.PropEditor && EGString_Equals( Prop.Info.PropEditor->GetVarName() , "BasePose") )
		{
			Prop.Control->RefreshFromSourceData();
		}
	}
}
