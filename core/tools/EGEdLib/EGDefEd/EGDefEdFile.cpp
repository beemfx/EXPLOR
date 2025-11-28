// (c) 2017 Beem Media

#include "EGDefEdFile.h"
#include "EGEntDef.h"
#include "EGEntObj.h"
#include "EGRenderer.h"
#include "EGFileData.h"
#include "EGDefEdComponentsPanel.h"
#include "EGAudio.h"
#include "EGDefEd.h"
#include "EGToolsHelper.h"
#include "EGDefEdPreviewPanel.h"
#include "EGEngineConfig.h"
#include "EGEngineSerializeTypes.h"
#include "EGDefEd.h"
#include "EGWndPanelPropEditor.h"
#include "EGDefEdTimelineEditor.h"
#include "EGTimeline.h"
#include "EGCrcDb.h"
#include "EGDefEdPreview.h"
#include "EGSkelMeshComponent.h"
#include "EGEdUtility.h"
#include "EGEngineDataAssetLib.h"
#include "EGComponent.h"
#include "EGPhysBodyComponent.h"
#include "EGDefEdPropPanel.h"
#include "EGAssetPath.h"
#include "EGTextNodeComponent.h"
#include "EGEdLib.h"
#include "EGGlobalConfig.h"

EGDefEdFile EGDefEdFile::GlobalLayoutFile;

void EGDefEdFile::Init()
{
	// EGEntComponentDef::QueryComponents = StaticQueryComponents;
	EGComponent::StaticQueryForSubMeshes = StaticQuerySubMeshes;
	
	m_LastMousePos = eg_vec2( 0.f, 0.f );
	m_NextMousePos = eg_vec2( 0.f, 0.f );
	m_EntDef = new ( m_EntDefMem ) EGEntDef( CT_Clear );
	RefreshComponentList();
	InitPreviewObject();

	DebugConfig_DrawGridMasks.SetValue( true );

	if( EGDefEd_GetSettingsPanel() )
	{
		EGDefEd_GetSettingsPanel()->SetEditObject( m_EntDef->GetEditor() );
	}

	if( EGDefEd_GetEditorConfigPanel() )
	{
		EGDefEd_GetEditorConfigPanel()->SetEditObject( m_EntDef->GetConfigEditor() );
	}

	if( EGDefEd_GetTimelineEditor() )
	{
		EGDefEd_GetTimelineEditor()->Refresh();
	}
}

void EGDefEdFile::Deinit()
{
	DeinitPreviewObject();

	if( EGDefEd_GetSettingsPanel() )
	{
		EGDefEd_GetSettingsPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
	}

	if( EGDefEd_GetEditorConfigPanel() )
	{
		EGDefEd_GetEditorConfigPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
	}

	if( EGDefEd_GetTimelineEditor() )
	{
		EGDefEd_GetTimelineEditor()->Refresh();
	}

	assert( nullptr == m_PreviewEnt );
	m_Components.Clear();
	if( m_EntDef )
	{
		m_EntDef->~EGEntDef();
		m_EntDef = nullptr;
	}
}

EGComponent* EGDefEdFile::GetComponentByMousePos( eg_real x , eg_real y )
{
	unused( x , y );

	return nullptr;
}

void EGDefEdFile::Update( eg_real DeltaTime )
{
	EGAudio_BeginFrame();

	if( m_PreviewEnt )
	{
		m_PreviewEnt->Update( DeltaTime );
	}

	eg_bool bMouseMoved = m_LastMousePos != m_NextMousePos;

	if( bMouseMoved )
	{

	}

	EGAudio_EndFrame();
}

void EGDefEdFile::Draw()
{
	if( m_PreviewEnt && m_EntDef )
	{
		DebugConfig_DrawMenuButtons.SetValue( false );
		DebugConfig_DrawTextNodes.SetValue( m_bShowTextOutlines );
		GlobalConfig_IsUiLayoutTool.SetValue( true );

		m_PreviewEnt->SetIsLit( m_EntDef->m_EditorConfig.Lights.Len() > 0 );

		m_PreviewEnt->DrawForTool();

		DebugConfig_DrawMenuButtons.SetValue( false );
		DebugConfig_DrawTextNodes.SetValue( false );
		GlobalConfig_IsUiLayoutTool.SetValue( false );
	}
}

