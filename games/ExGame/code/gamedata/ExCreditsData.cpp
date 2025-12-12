// (c) 2018 Beem Media

#include "ExCreditsData.h"

EG_CLASS_DECL( ExCreditsData )
EG_CLASS_DECL( ExCreditsCollection )


void ExCreditsData::OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , RootEditor , bNeedsRebuildOut );

	if( EGString_Equals( ChangedProperty.GetVarName() , "Type" ) || EGString_Equals( ChangedProperty.GetVarName() , "m_Lines" ) )
	{
		bNeedsRebuildOut = true;
		RefreshVisibleProperties( RootEditor );
	}
}

void ExCreditsData::PostLoad( eg_cpstr16 Filename, eg_bool bForEditor, egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( bForEditor )
	{
		RefreshVisibleProperties( RflEditor );
	}
}

void ExCreditsData::RefreshVisibleProperties( egRflEditor& RootEditor )
{
	egRflEditor* LinesEd = RootEditor.GetChildPtr( "m_Lines" );
	if( LinesEd )
	{
		for( eg_size_t i=0; i<LinesEd->GetNumChildren(); i++ )
		{
			egRflEditor* LineEd = LinesEd->GetArrayChildPtr( i );
			if( LineEd )
			{
				auto SetChildEditable = [&LineEd]( eg_cpstr ChildName , eg_bool bEditable ) -> void
				{
					egRflEditor* ChildEd = LineEd->GetChildPtr( ChildName );
					if( ChildEd )
					{
						ChildEd->SetEditable( bEditable );
						ChildEd->SetSerialized( bEditable );
					}
				};

				egRflEditor* TypeEd = LineEd->GetChildPtr( "Type" );
				if( TypeEd )
				{
					ex_credits_line_t LineType = *reinterpret_cast<ex_credits_line_t*>(TypeEd->GetData());
					switch( LineType )
					{
					case ex_credits_line_t::Normal:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , true );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::FuturePop:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::HeaderPop:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::FutureHeaderPop:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::Header:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::Center:
						SetChildEditable( "Title" , false );
						SetChildEditable( "Name" , true );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::Blob:
						SetChildEditable( "Title" , true );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , false );
						break;
					case ex_credits_line_t::Delay:
						SetChildEditable( "Title" , false );
						SetChildEditable( "Name" , false );
						SetChildEditable( "DelayTime" , true );
						break;
					}
				}
			}
		}
	}
}

void ExCreditsCollection::PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor )
{
	Super::PostLoad( Filename , bForEditor , RflEditor );

	if( !bForEditor )
	{
		for( exCreditsDataInfo& CreditsInfo : m_CreditsDataInfo )
		{
			CreditsInfo.LoadedCreditsData = EGDataAsset::LoadDataAsset<ExCreditsData>( *CreditsInfo.CreditsDataPath.GetFullPathWithExt() );
		}
	}
}
