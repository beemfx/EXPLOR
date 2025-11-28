// (c) 2017 Beem Media

#include "ExCombatMenuOpponentsWidget.h"
#include "ExBeastiary.h"
#include "EGAudio.h"
#include "EGAudioList.h"
#include "EGTextNodeComponent.h"
#include "EGSkelMeshComponent.h"
#include "EGDebugText.h"
#include "ExUiSounds.h"
#include "EGDebugText.h"
#include "EGCamera2.h"
#include "EGMenu.h"

EG_CLASS_DECL( ExCombatMenuOpponentsWidget )

const eg_real ExCombatMenuOpponentsWidget::CAMERA_MOVE_TIME = .25f; // How long it takes the camera to move to a new position.
const eg_real ExCombatMenuOpponentsWidget::CAMERA_RESTORE_TIME = .3f; // How long the camera lingers on something before being restored.
const eg_real ExCombatMenuOpponentsWidget::TEMP_ANIM_TIME = 1.f; // How long the monster object of a temporary animation exists before being purged.
const eg_real ExCombatMenuOpponentsWidget::MONSTER_SPACING = 50.f; // How far apart monsters are spaced.

ExCombatMenuOpponentsWidget::exMonsterObj* ExCombatMenuOpponentsWidget::GetMonsterObj( const ExCombatant* Combatant )
{
	for( exMonsterObj& MonsterObj : m_FrontRowMonsters )
	{
		assert( MonsterObj.Type != ex_monster_t::TempAnimation );
		if( MonsterObj.Type != ex_monster_t::Empty && MonsterObj.Combatant == Combatant )
		{
			return &MonsterObj;
		}
	}

	for( exMonsterObj& MonsterObj : m_TempMonsters )
	{
		assert( MonsterObj.Type == ex_monster_t::TempAnimation || MonsterObj.Type == ex_monster_t::Empty );
		if( MonsterObj.Type != ex_monster_t::Empty &&  MonsterObj.Combatant == Combatant )
		{
			return &MonsterObj;
		}
	}

	return nullptr;
};

ExCombatMenuOpponentsWidget::exMonsterObj* ExCombatMenuOpponentsWidget::SpawnMonsterObj( const ExCombatant* Combatant , eg_bool bSpawnAsTemp )
{
	ExMonsterArray& ArrayToUse = bSpawnAsTemp ? m_TempMonsters : m_FrontRowMonsters;

	auto InitObj = [this,&Combatant,&bSpawnAsTemp]( exMonsterObj& InObj , eg_int DisplaySlotIndex ) -> void
	{
		eg_transform StartingOffset = CT_Default;
		const eg_transform SlotPose = GetSlotPose( DisplaySlotIndex , bSpawnAsTemp , &StartingOffset );

		InObj.Combatant = Combatant;
		InObj.HealthTween.SetValue( Combatant ? Combatant->GetHP() : 0 );
		InObj.Type = bSpawnAsTemp ? ex_monster_t::TempAnimation : ex_monster_t::Visible;
		InObj.SetPose( SlotPose*StartingOffset );
		InObj.MoveTo( SlotPose );
		if( bSpawnAsTemp )
		{
			InObj.TempAnimLife = TEMP_ANIM_TIME;
		}

		const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( Combatant->GetBeastiaryId() );
		if( m_MonsterMap[Combatant->GetBeastiaryId()] )
		{
			if( nullptr == InObj.Obj )
			{
				InObj.Obj = new EGEntObj;
			}
			if( InObj.Obj )
			{
				InObj.Obj->InitClone( *m_MonsterMap[Combatant->GetBeastiaryId()] );
				InObj.Obj->SetTexture( eg_crc("Sprite") , eg_crc("Mesh") , *BeastInfo.ImagePath.FullPath );
			}
			InObj.PlayAnimation( ex_monster_anim::Idle );
		}
	};

	for( eg_int i=0; i<ArrayToUse.LenAs<eg_int>(); i++ )
	{
		exMonsterObj& Obj = ArrayToUse[i];

		// We can spawn in a dead or empty slot.
		if( Obj.Type == ex_monster_t::Empty || Obj.Type == ex_monster_t::Dead )
		{
			InitObj( Obj , i );
			return &Obj;
		}
	}

	// If we want a temp monster and there is no slot available create a new one.
	if( bSpawnAsTemp )
	{
		eg_int NewTempIndex = m_TempMonsters.LenAs<eg_int>();
		m_TempMonsters.Resize( NewTempIndex+1 );
		if( m_TempMonsters.IsValidIndex( NewTempIndex ) )
		{
			exMonsterObj& Obj = m_TempMonsters[NewTempIndex];
			InitObj( Obj , NewTempIndex );
			return &Obj;
		}

	}

	if( !bSpawnAsTemp )
	{
		assert( false ); // The active monster array should have been resized to the front lines size.
	}

	return nullptr;
}