void EGDefEdFile::Open( eg_cpstr16 Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( Filename , MemFile );

	eg_string_big GamePath = *EGEdLib_GetFilenameAsGameDataFilename( EGString_ToMultibyte(Filename) );
	eg_asset_path::SetCurrentFile( GamePath );

	DeinitPreviewObject();

	if( m_EntDef )
	{
		m_EntDef->LoadForEditor( MemFile , GamePath );
		RefreshComponentList();
		if( EGDefEd_GetSettingsPanel() )
		{
			EGDefEd_GetSettingsPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
			EGDefEd_GetSettingsPanel()->SetEditObject( m_EntDef->GetEditor() );
		}
		if( EGDefEd_GetEditorConfigPanel() )
		{
			EGDefEd_GetEditorConfigPanel()->SetEditObject( reinterpret_cast<egRflEditor*>(nullptr) );
			EGDefEd_GetEditorConfigPanel()->SetEditObject( m_EntDef->GetConfigEditor() );
		}
		if( EGDefEd_GetTimelineEditor() )
		{
			EGDefEd_GetTimelineEditor()->Refresh();
		}
	}

	InitPreviewObject();

	if( EGDefEdComponentsPanel::GetPanel() )
	{
		EGDefEdComponentsPanel::GetPanel()->RefreshPanel( -1 );
	}

	EGDefEdPreview::Get().ResetCamera();

	if( m_EntDef && m_EntDef->m_EditorConfig.HasCameraPose )
	{
		EGDefEdPreview::Get().GetCameraController().Set( m_EntDef->m_EditorConfig.CameraYaw , m_EntDef->m_EditorConfig.CameraPitch , m_EntDef->m_EditorConfig.CameraPos );
	}
}

eg_bool EGDefEdFile::Save( eg_cpstr16 Filename )
{
	EGFileData MemFile( eg_file_data_init_t::HasOwnMemory );
	if( m_EntDef )
	{
		m_EntDef->SerializeToXml( MemFile );
	}

	return EGEdUtility_SaveFile( Filename, MemFile );
}

void EGDefEdFile::InitPreviewObject()
{
	assert( nullptr == m_PreviewEnt );
	m_PreviewEnt = new EGEntObj;
	if( m_PreviewEnt )
	{
		m_PreviewEnt->Init( m_EntDef );

		eg_bool bRunFirstEvent = false;

		if( bRunFirstEvent )
		{
			EGArray<eg_d_string> EventList;
			m_PreviewEnt->QueryEvents( EventList );
			if( EventList.IsValidIndex( 0 ) )
			{
				m_PreviewEnt->RunEvent( EGCrcDb::StringToCrc( *EventList[0] ) );
			}
		}
	}
}

void EGDefEdFile::DeinitPreviewObject()
{
	if( m_PreviewEnt )
	{
		m_PreviewEnt->Deinit();
		delete m_PreviewEnt;
		m_PreviewEnt = nullptr;
	}
}

void EGDefEdFile::ApplyComponentChanges( egRflEditor* RootComponent , const egRflEditor* ChangedProperty )
{
	unused( ChangedProperty );

	eg_bool bNeedsRebuild = false;
	if( RootComponent )
	{
		EGComponent* ChangedComp = reinterpret_cast<EGComponent*>(RootComponent->GetData());
		ChangedComp->OnPropChanged( *ChangedProperty , bNeedsRebuild );

		if( m_PreviewEnt )
		{
			const_cast<egRflEditor*>(ChangedProperty)->ExecutePostLoad( EGDefEd_GetCurrentGameFilename() , true );
			m_PreviewEnt->RefreshFromDefForTool( ChangedComp );
		}
	}

	if( ChangedProperty && EGString_Equals( ChangedProperty->GetVarName() , "m_Name" ) )
	{
		EGDefEdComponentsPanel* LayoutPanel = EGDefEdComponentsPanel::GetPanel();
		eg_int LayoutSelectedIndex = LayoutPanel ? LayoutPanel->GetSelectedIndex() : -1;
		LayoutPanel->RefreshPanel( LayoutSelectedIndex );
	}

	if( bNeedsRebuild )
	{
		FullRebuild();
		EGDefEdPropPanel::GetPanel()->PropEditorRebuild();
	}

	EGDefEd_SetDirty();
}

