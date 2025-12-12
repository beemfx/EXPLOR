// (c) 2019 Beem Media

#include "ExMenu.h"
#include "EGUiGridWidget.h"
#include "EGAlwaysLoaded.h"
#include "EGPath2.h"
#include "EGEngine.h"

class ExDebugWarpMenu : public ExMenu
{
	EG_CLASS_BODY( ExDebugWarpMenu, ExMenu )
private:

	struct exWorldWarpInfo
	{
		eg_bool     bIsSave = false;
		eg_d_string DisplayName;
		eg_d_string Filename;
	};

private:

	EGUiGridWidget*          m_LevelListWidget;
	EGArray<exWorldWarpInfo> m_WarpDataList;

private:

	virtual void OnInit() override
	{
		Super::OnInit();

		BuildWarpDataList();

		m_LevelListWidget = GetWidget<EGUiGridWidget>( eg_crc("LevelList") );

		if( m_LevelListWidget )
		{
			m_LevelListWidget->RefreshGridWidget( m_WarpDataList.LenAs<eg_uint>() );
			m_LevelListWidget->OnItemChangedDelegate.Bind( this , &ThisClass::OnLevelListItemUpdated );
			m_LevelListWidget->OnItemPressedDelegate.Bind( this , &ThisClass::OnLevelListItemClicked );
			SetFocusedWidget( m_LevelListWidget , 0 , false );
		}

		DoReveal();
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override
	{
		if( InputType == eg_menuinput_t::BUTTON_BACK )
		{
			MenuStack_Pop();
			return true;
		}

		return Super::OnInput( InputType );
	}

	void OnLevelListItemUpdated( const egUIWidgetEventInfo& WidgetInfo )
	{
		if( WidgetInfo.GridWidgetOwner )
		{
			WidgetInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( WidgetInfo );
		}

		if( WidgetInfo.Widget )
		{
			eg_loc_text ItemText;
			if( m_WarpDataList.IsValidIndex( WidgetInfo.GridIndex ) )
			{
				ItemText = eg_loc_text(EGString_ToWide(*m_WarpDataList[WidgetInfo.GridIndex].DisplayName));
			}
			WidgetInfo.Widget->SetText( eg_crc("BaseText") , ItemText );
			WidgetInfo.Widget->SetText( eg_crc("HlText") , ItemText );
		}
	}

	void OnLevelListItemClicked( const egUIWidgetEventInfo& WidgetInfo )
	{
		if( m_WarpDataList.IsValidIndex( WidgetInfo.GridIndex ) )
		{
			MenuStack_Clear();
			GetClientOwner()->SDK_RunClientEvent( egRemoteEvent(eg_crc("ShowFullScreenLoading")) );

			const exWorldWarpInfo& WarpInfo = m_WarpDataList[WidgetInfo.GridIndex];

			Engine_QueMsg( EGString_Format( "server.LoadWorld(\"%s\")" , *WarpInfo.Filename ) );
		}
	}

	void BuildWarpDataList()
	{
		EGAlwaysLoadedFilenameList EnumList;
		AlwaysLoaded_GetFileList( &EnumList, "egworld" );

		{
			for( egAlwaysLoadedFilename* Filename : EnumList )
			{
				egPathParts2 PathParts = EGPath2_BreakPath( Filename->Filename );
				exWorldWarpInfo NewLevelInfo;
				NewLevelInfo.bIsSave = false;
				NewLevelInfo.Filename = *PathParts.ToString( false );
				NewLevelInfo.DisplayName = EGString_Format( "World: %s", *eg_d_string8(*PathParts.Filename) );
				m_WarpDataList.Append( NewLevelInfo );
			}
		}

		m_WarpDataList.Sort( []( const exWorldWarpInfo& Left , const exWorldWarpInfo& Right ) -> eg_bool
		{
			return Left.DisplayName < Right.DisplayName;
		} );
	}
};

EG_CLASS_DECL( ExDebugWarpMenu )