eg_transform ExCombatMenuOpponentsWidget::GetSlotPose( eg_int SlotIndex , eg_bool bTempSlot , eg_transform* StartingOffsetOut )
{
	const eg_vec2 MonsterOffset = eg_vec2( MONSTER_SPACING , 0.f );
	const eg_real DropInOffset = 160.f;

	auto OffsetFn = []( eg_int SlotIndexIn ) -> eg_real
	{
		if( 0 == (SlotIndexIn%2) )
		{
			// Even number
			return -(SlotIndexIn * .5f);
		}
		else
		{
			// Odd number
			return SlotIndexIn*.5f + .5f;
		}

		return 0.f;
	};
	
	eg_transform Out = CT_Default;
	if( bTempSlot )
	{
		eg_int AdjIndex = m_FrontRowMonsters.LenAs<eg_int>() + m_TempAnimPoseIndex;
		Out.TranslateThis( MonsterOffset.x * OffsetFn( AdjIndex ) , MonsterOffset.y * OffsetFn( AdjIndex ) , 0.f );
		if( StartingOffsetOut )
		{
			*StartingOffsetOut = CT_Default;
		}
		m_TempAnimPoseIndex = (m_TempAnimPoseIndex+1)%2;
	}
	else
	{
		Out.TranslateThis( MonsterOffset.x * OffsetFn( SlotIndex ) , MonsterOffset.y * OffsetFn( SlotIndex ) , 0.f );
		if( StartingOffsetOut )
		{
			*StartingOffsetOut = eg_transform::BuildTranslation( 0.f , DropInOffset , 0.f );
		}
	}


	return Out;
}

void ExCombatMenuOpponentsWidget::PlayAnimation( const exCombatAnimInfo& AnimInfo )
{		
	// Monster as defender
	{
		exMonsterObj* MonsterObj = GetMonsterObj( AnimInfo.Defender );
		if( MonsterObj )
		{
			eg_bool bCenterActiveCombatMonster = true;

			if( bCenterActiveCombatMonster || MonsterObj->Type == ex_monster_t::TempAnimation )
			{
				MoveCameraToMonster( MonsterObj , true );
			}
			else
			{
				RestoreCamera();
			}
		}
		else
		{
			if( m_CombatantList.Contains( AnimInfo.Defender ) )
			{
				MonsterObj = SpawnMonsterObj( AnimInfo.Defender , true );
				if( MonsterObj )
				{
					assert( MonsterObj->Type == ex_monster_t::TempAnimation );
					MoveCameraToMonster( MonsterObj , true );
				}
			}
		}

		if( MonsterObj )
		{
			const eg_bool bPlayDamageNumber = AnimInfo.IsDamageNumberType() && AnimInfo.Damage != 0;

			if( MonsterObj->Type == ex_monster_t::TempAnimation && AnimInfo.Type == ex_combat_anim::AnyDeath )
			{
				MonsterObj->PlayAnimation( ex_monster_anim::Die );
				MonsterObj->Type = ex_monster_t::Dead;
			}

			eg_transform Pose = MonsterObj->GetPose();
			Pose.TranslateThis( 0.f , 25.f , 0.f );
			eg_color32 Color = AnimInfo.Damage > 0 || AnimInfo.Type == ex_combat_anim::AnyDeath ? eg_color32(181,26,0) : eg_color32(255,255,255);
			if( AnimInfo.Type == ex_combat_anim::SpellAttack )
			{
				Color = eg_color32(227,36,0);
			}
			if( MonsterObj->Type != ex_monster_t::Dead )
			{
				MonsterObj->PlayAnimation( ex_monster_anim::Dodge );
			}
			PlayCombatTeamWidgetBaseAnimation( AnimInfo.Type , Pose  , .75f , Color );
			if( bPlayDamageNumber ) // Don't show damage number for death since that comes in as a regular and it would spawn two damage nubmers.
			{
				PlayDamageNumber( AnimInfo.Damage , Pose , 1.f , AnimInfo.Type == ex_combat_anim::SpellBonus );
			}
			if( AnimInfo.Type == ex_combat_anim::Burgled && AnimInfo.Damage > 0 )
			{
				eg_transform BurglePose = Pose;
				BurglePose.TranslateThis( 0.f , -10.f , 0.f );
				PlayBurgled( AnimInfo.Damage , BurglePose , .8f );
			}
			if( AnimInfo.Type == ex_combat_anim::CriticalHit && AnimInfo.Damage > 0 )
			{
				eg_transform AnimPose = Pose;
				AnimPose.TranslateThis( 0.f , 10.f , 0.f );
				PlayCriticalHit( AnimInfo.Damage , AnimPose , 1.1f );
			}
			if( AnimInfo.Type == ex_combat_anim::Resist )
			{
				eg_transform ResistPose = Pose;
				ResistPose.TranslateThis( 0.f , 10.f , 0.f );
				PlayResisted( static_cast<ex_attr_t>(AnimInfo.Damage) , ResistPose , 1.1f );
			}
		}
	}

	// Handle cases where monster is the attacker
	{
		exMonsterObj* MonsterObj = GetMonsterObj( AnimInfo.Attacker );
		if( MonsterObj )
		{
			eg_bool bCenterActiveCombatMonster = true;

			if( bCenterActiveCombatMonster || MonsterObj->Type == ex_monster_t::TempAnimation )
			{
				MoveCameraToMonster( MonsterObj , true );
			}
			else
			{
				RestoreCamera();
			}

			MonsterObj->PlayAnimation( ex_monster_anim::Attack );
		}
		else if( m_CombatantList.Contains( AnimInfo.Attacker ) )
		{
			MonsterObj = SpawnMonsterObj( AnimInfo.Attacker , true );
			if( MonsterObj )
			{
				assert( MonsterObj->Type == ex_monster_t::TempAnimation );
				MonsterObj->PlayAnimation( ex_monster_anim::Attack );
				MoveCameraToMonster( MonsterObj , true );
			}
		}
	}
}