void EGDefEdFile::ApplySettingChanges( const egRflEditor* ChangedProperty , egEntDefEditNeeds& NeedsOut )
{
	if( m_EntDef )
	{
		m_EntDef->OnEditPropertyChanged( ChangedProperty , NeedsOut );

		if( NeedsOut.bRefreshPreivew )
		{
			FullRebuild();
		}
	}
}

void EGDefEdFile::PreviewTimeline( eg_string_crc CrcId )
{
	if( m_PreviewEnt )
	{
		if( CrcId.IsNotNull() )
		{
			m_PreviewEnt->StopAllEvents();
			m_PreviewEnt->RunEvent( CrcId );
		}
		else
		{
			m_PreviewEnt->ResetEvents();
		}
	}
}

void EGDefEdFile::ResetTimeline()
{
	if( m_PreviewEnt )
	{
		m_PreviewEnt->ResetEvents();
	}
}

void EGDefEdFile::StopTimeline()
{
	if( m_PreviewEnt )
	{
		m_PreviewEnt->StopAllEvents();
	}
}

eg_aabb EGDefEdFile::GetBounds() const
{
	eg_aabb Out( CT_Default );

	if( m_EntDef )
	{
		Out = m_EntDef->m_BaseBounds;
	}

	return Out;
}

EGComponent* EGDefEdFile::InsertNewComponent( EGClass* ComponentClass )
{
	if( nullptr == m_EntDef || nullptr == ComponentClass || !ComponentClass->IsA( &EGComponent::GetStaticClass() ) )
	{
		EGLogf( eg_log_t::Error , "Tried to insert an invalid component." );
		return nullptr;
	}

	auto IsNameTaken = [this]( eg_cpstr Name ) -> eg_bool
	{
		for( EGComponent* Def : m_Components )
		{
			if( Def && Def->GetName().EqualsI( Name ) )
			{
				return true;
			}
		}
		return false;
	};

	auto IsInsertionAllowed = [this,&ComponentClass]() -> eg_bool
	{
		for( EGComponent* Def : m_Components )
		{
			if( Def && Def->GetObjectClass() == ComponentClass )
			{
				return Def->CanHaveMoreThanOne();
			}
		}

		return true;
	};

	if( !IsInsertionAllowed() )
	{
		EGLogf( eg_log_t::General , "Only one %s can be inserted." , EGComponent::GetShortNameFromClass( ComponentClass ).String() );
		return nullptr;
	}

	EGComponent* NewComponent = EGNewObject<EGComponent>( ComponentClass , eg_mem_pool::Entity );
	if( NewComponent )
	{
		eg_string ClassShortName = EGComponent::GetShortNameFromClass( ComponentClass );

		eg_bool bFoundUniqueName = false;
		eg_string UniqueName = "NewComponent";
		for( eg_uint UniqueNameId=0; UniqueNameId<1000 && !bFoundUniqueName; UniqueNameId++ )
		{
			UniqueName = EGString_Format( "%s%u" , ClassShortName.String() , UniqueNameId );
			if( !IsNameTaken( UniqueName ) )
			{
				break;
			}
		}

		NewComponent->InitNewInEditor( UniqueName );
		m_EntDef->m_ComponentTree.InsertLast( NewComponent );
	}

	RefreshComponentList();
	DeinitPreviewObject();
	InitPreviewObject();
	EGDefEdComponentsPanel::GetPanel()->RefreshPanel( -1 );
	EGDefEd_SetDirty();

	return NewComponent;
}

