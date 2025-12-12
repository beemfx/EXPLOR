// (c) 2016 Beem Media

#include "ExMenu.h"
#include "EGParse.h"
#include "ExEnt.h"
#include "ExMapInfos.h"
#include "ExClient.h"
#include "ExMapping.h"
#include "ExPlayerController.h"
#include "ExHudTextWidget.h"
#include "ExMapIconComponent.h"

static const eg_vec2 ExHUDAutomap_AutomapTileSize( 40.f/5.f , 40.f/5.f );

class ExHUDAutomap: public ExMenu
{
	EG_CLASS_BODY( ExHUDAutomap , ExMenu )

private:

	struct exTrackedIcon
	{
		EGUiMeshWidget* Widget = nullptr;
		ExEnt* Ent = nullptr;
		eg_bool bClampToEdge = false;
		mutable eg_real DistSqFromPlayer = 0.f;
	};

private:

	EGUiMeshWidget* m_Automap;
	EGUiMeshWidget* m_PlayerPoseWidget;
	EGUiMeshWidget* m_MapIconTemplate;
	EGUiMeshWidget* m_ClampedMapIconTemplate;
	EGWeakPtr<ExMapping> m_Mapping;
	exMapInfoData m_MapInfo;
	EGArray<EGUiMeshWidget*> m_AvailableMapIcons;
	EGArray<EGUiMeshWidget*> m_AvailableClampedMapIcons;
	EGArray<exTrackedIcon> m_TrackedIcons;
	eg_bool m_bCurrentlyVisible = true;

public:

	virtual void Refresh() override final
	{
		ExMenu::Refresh();

		m_MapInfo = ExMapInfos::Get().FindInfo( GetGame()->GetCurrentMapId() );

		if( m_Automap )
		{	
			OnMapMaterialChanged( m_Mapping->GetMapRenderMaterial() );
		}
	}

	virtual void OnInit() override final
	{ 
		Super::OnInit();
		m_Mapping = GetGame() ? GetGame()->GetMappingModule() : nullptr;
		m_Automap = GetWidget<EGUiMeshWidget>( eg_crc("Automap") );
		m_PlayerPoseWidget = GetWidget<EGUiMeshWidget>( eg_crc("PlayerPose") );
		m_MapIconTemplate = GetWidget<EGUiMeshWidget>( eg_crc("MapIconTemplate") );
		m_ClampedMapIconTemplate = GetWidget<EGUiMeshWidget>( eg_crc("ClampedMapIconTemplate") );
		m_MapInfo.Id = CT_Clear;

		if( m_Automap )
		{
			m_Automap->SetOverrideSamplerState( eg_sampler_s::TEXTURE_CLAMP_FILTER_POINT );
		}

		if( m_MapIconTemplate )
		{
			m_MapIconTemplate->SetVisible( false );
		}

		if( m_ClampedMapIconTemplate )
		{
			m_ClampedMapIconTemplate->SetVisible( false );
		}

		GetGame()->GetMappingModule()->MapMaterialChangedDelegate.AddUnique( this , &ThisClass::OnMapMaterialChanged );
		GetGame()->OnTrackedMapIconsChanged.AddUnique( this , &ThisClass::OnTrackedMapIconsChanged );
		OnTrackedMapIconsChanged();
		Refresh();
	}

	virtual void OnDeinit() override final
	{
		GetGame()->OnTrackedMapIconsChanged.RemoveAll( this );
		GetGame()->GetMappingModule()->MapMaterialChangedDelegate.RemoveAll( this );

		Super::OnDeinit();
	}