void ExCombatMenuOpponentsWidget::Init( EGMenu* InOwner, const egUiWidgetInfo* InInfo )
{
	Super::Init( InOwner , InInfo );

	m_CameraPose.SetMoveDuration( CAMERA_MOVE_TIME );
	m_CameraPose.SetRestoreWaitDuration( CAMERA_RESTORE_TIME );
}

void ExCombatMenuOpponentsWidget::Update( eg_real DeltaTime, eg_real AspectRatio )
{
	m_LastAspectRatio = AspectRatio;

	m_CameraPose.Update( DeltaTime );
	m_CenterPose.Update( DeltaTime );
	
	auto UpdateMonsterList = [&DeltaTime]( ExMonsterArray& Array ) -> void
	{
		for( exMonsterObj& MonsterObj : Array )
		{
			MonsterObj.Update( DeltaTime );
			if( MonsterObj.Type == ex_monster_t::TempAnimation )
			{
				MonsterObj.TempAnimLife -= DeltaTime;
				if( MonsterObj.TempAnimLife <= 0.f )
				{
					MonsterObj.Type = ex_monster_t::Empty;
					if( MonsterObj.Obj )
					{
						MonsterObj.Obj->Deinit();
					}
				}
			}
			else if( MonsterObj.Type == ex_monster_t::Dead )
			{
				MonsterObj.Type = ex_monster_t::Empty;
				if( MonsterObj.Obj )
				{
					MonsterObj.Obj->Deinit();
				}
			}
		}
	};

	UpdateMonsterList( m_FrontRowMonsters );
	UpdateMonsterList( m_TempMonsters );

	UpdateCombatTeamWidgetBase( DeltaTime );
}

void ExCombatMenuOpponentsWidget::Draw( eg_real AspectRatio )
{
	eg_transform BasePose = m_CenterPose.GetPose() * m_CameraPose.GetPose() * GetFullPose( AspectRatio );

	auto DrawMonsterList = [this,&BasePose]( ExMonsterArray& Array ) -> void
	{
		for( exMonsterObj& MonsterObj : Array )
		{
			eg_transform FinalPose = MonsterObj.GetPose() * BasePose;
			if( MonsterObj.Type != ex_monster_t::Empty && MonsterObj.Obj )
			{
				MonsterObj.Obj->SetDrawInfo( FinalPose , m_ScaleVector , false );
				MonsterObj.Obj->Draw();
			}
		}
	};

	DrawMonsterList( m_FrontRowMonsters );
	DrawMonsterList( m_TempMonsters );

	DrawCombatTeamWidgetBase( BasePose );
}