void EGDefEdFile::MoveObject( EGComponent* ObjToMove , EGComponent* ObjToMoveBefore , EGComponent* ObjParent )
{
	if( nullptr == m_EntDef || nullptr == ObjToMove )
	{
		return;
	}

	// Make sure the ObjToMoveBefore is not a child of ObjToMove
	for( EGComponent* MoveBeforeParent = ObjToMoveBefore; MoveBeforeParent != nullptr; MoveBeforeParent = MoveBeforeParent->GetParent() )
	{
		if( MoveBeforeParent == ObjToMove )
		{
			return;
		}
	}

	// Make sure the object to set as parent is not a child of the object
	for( EGComponent* ParentChild = ObjParent; ParentChild != nullptr; ParentChild = ParentChild->GetParent() )
	{
		if( ParentChild == ObjToMove )
		{
			return;
		}
	}

	if( ObjToMove->GetParent() )
	{
		ObjToMove->GetParent()->GetChildren().Remove( ObjToMove );
	}
	else
	{
		m_EntDef->m_ComponentTree.Remove( ObjToMove );
	}

	ObjToMove->SetParent( nullptr );

	if( ObjParent )
	{
		ObjParent->GetChildren().InsertLast( ObjToMove );
		ObjToMove->SetParent( ObjParent );
	}
	else
	{
		if( ObjToMoveBefore )
		{
			if( ObjToMoveBefore->GetParent() )
			{
				ObjToMoveBefore->GetParent()->GetChildren().InsertBefore( ObjToMoveBefore , ObjToMove );
				ObjToMove->SetParent( ObjToMoveBefore->GetParent() );
			}
			else
			{
				m_EntDef->m_ComponentTree.InsertBefore( ObjToMoveBefore , ObjToMove );
			}
		}
		else
		{
			m_EntDef->m_ComponentTree.InsertLast( ObjToMove );
		}
	}

	// Now that the move is complete notify the other panels that need to know
	// about it.

	RefreshComponentList();
	DeinitPreviewObject();
	InitPreviewObject();

	eg_int NewIndex = -1;
	for( eg_size_t i = 0; i < m_Components.Len(); i++ )
	{
		if( m_Components[i] == ObjToMove )
		{
			NewIndex = static_cast<eg_int>( i );
			break;
		}
	}

	EGDefEdComponentsPanel::GetPanel()->RefreshPanel( NewIndex );
	EGDefEd_SetDirty();
}

void EGDefEdFile::DeleteObject( EGComponent* ObjToMove )
{
	if( nullptr == m_EntDef || nullptr == ObjToMove )
	{
		return;
	}

	if( ObjToMove->GetParent() )
	{
		ObjToMove->GetParent()->GetChildren().Remove( ObjToMove );
	}
	else
	{
		m_EntDef->m_ComponentTree.Remove( ObjToMove );
	}

	eg_bool bPreserveChildren = false;

	if( bPreserveChildren )
	{
		while( ObjToMove->GetChildren().HasItems() )
		{
			EGComponent* Def = ObjToMove->GetChildren().GetFirst();
			ObjToMove->GetChildren().Remove( Def );
			Def->SetParent( nullptr );
			m_EntDef->m_ComponentTree.InsertLast( Def );
		}
	}

	EG_SafeRelease( ObjToMove );

	RefreshComponentList();
	DeinitPreviewObject();
	InitPreviewObject();
	EGDefEdComponentsPanel::GetPanel()->RefreshPanel( -1 );
	EGDefEd_SetDirty();
}

void EGDefEdFile::DeleteTimeline( eg_size_t TimelineIndex )
{
	DeinitPreviewObject();

	if( m_EntDef && m_EntDef->m_Timelines.IsValidIndex( TimelineIndex ) )
	{
		EGTimeline* FoundTimeline = m_EntDef->m_Timelines[TimelineIndex];
		m_EntDef->m_Timelines.DeleteByIndex( TimelineIndex );
		EG_SafeRelease( FoundTimeline );
		EGDefEd_SetDirty();
	}

	InitPreviewObject();
}

