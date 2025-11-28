#include "EGMenu.h"
#include "EGSaveMgr.h"
#include "EGEngine.h"
#include "EGUiWidget.h"
#include "EGUiGridWidget.h"
#include "EGUiButtonWidget.h"
#include "EGAlwaysLoaded.h"
#include "EGPath2.h"

class EGLevelListMenu: public EGMenu
{
	EG_CLASS_BODY( EGLevelListMenu , EGMenu )

private:

	struct egLevelInfo
	{
		eg_bool     bIsSave = false;
		eg_d_string DisplayName;
		eg_d_string Filename;
	};

private:

	EGArray<egLevelInfo> m_LevelInfos;

public:

	void BuildLevelsList()
	{
		EGAlwaysLoadedFilenameList EnumList;
		AlwaysLoaded_GetFileList( &EnumList , "egworld" );

		{
			for( egAlwaysLoadedFilename* Filename : EnumList )
			{
				egPathParts2 PathParts = EGPath2_BreakPath( Filename->Filename );
				egLevelInfo NewLevelInfo;
				NewLevelInfo.bIsSave = false;
				NewLevelInfo.Filename = eg_d_string8(*PathParts.ToString( false ));
				NewLevelInfo.DisplayName = EGString_Format( "World: %s" , *eg_d_string8(*PathParts.Filename) );
				m_LevelInfos.Append( NewLevelInfo );
			}
		}

		{
			const eg_uint NumLevels = SaveMgr_GetNumLevels();
			for( eg_uint i=0; i<NumLevels; i++ )
			{
				egSaveInfo SaveInfo;
				SaveMgr_GetLevelInfo( i , &SaveInfo );

				egPathParts2 PathParts = EGPath2_BreakPath( SaveInfo.File );
				egLevelInfo NewLevelInfo;
				NewLevelInfo.bIsSave = true;
				NewLevelInfo.Filename = eg_d_string8(*PathParts.ToString( false ));
				NewLevelInfo.DisplayName = EGString_Format( "Level: %s" , *eg_d_string8(*PathParts.Filename) );
				m_LevelInfos.Append( NewLevelInfo );
			}
		}

		{
			const eg_uint NumSaves = SaveMgr_GetNumUserSaves();
			for( eg_uint i=0; i<NumSaves; i++ )
			{
				egSaveInfo SaveInfo;
				SaveMgr_GetUserSaveInfo( i , &SaveInfo );

				egPathParts2 PathParts = EGPath2_BreakPath( SaveInfo.File );
				egLevelInfo NewLevelInfo;
				NewLevelInfo.bIsSave = true;
				NewLevelInfo.Filename = eg_d_string8(*PathParts.ToString( false ));
				NewLevelInfo.DisplayName = EGString_Format( "Save: %s" , *eg_d_string8(*PathParts.Filename) );
				m_LevelInfos.Append( NewLevelInfo );
			}
		}
	}

	virtual void OnInit() override final
	{
		Super::OnInit();

		BuildLevelsList();

		EGUiButtonWidget* ExitButton = GetWidget<EGUiButtonWidget>( eg_crc("ExitButton") );
		if( ExitButton )
		{
			ExitButton->OnPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
		}

		EGUiGridWidget* LevelsList = GetWidget<EGUiGridWidget>( eg_crc("LevelsList") );
		if( LevelsList )
		{
			LevelsList->OnItemPressedDelegate.Bind( this , &ThisClass::OnObjPressed );
			LevelsList->OnItemChangedDelegate.Bind( this , &ThisClass::OnGridWidgetItemChanged );
			LevelsList->RefreshGridWidget( m_LevelInfos.LenAs<eg_uint>() );
			if( m_LevelInfos.Len() > 0 )
			{
				SetFocusedWidget( LevelsList , 0 , false );
			}
			else
			{
				SetFocusedWidget( ExitButton , 0 , false );
			}
		}
	}

	virtual void OnActivate() override final
	{
		Super::OnActivate();
	}

	virtual eg_bool OnInput( eg_menuinput_t InputType ) override final
	{
		switch( InputType )
		{
		case eg_menuinput_t::BUTTON_BACK:
		{
			MenuStack_Pop();
		} break;
		default: return false;
		}

		return true;
	}

	void OnObjPressed( const egUIWidgetEventInfo& Info )
	{ 
		if( Info.WidgetId == eg_crc("ExitButton") )
		{
			MenuStack_Pop();
		}
		else if( Info.WidgetId == eg_crc("LevelsList") )
		{
			if( m_LevelInfos.IsValidIndex( Info.GridIndex ) )
			{
				if( m_LevelInfos[Info.GridIndex].bIsSave )
				{
					MenuStack_Clear();
					Engine_QueMsg( EGString_Format( "server.Load(\"%s\")" , *m_LevelInfos[Info.GridIndex].Filename ) );
				}
				else
				{
					MenuStack_Clear();
					Engine_QueMsg( EGString_Format( "server.LoadWorld(\"%s\")" , *m_LevelInfos[Info.GridIndex].Filename ) );
				}
			}
		}
	}

	void OnGridWidgetItemChanged( const egUIWidgetEventInfo& ItemInfo )
	{
		if( eg_crc("LevelsList") == ItemInfo.WidgetId && ItemInfo.Widget && ItemInfo.GridWidgetOwner && m_LevelInfos.IsValidIndex( ItemInfo.GridIndex ) )
		{
			ItemInfo.GridWidgetOwner->DefaultOnGridWidgetItemChanged( ItemInfo );
			egSaveInfo LevelInfo;
			SaveMgr_GetLevelInfo( ItemInfo.GridIndex , &LevelInfo );

			ItemInfo.Widget->SetText( eg_crc("ButtonText") , eg_loc_text(EGString_ToWide(*m_LevelInfos[ItemInfo.GridIndex].DisplayName)) );
		}
	}
};

EG_CLASS_DECL( EGLevelListMenu )