	virtual void OnUpdate( eg_real DeltaTime , eg_real AspectRatio ) override final
	{
		Super::OnUpdate( DeltaTime , AspectRatio );
		eg_angle Rot = EG_Rad(0.f);
		eg_vec2 PlayerPos(0.f,0.f);
		eg_ent_id EntId = GetGame()->GetPrimaryPlayer();
		const ExEnt* Ent = FindEntData( EntId );
		if( Ent )
		{
			eg_transform EntPose = Ent->GetPose();
			eg_vec4 FaceDir( 0 , 0 , 1.f , 0.f );
			eg_transform EntPoseM( EntPose );
			FaceDir *= EntPoseM;
			const eg_real& y = FaceDir.x;
			const eg_real& x = FaceDir.z;

			Rot = EGMath_atan2( y , x );

			PlayerPos.x = EntPose.GetTranslation().x/m_MapInfo.MapMetaInfo.TileSize.x - .5f;
			PlayerPos.y = EntPose.GetTranslation().z/m_MapInfo.MapMetaInfo.TileSize.y - .5f;
		}

		if( m_Automap )
		{
			eg_real OffsetX = ExHUDAutomap_AutomapTileSize.x*(-PlayerPos.x + 2.f + m_MapInfo.MapMetaInfo.LowerLeftOrigin.x);
			eg_real OffsetY = ExHUDAutomap_AutomapTileSize.y*(m_MapInfo.MapMetaInfo.MapSize.y-PlayerPos.y - 3.f + m_MapInfo.MapMetaInfo.LowerLeftOrigin.y );

			eg_real ScaleX = ExHUDAutomap_AutomapTileSize.x*(m_MapInfo.MapMetaInfo.MapSize.x-5.f);
			eg_real ScaleY = ExHUDAutomap_AutomapTileSize.y*(m_MapInfo.MapMetaInfo.MapSize.y-5.f);

			//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format( "Pos (%g,%g)" , OffsetX , OffsetY ) );

			eg_transform Pose;
			// Upper left is only the offset
			Pose = eg_transform::BuildTranslation( eg_vec3(OffsetX , OffsetY , 0.f) );
			m_Automap->SetCBone( eg_crc("Base") , eg_crc("MapUL") , Pose );
			// Upper right is offset + span x
			Pose = eg_transform::BuildTranslation( eg_vec3(OffsetX + ScaleX , OffsetY , 0.f) );
			m_Automap->SetCBone( eg_crc("Base") , eg_crc("MapUR") , Pose );
			// Lower left is offset + span y
			Pose = eg_transform::BuildTranslation( eg_vec3(OffsetX , OffsetY - ScaleY , 0.f) );
			m_Automap->SetCBone( eg_crc("Base") , eg_crc("MapLL") , Pose );
			// Lower right is offset + span x, span y
			Pose = eg_transform::BuildTranslation( eg_vec3(OffsetX + ScaleX , OffsetY - ScaleY , 0.f) );
			m_Automap->SetCBone( eg_crc("Base") , eg_crc("MapLR") , Pose );
		}

		if( m_PlayerPoseWidget )
		{
			eg_transform Pose = eg_transform::BuildIdentity();
			Pose.RotateZThis( -Rot );

			m_PlayerPoseWidget->SetOffset( EGUiWidget::eg_offset_t::PRE , Pose );
		}

		for( const exTrackedIcon& TrackedIcon : m_TrackedIcons )
		{
			SetWorldWidgetPose( TrackedIcon );
		}

		// Sort the icons by distance from player and change their z-order.
		{
			m_TrackedIcons.Sort( []( const exTrackedIcon& Left , const exTrackedIcon& Right ) -> eg_bool
			{
				return Left.DistSqFromPlayer > Right.DistSqFromPlayer;
			} );

		
			EGUiMeshWidget* NextWidgetOnTop = nullptr;
			for( const exTrackedIcon& TrackedIcon : m_TrackedIcons )
			{
				if( TrackedIcon.bClampToEdge ) // Only need to move clamped icons.
				{
					if( nullptr == NextWidgetOnTop )
					{
						NextWidgetOnTop = TrackedIcon.Widget;
					}
					else
					{
						if( TrackedIcon.Widget )
						{
							MoveWidgetAfter( TrackedIcon.Widget , NextWidgetOnTop );
							NextWidgetOnTop = TrackedIcon.Widget;
						}
					}
				}
			}
		}
	}

	virtual void OnPreDraw( eg_real AspectRatio ) override
	{
		Super::OnPreDraw( AspectRatio );

		if( GetGame() )
		{
			GetGame()->GetMappingModule()->SetGlobalSamplerToMap();
		}
	}

	void OnMapMaterialChanged( egv_material NewMaterial )
	{
		if( m_Automap )
		{
			m_Automap->SetMaterial( eg_crc( "Base" ), eg_crc( "Map" ), NewMaterial );
		}
	}

	virtual void OnHudVisibleChanged(  eg_bool bVisible , eg_bool bPlayTransition )
	{
		m_bCurrentlyVisible = bVisible;

		auto HandleMeshWidget = [bVisible,bPlayTransition]( EGUiWidget* Widget ) -> void
		{
			if( Widget )
			{
				if( bVisible )
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Show") : eg_crc("ShowNow") );
				}
				else
				{
					Widget->RunEvent( bPlayTransition ? eg_crc("Hide") : eg_crc("HideNow") );
				}
			}
		};