void EGDefEdFile::CreateNewTimeline( eg_size_t InsertAtIndex )
{
	DeinitPreviewObject();

	auto GenerateNewTimelineName = [this]() -> eg_string_crc
	{
		for( eg_int i=0; i<10000; i++ )
		{
			eg_string_small Name = EGString_Format( "NewEvent%u" , i );
			eg_string_crc CrcName = EGCrcDb::StringToCrc( Name );
			if( !IsTimelineEventNameTaken( CrcName ) )
			{
				return CrcName;
			}
		}
		return EGCrcDb::StringToCrc( "NewEventX" );
	};

	if( m_EntDef )
	{
		EGTimeline* NewTimeline = EGNewObject<EGTimeline>( eg_mem_pool::System );
		if( NewTimeline )
		{
			NewTimeline->SetId( GenerateNewTimelineName() );
			NewTimeline->InsertKeyframe( egTimelineKeyframe() );
			NewTimeline->CompileScript();
			m_EntDef->m_Timelines.InsertAt( InsertAtIndex , NewTimeline );
			EGDefEd_SetDirty();
		}
	}

	InitPreviewObject();
}

eg_int EGDefEdFile::MoveTimelineEvent( eg_size_t TimelineIndex , eg_int DeltaMove )
{
	eg_int NewIndex = EG_To<eg_int>(TimelineIndex);
	
	if( m_EntDef && m_EntDef->m_Timelines.IsValidIndex( TimelineIndex ) )
	{
		NewIndex = EG_Clamp<eg_int>( NewIndex + DeltaMove , 0 , m_EntDef->m_Timelines.LenAs<eg_int>()-1 );
		if( NewIndex != TimelineIndex && m_EntDef->m_Timelines.IsValidIndex( NewIndex ) )
		{
			EGTimeline* FoundTimeline = m_EntDef->m_Timelines[TimelineIndex];
			m_EntDef->m_Timelines.DeleteByIndex( TimelineIndex );
			m_EntDef->m_Timelines.InsertAt( NewIndex , FoundTimeline );
			EGDefEd_SetDirty();
		}
		else
		{
			assert( NewIndex == TimelineIndex ); // Should be a valid index.
		}
	}

	return NewIndex;
}

eg_bool EGDefEdFile::IsTimelineEventNameTaken( eg_string_crc Name ) const
{
	if( m_EntDef )
	{
		for( EGTimeline* Timeline : m_EntDef->m_Timelines )
		{
			if( Timeline->GetId() == Name )
			{
				return true;
			}
		}
	}

	return false;
}

void EGDefEdFile::SetBoundsTo( eg_setbound_t BoundsToType )
{
	if( m_EntDef )
	{
		eg_bool bGotFirstBounds = false;
		eg_aabb FinalBounds( CT_Default );

		auto AddBounds = [&bGotFirstBounds,&FinalBounds]( const eg_aabb& NewBounds )
		{
			if( !bGotFirstBounds )
			{
				bGotFirstBounds = true;
				FinalBounds = NewBounds;
			}
			else
			{
				FinalBounds.AddBox( NewBounds );
			}
		};

		eg_bool bWantSkelMesh = BoundsToType == eg_setbound_t::ToMeshBounds || BoundsToType == eg_setbound_t::ToMeshAndPhysicsBounds;
		eg_bool bWantPhys = BoundsToType == eg_setbound_t::ToPhysicsBounds || BoundsToType == eg_setbound_t::ToMeshAndPhysicsBounds;

		for( const EGComponent* Comp : m_Components )
		{
			if( bWantPhys )
			{
				const EGPhysBodyComponent* AsPhys = EGCast<EGPhysBodyComponent>( Comp );
				if( AsPhys )
				{
					for( const eg_phys_shape& Shape : AsPhys->GetShapes() )
					{
						egPhysShapeDef PhysShape = Shape;
						AddBounds( PhysShape.Shape.GetAABB() );
					}
				}
			}
		}

		if( bWantSkelMesh )
		{
			if( m_PreviewEnt )
			{
				AddBounds( m_PreviewEnt->GetMeshComponentBounds() );
			}
		}

		m_EntDef->m_BaseBounds = FinalBounds;

		EGWndPanelPropEditor* SettingsPanel = EGDefEd_GetSettingsPanel();
		if( SettingsPanel )
		{
			SettingsPanel->RefreshAll();
		}

		FullRebuild();
	}
}