void ExCombatMenuOpponentsWidget::OnConstruct()
{
	Super::OnConstruct();

	InitCombatTeamWidgetBase();
}

void ExCombatMenuOpponentsWidget::OnDestruct()
{
	Super::OnDestruct();

	m_FrontRowMonsters.Clear();
	m_TempMonsters.Clear();

	DeintCombatTeamWidgetBase();
	DestroyMonsterMap();
}

void ExCombatMenuOpponentsWidget::SetEnabled( eg_bool IsActive )
{
	Super::SetEnabled( IsActive );

	RefreshTargetted( nullptr );

	if( IsEnabled() )
	{
		SelectInitialTarget();
		exMonsterObj* TargetedObj = GetMonsterAtTargetOffset( m_TargetOffset );
		if( TargetedObj )
		{
			ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
			MoveCameraToMonster( TargetedObj , false );
		}
		else
		{
			RestoreCamera();
		}
		RefreshTargetted( TargetedObj );
	}
}

eg_bool ExCombatMenuOpponentsWidget::HandleInput( eg_menuinput_t Event , const eg_vec2& WidgetHitPoint , eg_bool bFromMouse )
{
	unused( bFromMouse , WidgetHitPoint );
	
	if( IsEnabled() )
	{
		if( Event == eg_menuinput_t::BUTTON_RIGHT )
		{
			MoveTarget( 1 );
			return true;
		}
		else if( Event == eg_menuinput_t::BUTTON_LEFT )
		{
			MoveTarget( -1 );
			return true;
		}
		else if( Event == eg_menuinput_t::BUTTON_PRIMARY )
		{
			exMonsterObj* TargetedObj = GetMonsterAtTargetOffset( m_TargetOffset );
			if( TargetedObj && TargetedObj->Combatant )
			{
				m_LastAttacked = bFromMouse ? nullptr : TargetedObj->Combatant;
				TargetChosenDelegate.ExecuteIfBound( TargetedObj->Combatant );
			}
			else
			{
				assert( false );
			}
			return true;
		}
	}

	return false;
}

eg_bool ExCombatMenuOpponentsWidget::OnMouseMovedOver( const eg_vec2& WidgetHitPoint , eg_bool bIsMouseCaptured )
{
	unused( bIsMouseCaptured );

	HandleMouseHoveredOn( WidgetHitPoint );
	
	return true;
}

eg_bool ExCombatMenuOpponentsWidget::OnMouseMovedOn( const eg_vec2& WidgetHitPoint )
{
	HandleMouseHoveredOn( WidgetHitPoint );
	
	return true;
}

void ExCombatMenuOpponentsWidget::OnMouseMovedOff()
{
	
}

void ExCombatMenuOpponentsWidget::HandleMouseHoveredOn( const eg_vec2& WidgetHitPoint )
{
	const eg_int HitTargetOffset = GetTargetOffsetAtHitPoint( WidgetHitPoint );

	if( nullptr == m_Owner || nullptr == m_Owner->GetMouseCapture() )
	{
		if( HitTargetOffset != m_TargetOffset && nullptr != GetMonsterAtTargetOffset( HitTargetOffset ) )
		{
			RestoreCamera();
			MoveTarget( HitTargetOffset - m_TargetOffset , true );
		}
	}
}

eg_int ExCombatMenuOpponentsWidget::GetTargetOffsetAtHitPoint( const eg_vec2& WidgetHitPoint ) const
{
	// We want the widget hit point in menu space...
	// Get the hit point in projection space
	eg_real HitX = EGMath_GetMappedRangeValue( WidgetHitPoint.x , eg_vec2(0.f,1.f) , eg_vec2(-m_LastAspectRatio,m_LastAspectRatio) );
	// Transform to view space (this is the camera size, we aren't grabbing the actual camera data but we know it is -100 to 100)
	// If the menu camera gets changed this will be broken...
	HitX *= 100.f;

	const eg_real CameraX = m_CameraPose.GetPose().GetPosition().x;
	const eg_real CenterX = m_CenterPose.GetPose().GetPosition().x;

	// Next transform by the camera and center (we can do fast mast because we know it only moves left or right)
	HitX -= (CameraX + CenterX);

	// From here we can determine what slot was hit by the size
	const eg_int HitTargetOffset = EGMath_ceil( (HitX-MONSTER_SPACING*.5f)/MONSTER_SPACING );

	// EGLogToScreen::Get().Log( this , __LINE__ , 1.f , *EGSFormat8( "Opponents Widget Hit: {0} -> {1} (Target Offset: {2})" , WidgetHitPoint.x , HitX , HitTargetOffset ) );

	return HitTargetOffset;
}