		auto HandleTextWidget = [bVisible,bPlayTransition]( ExHudTextWidget* TextWidget ) -> void
		{
			if( TextWidget )
			{
				if( bVisible )
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayShow();
					}
					else
					{
						TextWidget->HudShowNow();
					}
				}
				else
				{
					if( bPlayTransition )
					{
						TextWidget->HudPlayHide();
					}
					else
					{
						TextWidget->HudHideNow();
					}
				}
			}
		};

		HandleMeshWidget( m_Automap );
		HandleMeshWidget( m_PlayerPoseWidget );
		HandleMeshWidget( GetWidget( eg_crc("Bg") ) );
		HandleMeshWidget( GetWidget( eg_crc("MapBg") ) );
		for( exTrackedIcon& TrackedIcon : m_TrackedIcons )
		{
			HandleMeshWidget( TrackedIcon.Widget );
		}
	}

	EGUiMeshWidget* CreateMapIcon( egv_material InMat , eg_bool bClamped )
	{
		EGUiMeshWidget* OutImage = nullptr;
		EGArray<EGUiMeshWidget*>& IconArray = bClamped ? m_AvailableClampedMapIcons : m_AvailableMapIcons;
		if( IconArray.HasItems() )
		{
			OutImage = IconArray.Last();
			IconArray.DeleteByItem( OutImage );
		}

		if( nullptr == OutImage )
		{
			OutImage = EGCast<EGUiMeshWidget>(DuplicateWidget( bClamped ? m_ClampedMapIconTemplate : m_MapIconTemplate ));
		}

		if( OutImage )
		{
			OutImage->SetVisible( true );
			OutImage->SetMaterial( eg_crc("Icon") , eg_crc("Image") , InMat );
			OutImage->RunEvent( m_bCurrentlyVisible ? eg_crc("ShowNow") : eg_crc("HideNow") );
		}

		return OutImage;
	}

	void DestroyMapIcon( EGUiMeshWidget* InIcon , eg_bool bClamped)
	{
		if( InIcon )
		{
			InIcon->SetVisible( false );
			EGArray<EGUiMeshWidget*>& IconArray = bClamped ? m_AvailableClampedMapIcons : m_AvailableMapIcons;
			IconArray.Append( InIcon );
		}
	}

	void ClearTrackedIcons()
	{
		for( exTrackedIcon& TrackedIcon : m_TrackedIcons )
		{
			DestroyMapIcon( TrackedIcon.Widget , TrackedIcon.bClampToEdge );
		}

		m_TrackedIcons.Clear( false );
	}

	void OnTrackedMapIconsChanged()
	{
		ClearTrackedIcons();

		EGArray<ExMapIconComponent*> TrackedMapIcons;
		if( ExGame* Game = GetGame() )
		{
			TrackedMapIcons = Game->GetTrackedMapIcons();
		}

		for( ExMapIconComponent* TrackedMapIcon : TrackedMapIcons )
		{
			ExEnt* Ent = TrackedMapIcon && TrackedMapIcon->GetIconMaterial() != EGV_MATERIAL_NULL ? TrackedMapIcon->GetOwner<ExEnt>() : nullptr;
			if( Ent )
			{
				exTrackedIcon NewIcon;
				NewIcon.Ent = Ent;
				NewIcon.bClampToEdge = TrackedMapIcon->IsIconClampedToMinimapEdge();
				NewIcon.Widget = CreateMapIcon( TrackedMapIcon->GetIconMaterial() , NewIcon.bClampToEdge );
				m_TrackedIcons.Append( NewIcon );
			}
		}
	}

	static eg_vec2 GetEdgePos( const eg_vec2& Dir , const eg_vec2& UpperRight )
	{
		// We are simplifying the math because we know some things about this so
		// the planes are thus:
		// Upper: <0,-1,0,UpperRight.y>
		// Lower: <0,1,0,UpperRight.y>
		// Right: <-1,0,0,UpperRight.x>
		// Left: <1,0,0,UpperRight.x>
		// Dir: <Dir.x,Dir.y,0,0>

		eg_vec2 Out = Dir;
		eg_real SmallestT = 1e32f;

		if( Dir.y > EG_SMALL_NUMBER ) // Hits top bound
		{
			eg_real t = UpperRight.y/Dir.y; // This is dot product with plane
			assert( t > 0.f );
			if( t < SmallestT )
			{
				SmallestT = t;
			}
		}
		else if( Dir.y < -EG_SMALL_NUMBER ) // Hits bottom bound
		{
			eg_real t = -UpperRight.y/Dir.y; // This is dot product with plane
			assert( t > 0.f );
			if( t < SmallestT )
			{
				SmallestT = t;
			}
		}
		else // Hits right or left
		{
		
		}

		if( Dir.x > EG_SMALL_NUMBER ) // Hits right bound
		{
			eg_real t = UpperRight.x/Dir.x; // This is dot product with plane
			assert( t > 0.f );
			if( t < SmallestT )
			{
				SmallestT = t;
			}
		}
		else if( Dir.x < - EG_SMALL_NUMBER ) // Hits left bound
		{
			eg_real t = -UpperRight.x/Dir.x; // This is dot product with plane
			assert( t > 0.f );
			if( t < SmallestT )
			{
				SmallestT = t;
			}
		}
		else // Hits top or bottom
		{

		}

		if( SmallestT < 1e28f )
		{
			Out = Dir * SmallestT;
		}

		return Out;
	}

	void SetWorldWidgetPose( const exTrackedIcon& TrackedIcon )
	{
		if( TrackedIcon.Widget && m_Mapping )
		{
			const eg_real SquareSize = ExHUDAutomap_AutomapTileSize.x;

			const eg_transform PlayerPose = GetGame() && GetGame()->GetPlayerEnt() ? GetGame()->GetPlayerEnt()->GetPose() : CT_Default;
			const eg_transform EntPose = TrackedIcon.Ent ? TrackedIcon.Ent->GetPose() : CT_Default;

			eg_vec3 IconOffset = EntPose.GetTranslation() - PlayerPose.GetTranslation();
			TrackedIcon.DistSqFromPlayer = IconOffset.LenSq();

			if( TrackedIcon.bClampToEdge )
			{
				if( 
					EG_IsBetween( IconOffset.x , -ExHUDAutomap_AutomapTileSize.x*2.f , ExHUDAutomap_AutomapTileSize.x*2.f )
					&&
					EG_IsBetween( IconOffset.z , -ExHUDAutomap_AutomapTileSize.y*2.f , ExHUDAutomap_AutomapTileSize.y*2.f )
				)
				{
					// Do nothing...
				}
				else
				{
					eg_vec2 EdgePos = GetEdgePos( IconOffset.GetNormalized().XZ_ToVec2() , ExHUDAutomap_AutomapTileSize*2.f );
					IconOffset = eg_vec3( EdgePos.x , 0.f , EdgePos.y );
				}
			}
			else
			{
				IconOffset.x = EG_Clamp( IconOffset.x , -ExHUDAutomap_AutomapTileSize.x*3.f , ExHUDAutomap_AutomapTileSize.x*3.f );
				IconOffset.z = EG_Clamp( IconOffset.z , -ExHUDAutomap_AutomapTileSize.y*3.f , ExHUDAutomap_AutomapTileSize.y*3.f );
			}

			const eg_aabb& PoseBounds = TrackedIcon.Widget->GetEntObj().GetBaseBounds();
			const eg_vec2 OriginalWidgetSize( PoseBounds.GetWidth() , PoseBounds.GetHeight() );

			eg_transform IconTr( CT_Default );

			const eg_real AdditionalScale = 2.f;

			eg_vec2 FinalOffset(0.f,0.f);

			FinalOffset.x += (IconOffset.x/m_MapInfo.MapMetaInfo.TileSize.x) * ExHUDAutomap_AutomapTileSize.x;
			FinalOffset.y += (IconOffset.z/m_MapInfo.MapMetaInfo.TileSize.y) * ExHUDAutomap_AutomapTileSize.y;

			IconTr.TranslateThis( FinalOffset.x , FinalOffset.y , 0.f );
			TrackedIcon.Widget->GetEntObj().SetScale( (SquareSize/(OriginalWidgetSize.x+1.f))*AdditionalScale );
			TrackedIcon.Widget->SetOffset( EGUiWidget::eg_offset_t::PRE , IconTr );
			TrackedIcon.Widget->SetVisible( m_Mapping->IsPoseRevealed( m_Mapping->WorldPoseToGridPos( EntPose ) ) );
		}
	}
};

EG_CLASS_DECL( ExHUDAutomap )