void EGDefEdFile::CreateTextComponents()
{
	struct egTodo
	{
		EGSkelMeshComponent* ParentDef = nullptr;
		EGMeshBase::egTextNode TextNode;
		eg_string_small ParentBoneId = CT_Clear;
		eg_transform ParentBoneBasePose = CT_Default;
	};

	EGArray<egTodo> TodoList;

	auto GetComponentDef = [this]( const EGComponent* Component ) -> EGComponent*
	{
		for( EGComponent* Def : m_Components )
		{
			if( Component->GetDef() == Def )
			{
				return Def;
			}
		}

		return nullptr;
	};

	auto HandleComponent = [this,&GetComponentDef,&TodoList]( const EGComponent* Component ) -> void
	{
		const EGSkelMeshComponent* SkelMesh = EGCast<EGSkelMeshComponent>( Component );
		if( SkelMesh )
		{
			EGArray<const EGMeshBase::egTextNode*> TextNodes;
			SkelMesh->GetTextNodes( TextNodes );

			EGComponent* ParentDef = GetComponentDef( Component );

			if( ParentDef )
			{
				for( const EGMeshBase::egTextNode* Node : TextNodes )
				{
					if( Node )
					{
						egTodo NewTodo;
						NewTodo.ParentDef = EGCast<EGSkelMeshComponent>(ParentDef);
						NewTodo.TextNode = *Node;
						if( Node->Bone > 0 )
						{
							NewTodo.ParentBoneId = SkelMesh->GetTextNodeBoneId( Node->Bone );
							NewTodo.ParentBoneBasePose = SkelMesh->GetTextNodeBonePose( Node->Bone );
						}
						TodoList.Append( NewTodo );
					}
				}
			}
		}
	};

	if( m_PreviewEnt )
	{
		const EGEntComponentTree& ComponentTree = m_PreviewEnt->GetComponentTree();
		ComponentTree.IterateComponents( HandleComponent );
	}

	for( egTodo& Todo : TodoList )
	{
		if( Todo.ParentDef )
		{
			EGTextNodeComponent* NewTextNode = EGCast<EGTextNodeComponent>(InsertNewComponent( &EGTextNodeComponent::GetStaticClass() ));
			NewTextNode->m_Width = Todo.TextNode.Width;
			NewTextNode->m_Height = Todo.TextNode.Height;
			NewTextNode->m_NumLines = EG_Max<eg_uint>( 1 , static_cast<eg_uint>(Todo.TextNode.Height/Todo.TextNode.LineHeight) );
			NewTextNode->m_Font.Path = EGCrcDb::CrcToString( Todo.TextNode.Font );
			NewTextNode->m_Color = Todo.TextNode.Color;
			NewTextNode->m_Alignment = static_cast<eg_text_align_ed>(Todo.TextNode.Justify);
			NewTextNode->m_BasePose = Todo.TextNode.Pose;
			if( Todo.ParentBoneId.Len() > 0 )
			{
				NewTextNode->m_ParentJointId = EGCrcDb::StringToCrc(Todo.ParentBoneId);
				eg_transform BonePose = Todo.ParentBoneBasePose;
				eg_transform BonePoseInv = BonePose.GetInverse();
				NewTextNode->m_BasePose = NewTextNode->m_BasePose * BonePoseInv;
			}
			NewTextNode->m_LocText = EGCrcDb::StringToCrc(EGString_Format( "%sText" , *NewTextNode->GetName() ));
			NewTextNode->m_LocText_enus = "";

			MoveObject( NewTextNode , nullptr , Todo.ParentDef );
		}
	}

	FullRebuild();
}