eg_aabb ExCombatMenuOpponentsWidget::GetBoundsInMouseSpace( eg_real AspectRatio , const eg_mat& MatView , const eg_mat& MatProj ) const
{ 
	const eg_aabb MeshWidgetBounds = Super::GetBoundsInMouseSpace( AspectRatio , MatView , MatProj );
	eg_aabb Out = MeshWidgetBounds;
	// Widget occupies full screen width:
	Out.Min.x = -AspectRatio;
	Out.Max.x = AspectRatio;
	return Out;
}

eg_bool ExCombatMenuOpponentsWidget::OnMousePressed( const eg_vec2& WidgetHitPoint )
{
	HandleMouseHoveredOn( WidgetHitPoint );

	m_MouseCaptureTarget = GetTargetOffsetAtHitPoint( WidgetHitPoint );

	if( m_MouseCaptureTarget == m_TargetOffset )
	{
		if( m_Owner )
		{
			m_Owner->BeginMouseCapture( this );
		}
	}

	return true;
}

eg_bool ExCombatMenuOpponentsWidget::OnMouseReleased( const eg_vec2& WidgetHitPoint , EGUiWidget* WidgetReleasedOn )
{
	const eg_int HitTarget = GetTargetOffsetAtHitPoint( WidgetHitPoint );

	if( m_Owner && m_Owner->GetMouseCapture() == this )
	{
		m_Owner->EndMouseCapture();
		if( this == WidgetReleasedOn && HitTarget == m_MouseCaptureTarget && m_MouseCaptureTarget == m_TargetOffset )
		{
			HandleInput( eg_menuinput_t::BUTTON_PRIMARY , WidgetHitPoint , true );
		}
	}
	return true;
}

void ExCombatMenuOpponentsWidget::SelectInitialTarget()
{
	m_TargetOffset = 0;

	const eg_int FrontRowSize = m_FrontRowMonsters.LenAs<eg_int>();
	const eg_bool bIsEven = 0 == (FrontRowSize%2);
	const eg_int SideOffset = FrontRowSize/2;
	const eg_ivec2 CheckRange( -SideOffset + (bIsEven ? 1 : 0)  , SideOffset ); // Even though ExGame is always 5 opponents, we'll account for an even number just in case.

	// Theoretically the widget should have all the monsters next to each other so this algorithm should
	// work. Basically every time we find an even number of monsters we offset the target by one.
	eg_int MonstersFound = 0;
	for( eg_int i = CheckRange.x; i <= CheckRange.y; i++ )
	{
		exMonsterObj* MonsterAtTarget = GetMonsterAtTargetOffset( i );

		if( MonsterAtTarget )
		{
			if( MonsterAtTarget->Combatant == m_LastAttacked )
			{
				// If this was the last monster attacked, choose it.
				m_TargetOffset = i;
				break;
			}

			// Every even number of monsters advance the target offset by one
			// this will always be in the middle.
			const eg_bool bWasEven = 0 == (MonstersFound%2);
			MonstersFound++;
			if( MonstersFound == 1 )
			{
				m_TargetOffset = i;
			}
			else if( bWasEven )
			{
				m_TargetOffset++;
			}
		}
	}

	// Fallback if we didnt' find anything (should never happen );
	if( nullptr == GetMonsterAtTargetOffset( m_TargetOffset ) )
	{
		assert( false );
		m_TargetOffset = 0;
	}
}

eg_size_t ExCombatMenuOpponentsWidget::TargetOffsetToMonsterIndex( eg_int TargetOffset )
{
	eg_int ListIndex = TargetOffset;
	if( TargetOffset > 0 )
	{
		ListIndex = TargetOffset*2 - 1;
	}
	else
	{
		ListIndex = -TargetOffset*2;
	}

	return ListIndex;
}

