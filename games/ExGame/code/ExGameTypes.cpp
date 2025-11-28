// (c) 2018 Beem Media

#include "ExGameTypes.h"

void exAttrSet::ZeroAll()
{
	#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) _var_ = 0;
	#include "ExAttrs.items"
}

ex_attr_value exAttrSet::GetAttrValue( ex_attr_t Type ) const
{
	switch( Type )
	{
		#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) case ex_attr_t::_var_: return _var_;
		#include "ExAttrs.items"
	}

	return 0;
}

void exAttrSet::RefreshVisibleAttributes( egRflEditor* Editor ) const
{
	if( Editor )
	{
		assert( Editor->GetData() == this );

		auto HideAttr = [&Editor]( eg_cpstr Name ) -> void
		{
			egRflEditor* AttEd = Editor->GetChildPtr( Name );
			if( AttEd )
			{
				AttEd->SetEditable( false );
				AttEd->SetSerialized( false );
			}
		};

		auto ShowAttr = [&Editor]( eg_cpstr Name ) -> void
		{
			egRflEditor* AttEd = Editor->GetChildPtr( Name );
			if( AttEd )
			{
				AttEd->SetEditable( true );
				AttEd->SetSerialized( true );
			}
		};

		#define HideAttrM( _type_ ) HideAttr( #_type_ )
		#define ShowAttrM( _type_ ) HideAttr( #_type_ )

		if( SetTypeForEditor == ex_attr_set_t::BeastAttributes || SetTypeForEditor == ex_attr_set_t::Boosts || SetTypeForEditor == ex_attr_set_t::WeaponDamage )
		{
			HideAttrM( XP_ );
			HideAttrM( LVL );
			HideAttrM( HP );
			HideAttrM( MP );
			HideAttrM( DEF_ );
			HideAttrM( DMG_ );
			HideAttrM( HIT );
			HideAttrM( DG );
			HideAttrM( ATK );
		}
		else if( SetTypeForEditor == ex_attr_set_t::CharacterCreation )
		{
			HideAttrM( XP_ );
			HideAttrM( LVL );
			HideAttrM( HP );
			HideAttrM( MP );
			HideAttrM( DEF_ );
			HideAttrM( DMG_ );
			HideAttrM( HIT );
			HideAttrM( DG );
			HideAttrM( ATK );
			HideAttrM( PDMG );
			HideAttrM( FDMG );
			HideAttrM( WDMG );
			HideAttrM( EDMG );
			HideAttrM( ADMG );
			HideAttrM( FDEF );
			HideAttrM( PDEF );
			HideAttrM( WDEF );
			HideAttrM( EDEF );
			HideAttrM( ADEF );

			/*
			#define ATTR_ITEM( _var_ , _aname_ , _lname_ , _desc_ ) HideAttrM( #_var_ );
			#include "ExAttrs.items"
			
			ShowAttrM( STR );
			ShowAttrM( MAG );
			ShowAttrM( END );
			ShowAttrM( SPD );
			ShowAttrM( AGR );
			ShowAttrM( ACC );
			ShowAttrM( BHP );
			ShowAttrM( BMP );
			*/
		}

		#undef HideAttrM
		#undef ShowAttrM
	}
}

void exPlayerClassSet::GetAllowedClasses( EGArray<eg_string_crc>& Out ) const
{
	Out.Reserve( 4 );

	if( bWarrior )
	{
		Out.AppendUnique( eg_crc( "Warrior" ) );
	}
	if( bThief )
	{
		Out.AppendUnique( eg_crc( "Thief" ) );
	}
	if( bMage )
	{
		Out.AppendUnique( eg_crc( "Mage" ) );
	}
	if( bCleric )
	{
		Out.AppendUnique( eg_crc( "Cleric" ) );
	}
}

void exPlayerClassSet::GetAllowedClasses( EGArray<ex_class_t>& Out ) const
{
	Out.Reserve( 4 );

	if( bWarrior )
	{
		Out.AppendUnique( ex_class_t::Warrior );
	}
	if( bThief )
	{
		Out.AppendUnique( ex_class_t::Thief );
	}
	if( bMage )
	{
		Out.AppendUnique( ex_class_t::Mage );
	}
	if( bCleric )
	{
		Out.AppendUnique( ex_class_t::Cleric );
	}
}

eg_bool exPlayerClassSet::IsClassSet( ex_class_t ClassType ) const
{
	switch( ClassType )
	{
	case ex_class_t::Unknown: return false;
	case ex_class_t::Warrior: return bWarrior;
	case ex_class_t::Thief: return bThief;
	case ex_class_t::Mage: return bMage;
	case ex_class_t::Cleric: return bCleric;
	case ex_class_t::EnemyMonster: return false;
	}

	return false;
}

void exSlotSet::GetAllowedSlots( EGArray<ex_item_slot>& Out ) const
{
	Out.Reserve( 2 );
	if( bHelm )
	{
		Out.Append( ex_item_slot::HELM );
	}
	if( bArmor )
	{
		Out.Append( ex_item_slot::ARMOR );
	}
	if( bWeapon1 )
	{
		Out.Append( ex_item_slot::WEAPON1 );
	}
	if( bWeapon2 )
	{
		Out.Append( ex_item_slot::WEAPON2 );
	}
	if( bAmulet )
	{
		Out.Append( ex_item_slot::AMULET );
	}
	if( bRing1 )
	{
		Out.Append( ex_item_slot::RING1 );
	}
	if( bRing2 )
	{
		Out.Append( ex_item_slot::RING2 );
	}
}