eg_size_t EGDefEdFile::GetNumObjects()
{
	return m_Components.Len();
}

EGComponent* EGDefEdFile::GetComponentInfoByIndex( eg_size_t Index )
{
	return m_Components.IsValidIndex( Index ) ? m_Components[Index] : nullptr;
}

void EGDefEdFile::FullRebuild()
{
	EGDefEdComponentsPanel* LayoutPanel = EGDefEdComponentsPanel::GetPanel();
	eg_int LayoutSelectedIndex = LayoutPanel ? LayoutPanel->GetSelectedIndex() : -1;

	// m_MenuLayout.OnToolFullRebuild();

	DeinitPreviewObject();
	InitPreviewObject();

	RefreshComponentList();

	if( LayoutPanel )
	{
		LayoutPanel->RefreshPanel( LayoutSelectedIndex );
	}
}

void EGDefEdFile::ConvertToBWNoShadow()
{
	if( m_EntDef )
	{
		for( EGComponent* Comp : m_EntDef->m_ComponentTree )
		{
			if( EGTextNodeComponent* AsTextNode = EGCast<EGTextNodeComponent>( Comp ) )
			{
				AsTextNode->ConvertToBWNoShadow();
			}
		}
	}

	FullRebuild();
}

void EGDefEdFile::RefreshComponentList()
{
	m_Components.Clear();
	if( m_EntDef )
	{
		m_EntDef->GetComponents( m_Components );
	}
}

void EGDefEdFile::StaticQueryComponents( const EGComponent* ComponentDef , EGArray<eg_string_crc>& Out )
{
	EGDefEdFile& _this = Get();

	unused( ComponentDef );

	for( EGComponent* Def : _this.m_Components )
	{
		if( Def )
		{
			Out.Append( EGCrcDb::StringToCrc( *Def->GetName() ) );
		}
	}
}

void EGDefEdFile::StaticQuerySubMeshes( const EGComponent* ComponentDef , EGArray<eg_d_string>& Out )
{
	EGDefEdFile& _this = Get();

	if( _this.m_PreviewEnt && ComponentDef )
	{
		EGMeshComponent* SkelMesh = _this.m_PreviewEnt->GetComponentTree().GetComponentById<EGMeshComponent>( EGCrcDb::StringToCrc(*ComponentDef->GetName()) );
		if( SkelMesh )
		{
			SkelMesh->QuerySubMeshes( Out );
		}
	}
}

void EGDefEdFile::QueryTimelines( EGArray<eg_d_string>& Out )
{
	if( m_EntDef )
	{
		m_EntDef->QueryEventList( Out );
	}
}

EGTimeline* EGDefEdFile::GetTimelineById( eg_string_crc TimelineId )
{
	if( TimelineId.IsNotNull() && m_EntDef )
	{
		for( EGTimeline* Timeline : m_EntDef->m_Timelines )
		{
			if( Timeline && Timeline->GetId() == TimelineId )
			{
				return Timeline;
			}
		}
	}

	return nullptr;
}

void EGDefEdFile::MoveComponentToPose( EGComponent* Component , const eg_transform& NewPose )
{
	if( Component )
	{	
		if( egRflEditor* CompEd = Component->GetEditor() )
		{
			if( egRflEditor* BasePoseEditor = CompEd->GetChildPtr( "m_BasePose" ) )
			{
				if( eg_transform* BasePose = reinterpret_cast<eg_transform*>(BasePoseEditor->GetData()) )
				{
					*BasePose = NewPose;
					FullRebuild();
					EGDefEdPropPanel::GetPanel()->PropEditorRebuild();
				}
			}
		}
	}
}