void ExCombatMenuOpponentsWidget::MoveTarget( eg_int MoveDelta , eg_bool bDontMoveCamera /* = false */ )
{
	eg_int OriginalOffset = m_TargetOffset;
	m_TargetOffset += MoveDelta;

	// EGLogToScreen::Get().Log( this , __LINE__ , 5.f , *EGSFormat8( "TargetOffset: {0}" , m_TargetOffset ) );

	exMonsterObj* TargetedObj = GetMonsterAtTargetOffset( m_TargetOffset );
	if( TargetedObj )
	{
		ExUiSounds::Get().PlaySoundEvent( ExUiSounds::ex_sound_e::SELECTION_CHANGED );
		if( !bDontMoveCamera )
		{
			MoveCameraToMonster( TargetedObj , false );
		}
		RefreshTargetted( TargetedObj );
	}
	else
	{
		m_TargetOffset = OriginalOffset;
	}
}

void ExCombatMenuOpponentsWidget::RefreshTargetted( exMonsterObj* NewTarget )
{
	for( exMonsterObj& MonsterObj : m_FrontRowMonsters )
	{
		MonsterObj.SetIsSelected( &MonsterObj == NewTarget );
	}

	for( exMonsterObj& MonsterObj : m_TempMonsters )
	{
		MonsterObj.SetIsSelected( false );
	}
}

void ExCombatMenuOpponentsWidget::MoveCameraToMonster( exMonsterObj* MonsterObj , eg_bool bRestore )
{
	const eg_transform NewCameraPose = eg_transform::BuildTranslation( eg_vec3(-MonsterObj->GetTargetPose().GetTranslation().x , 0.f , 0.f) );
	const eg_transform FinalPose = m_CenterPose.GetTargetPose().GetInverse() * NewCameraPose; // Remove the centering pose since that would offset the monster.
	if( bRestore )
	{
		m_CameraPose.MoveToThenRestore( FinalPose , CT_Default , true );
	}
	else
	{
		m_CameraPose.MoveTo( FinalPose , true );
	}
}

void ExCombatMenuOpponentsWidget::RestoreCamera()
{	
	m_CameraPose.MoveTo( CT_Default , true );
}

void ExCombatMenuOpponentsWidget::CreateMonsterMap()
{
	if( m_Combat )
	{
		ExCombatantList OppTeamList;
		m_Combat->GetCombatantList( 1 , ExCombat::F_ALL , OppTeamList );

		for( ExCombatant* Monster : OppTeamList )
		{
			if( !m_MonsterMap.Contains( Monster->GetBeastiaryId() ) )
			{
				const exBeastInfo& BeastInfo = ExBeastiary::Get().FindInfo( Monster->GetBeastiaryId() );

				EGEntObj* NewEntObj = new EGEntObj;
				if( NewEntObj )
				{
					NewEntObj->Init( eg_crc("ENT_UI_MonsterSprite") );
					NewEntObj->SetTexture( eg_crc("Sprite") , eg_crc("Mesh") , *BeastInfo.ImagePath.FullPath );
					m_MonsterMap.Insert( Monster->GetBeastiaryId() , NewEntObj );
				}
				else
				{
					assert( false );
				}
			}
		}
	}
}

void ExCombatMenuOpponentsWidget::DestroyMonsterMap()
{
	for( eg_size_t i=0; i<m_MonsterMap.Len(); i++ )
	{
		EGEntObj* DeleteObj = m_MonsterMap.GetByIndex( i );
		if( DeleteObj )
		{
			DeleteObj->Deinit();
			delete DeleteObj;
		}
	}
	m_MonsterMap.Clear();
}

ExCombatMenuOpponentsWidget::exMonsterObj* ExCombatMenuOpponentsWidget::GetMonsterAtTargetOffset( eg_int Offset )
{
	eg_size_t ListIndex = TargetOffsetToMonsterIndex( Offset );
	return m_FrontRowMonsters.IsValidIndex( ListIndex ) && m_FrontRowMonsters[ListIndex].Type != ex_monster_t::Empty ? &m_FrontRowMonsters[ListIndex] : nullptr;
}

void ExCombatMenuOpponentsWidget::InitCombat( class ExCombat* Combat )
{
	ExCombatTeamWidgetBase::InitCombat( Combat );

	CreateMonsterMap();
}