eg_bool exSlotSet::IsSlotSet( ex_item_slot SlotType ) const
{
	switch( SlotType )
	{
	case ex_item_slot::NONE: return false;
	case ex_item_slot::HELM: return bHelm;
	case ex_item_slot::ARMOR: return bArmor;
	case ex_item_slot::WEAPON1: return bWeapon1;
	case ex_item_slot::WEAPON2: return bWeapon2;
	case ex_item_slot::AMULET: return bAmulet;
	case ex_item_slot::RING1: return bRing1;
	case ex_item_slot::RING2: return bRing2;
	case ex_item_slot::COUNT: return false;
	}

	return false;
}

ex_item_slot exSlotSet::GetDefaultSlot() const
{
	if( bHelm )
	{
		return ex_item_slot::HELM;
	}
	if( bArmor )
	{
		return ex_item_slot::ARMOR;
	}
	if( bWeapon1 )
	{
		return ex_item_slot::WEAPON1;
	}
	if( bWeapon2 )
	{
		return ex_item_slot::WEAPON2;
	}
	if( bAmulet )
	{
		return ex_item_slot::AMULET;
	}
	if( bRing1 )
	{
		return ex_item_slot::RING1;
	}
	if( bRing2 )
	{
		return ex_item_slot::RING2;
	}

	return ex_item_slot::NONE;
}

eg_int exSlotSet::GetNumSlots() const
{
	eg_int TotalSlots = 0;

	if( bHelm )
	{
		TotalSlots++;
	}
	if( bArmor )
	{
		TotalSlots++;
	}
	if( bWeapon1 )
	{
		TotalSlots++;
	}
	if( bWeapon2 )
	{
		TotalSlots++;
	}
	if( bAmulet )
	{
		TotalSlots++;
	}
	if( bRing1 )
	{
		TotalSlots++;
	}
	if( bRing2 )
	{
		TotalSlots++;
	}

	return TotalSlots;
}

const eg_real exMovingPose::DEFAULT_MOVE_DURATION = .5f;

void exMovingPose::Update( eg_real DeltaTime )
{
	if( bIsMoving )
	{
		MoveTime += DeltaTime*MoveSpeed;
		if( MoveTime < 1.f )
		{
			eg_real t = EG_Clamp( EGMath_CubicInterp( 0.f , 0.f , 1.f , 0.f , MoveTime ) , 0.f , 1.f );
			Pose = eg_transform::Lerp( PrevPose , NextPose , t );
		}
		else
		{
			Pose = NextPose;
			bIsMoving = false;
		}
	}

	if( bWantsRestore && !bIsMoving )
	{
		RestoreTimer -= DeltaTime;
		if( RestoreTimer <= 0.f )
		{
			bWantsRestore = false;
			MoveTo( RestorePose , true );
		}
	}
}

void exMovingPose::SetPose( const eg_transform& NewPose )
{
	Pose = NewPose; 
	bIsMoving = false;
	bWantsRestore = false;
}

void exMovingPose::MoveTo( const eg_transform& NewPose , eg_bool bResetMoveTime )
{
	PrevPose = Pose; 
	NextPose = NewPose; 
	if( !bIsMoving || bResetMoveTime )
	{
		bIsMoving = true;
		MoveTime = 0.f; 
	}
	bWantsRestore = false;
}

void exMovingPose::MoveToThenRestore( const eg_transform& InNewPose , const eg_transform& InRestorePose , eg_bool bResetMoveTime )
{
	MoveTo( InNewPose , bResetMoveTime );
	bWantsRestore = true;
	RestoreTimer = RestoreWaitDuration; 
	RestorePose = InRestorePose;
}

const eg_transform& exMovingPose::GetTargetPose() const
{
	if( bIsMoving && !bWantsRestore )
	{
		return NextPose;
	}
	else if( bWantsRestore )
	{
		return RestorePose;
	}

	return Pose;
}

void exResourceBarTween::Update( eg_real DeltaTime , ex_attr_value CurValue , ex_attr_value MaxValue )
{
	Tween.Update( DeltaTime );

	if( CurValue != LastKnownValue && MaxValue > 0 )
	{
		LastKnownValue = CurValue;

		if( MaxValue > 0 )
		{
			// Animation time is based on how far it is moving.	
			const eg_real AnimationTime = SecondsForFullBarAnim * EG_Abs((Tween.GetCurrentValue() - EG_To<eg_real>(LastKnownValue)))/EG_To<eg_real>(MaxValue);
			Tween.MoveTo( EG_To<eg_real>(LastKnownValue) , AnimationTime , eg_tween_animation_t::Linear );
		}
		else
		{
			Tween.SetValue( EG_To<eg_real>(CurValue) );
		}
	}
}

void exResourceBarTween::SetValue( ex_attr_value NewValue )
{
	LastKnownValue = NewValue;
	Tween.SetValue( EG_To<eg_real>(LastKnownValue) );
}