void ExCombatMenuOpponentsWidget::SetCombatantList( const ExCombatTeam& Team , const ExCombatantList& List )
{
	ExCombatTeamWidgetBase::SetCombatantList( Team , List );

	const eg_int FrontRowSize = Team.GetFrontRowWidth();

	m_FrontRowMonsters.Resize( FrontRowSize ); // Maybe be removing or adding slots...

	auto IsInList = [this]( const ExCombatant* Cmb ) -> eg_bool
	{
		for( const ExCombatant* CompareCmb : m_CombatantList )
		{
			if( CompareCmb == Cmb )
			{
				return true;
			}
		}
		return false;
	};

	for( exMonsterObj& SlotObj : m_FrontRowMonsters )
	{
		if( SlotObj.Type == ex_monster_t::Empty || SlotObj.Type == ex_monster_t::Dead || !IsInList( SlotObj.Combatant ) )
		{
			SlotObj.Type = ex_monster_t::Empty;
		}
	}

	// We now want to move all monsters towards the center...
	for( eg_int SlotIndex=0; SlotIndex<FrontRowSize; SlotIndex++ )
	{
		eg_bool bWasMoved = false;
		exMonsterObj& SlotObj = m_FrontRowMonsters[SlotIndex];
		if( SlotObj.Type == ex_monster_t::Empty )
		{
			eg_bool bCanSearchRight = (SlotIndex == 0) || ((SlotIndex%2) == 1);
			eg_bool bCanSearchLeft = (SlotIndex == 0) || ((SlotIndex%2) == 0);
			for( eg_int CompareIndex = SlotIndex+1; CompareIndex<FrontRowSize; CompareIndex++ )
			{
				exMonsterObj& CompareObj = m_FrontRowMonsters[CompareIndex];
				const eg_bool bIsRight = ((CompareIndex%2)==1); // right is odd numbers, left is even
				if( !bWasMoved && ((bIsRight && bCanSearchRight) || (!bIsRight && bCanSearchLeft)) )
				{
					if( CompareObj.Type != ex_monster_t::Empty && CompareObj.Type != ex_monster_t::Dead )
					{
						SlotObj = std::move( CompareObj );
						SlotObj.SetPose( GetSlotPose( CompareIndex , false , nullptr ) );
						SlotObj.MoveTo( GetSlotPose( SlotIndex , false , nullptr ) );
						bWasMoved = true;
					}
				}
			}
		}
	}

	// Reset the type of all Visible monsters to dead if they are actually
	// still on the front row this will be reset.
	for( exMonsterObj& MonsterObj : m_FrontRowMonsters )
	{
		if( MonsterObj.Type == ex_monster_t::Visible )
		{
			MonsterObj.Type = ex_monster_t::Dead;
		}
	}

	eg_vec2 MonsterExts( 0.f , 0.f );

	for( eg_uint i=0; i<m_FrontRowMonsters.LenAs<eg_uint>(); i++ )
	{
		if( m_CombatantList.IsValidIndex(i) )
		{
			const ExCombatant* MonsterCombatant = m_CombatantList[i];
			exMonsterObj* MonsterObj = GetMonsterObj( MonsterCombatant );

			if( MonsterObj )
			{
				MonsterObj->Type = ex_monster_t::Visible; // In case it wasn't visible
			}
			else
			{
				MonsterObj = SpawnMonsterObj( MonsterCombatant , false );
			}

			if( MonsterObj )
			{
				MonsterExts.x = EG_Min( MonsterExts.x , MonsterObj->GetTargetPose().GetTranslation().x );
				MonsterExts.y = EG_Max( MonsterExts.y , MonsterObj->GetTargetPose().GetTranslation().x );
			}
		}
	}

	m_CenterPose.MoveTo( eg_transform::BuildTranslation( -(MonsterExts.x + MonsterExts.y)*.5f , 0.f , 0.f ) , false );
}

ExCombatMenuOpponentsWidget::exMonsterObj::exMonsterObj( exMonsterObj&& rhs )
{
	*this = std::move( rhs );
	assert( rhs.Type == ex_monster_t::Empty );
	assert( rhs.Obj == nullptr );
}

ExCombatMenuOpponentsWidget::exMonsterObj::~exMonsterObj()
{
	if( Obj )
	{
		Obj->Deinit();
		delete Obj;
	}
}

const ExCombatMenuOpponentsWidget::exMonsterObj& ExCombatMenuOpponentsWidget::exMonsterObj::operator=( const exMonsterObj& rhs )
{
	assert( rhs.Obj == nullptr );
	Type = rhs.Type;
	Obj = rhs.Obj;
	Combatant = rhs.Combatant;
	Pose = rhs.Pose;
	TempAnimLife = rhs.TempAnimLife;
	bIsSelected = rhs.bIsSelected;
	HealthTween.SetValue( HealthTween.GetValue() );

	return *this;
}

const ExCombatMenuOpponentsWidget::exMonsterObj& ExCombatMenuOpponentsWidget::exMonsterObj::operator=( exMonsterObj&& rhs )
{
	if( Obj )
	{
		Obj->Deinit();
		delete Obj;
		Obj = nullptr;
	}

	Type = rhs.Type;
	Obj = rhs.Obj;
	Combatant = rhs.Combatant;
	Pose = rhs.Pose;
	TempAnimLife = rhs.TempAnimLife;
	bIsSelected = rhs.bIsSelected;
	HealthTween.SetValue( rhs.HealthTween.GetValue() );

	rhs.Type = ex_monster_t::Empty;
	rhs.Obj = nullptr;
	rhs.Combatant = nullptr;

	return *this;
}

void ExCombatMenuOpponentsWidget::exMonsterObj::Update( eg_real DeltaTime )
{
	if( Type == ex_monster_t::Empty )
	{
		return;
	}

	if( Obj )
	{
		Obj->Update( DeltaTime );
	}

	Pose.Update( DeltaTime );

	if( Combatant && Obj )
	{
		EGTextNodeComponent* NameText = Obj->GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("NameText") );
		EGTextNodeComponent* HealthText = Obj->GetComponentTree().GetComponentById<EGTextNodeComponent>( eg_crc("HealthText") );
		EGSkelMeshComponent* StatusOverlay = Obj->GetComponentTree().GetComponentById<EGSkelMeshComponent>( eg_crc("StatusOverlay") );

		ex_attr_value CurHP = Combatant->GetHP();
		ex_attr_value MaxHP = Combatant->GetAttrValue( ex_attr_t::HP );

		HealthTween.Update( DeltaTime , CurHP , MaxHP );

		if( NameText )
		{
			const eg_string_crc FormatStringName = eg_loc( "CombatMenuTeamBName", "{0:NAME} LVL {0:ATTR:LVL}" );

			const eg_loc_text FinalName = EGFormat( FormatStringName , Combatant );

			NameText->SetText( FinalName );
		}

		if( HealthText  )
		{
			HealthText->SetText( EGFormat( L"{0}/{1}" , CurHP , MaxHP ) );
		}

		if( StatusOverlay )
		{
			eg_real HealthTarget = EGMath_GetMappedRangeValue( HealthTween.GetCurrentValue() , eg_vec2(0.f,EG_To<eg_real>(MaxHP)) , eg_vec2(-2.f,0.f) );
			StatusOverlay->SetBone( eg_crc("HealthRightJoint") , eg_transform::BuildTranslation( HealthTarget, 0.f , 0.f ) );
		}
	}
}

void ExCombatMenuOpponentsWidget::exMonsterObj::PlayAnimation( ex_monster_anim InAnim )
{
	static const struct
	{
		ex_monster_anim AnimType;
		eg_string_crc AnimCrc;
	}
	AnimTable[] =
	{
		{ ex_monster_anim::Idle , eg_crc( "Idle" ) } ,
		{ ex_monster_anim::Dodge , eg_crc( "Dodge" ) } ,
		{ ex_monster_anim::Attack , eg_crc( "Attack" ) } ,
		{ ex_monster_anim::Die , eg_crc( "Die" ) } ,
	};

	for( eg_size_t i = 0; i < countof( AnimTable ); i++ )
	{
		if( AnimTable[i].AnimType == InAnim )
		{
			if( Obj )
			{
				Obj->RunEvent( AnimTable[i].AnimCrc );
			}
		}
	}
}

void ExCombatMenuOpponentsWidget::exMonsterObj::SetIsSelected( eg_bool bNewValue )
{
	if( bIsSelected != bNewValue )
	{
		bIsSelected = bNewValue;
		if( Obj )
		{
			Obj->RunEvent( bIsSelected ? eg_crc("ShowReticle") : eg_crc("HideReticle") );
		}
	}
}
