// (c) 2017 Beem Media

#include "EGSkelMeshComponent.h"
#include "EGXMLBase.h"
#include "EGEngine.h"
#include "EGWorkerThreads.h"
#include "EGMesh.h"
#include "EGSkel.h"
#include "EGMeshMgr2.h"
#include "EGCrcDb.h"
#include "EGTimeline.h"
#include "EGEnt.h"
#include "EGAudioList.h"
#include "EGMeshState.h"
#include "EGRenderer.h"
#include "EGClient.h"
#include "EGFont.h"
#include "EGAudio.h"
#include "EGEntWorld.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGSkelMeshComponent )

void EGSkelMeshComponent::SetBone( eg_string_crc BoneId , const eg_transform& Transform )
{
	eg_bool bFound = false;
	for( eg_size_t i = 0; i < m_CustomBones.Len() && !bFound; i++ )
	{
		if( BoneId == m_CustomBones[i].BoneId )
		{
			m_CustomBones[i].Transform = Transform;
			bFound = true;
		}
	}

	if( !bFound )
	{
		egCustomBone NewBone;
		NewBone.BoneId = BoneId;
		NewBone.Transform = Transform;
		m_CustomBones.Append( NewBone );
	}
}

void EGSkelMeshComponent::Reset()
{
	Super::Reset();

	m_CustomBones.Clear();
}

void EGSkelMeshComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	m_SkelMeshDef = EGCast<EGSkelMeshComponent>( InitData.Def );

	// Dummy skel for index 0
	{
		egNamedSkel DummySkelZero;
		DummySkelZero.Id = CT_Clear;
		DummySkelZero.Skel = nullptr;
		DummySkelZero.SkelObj = nullptr;
		m_CreatedSkeletons.Append( DummySkelZero ); // Always have a dummy skeleton at the start, this is kinda of a stupid thing so that skeleton index 0 is nothing.
	}

	if( InitData.bIsClient )
	{
		if( m_SkelMeshDef )
		{
			for( const egSkelPath& SkelInfo : m_SkelMeshDef->m_Skeletons )
			{
				egNamedSkel NewSkel;
				NewSkel.Id = SkelInfo.Id;
				NewSkel.SkelObj = EGMeshMgr2::Get().CreateSkel( *SkelInfo.Filename.FullPath );
				NewSkel.SkelObj->OnLoaded.Bind( this , &ThisClass::OnAssetLoaded );
				NewSkel.Skel = NewSkel.SkelObj->Obj;
				m_CreatedSkeletons.Append( NewSkel );
			}
		}

		UpdateRenderAssetLoad();
	}
}

eg_transform EGSkelMeshComponent::GetJointPose( eg_string_crc JointName ) const
{
	eg_transform Out( CT_Default );

	if( m_bReady && !JointName.IsNull() )
	{
		eg_vec4 Scale = m_CachedScale.GetCurrentValue() * m_GlobalScale;
		eg_uint JointRef = m_CreatedMesh->GetJointRef( JointName );
		if( JointRef != EGMeshBase::INVALID_JOINT )
		{
			Out = *m_CreatedMesh->GetJointAttachTransform( m_MeshState , JointRef );
			Out.ScaleTranslationOfThisNonUniformly( Scale.ToVec3() );
		}
	}

	return Out;
}

void EGSkelMeshComponent::OnDestruct()
{
	for( egNamedSkel& NamedSkel : m_CreatedSkeletons )
	{
		EG_SafeRelease( NamedSkel.SkelObj );
	}
	m_CreatedSkeletons.Clear();

	Super::OnDestruct();
}

void EGSkelMeshComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	m_MeshState.Update( DeltaTime );
}

void EGSkelMeshComponent::Draw( const eg_transform& ParentPose ) const
{
	auto SetupOverrides = [this]() -> void
	{
		//We now put the addition transform the skeleton.
		for( eg_uint i = 0; i < m_CustomBones.Len(); i++ )
		{
			const egCustomBone& CustomBone = m_CustomBones[i];

			if( m_CreatedMesh )
			{
				m_CreatedMesh->SetupCustomBone( m_MeshState , CustomBone.BoneId , CustomBone.Transform );
			}
		}
	};

	
	eg_transform FullPose = m_Pose.GetCurrentValue();
	FullPose.ScaleTranslationOfThisNonUniformly( m_GlobalScale.ToVec3() );
	FullPose *= ParentPose;
	
	if( m_bReady && m_CreatedMesh && !m_bIsHidden )
	{
		MainDisplayList->SetWorldTF( eg_mat( FullPose ) );
		MainDisplayList->SetFloat( F_TIME, m_Life );
		MainDisplayList->SetVec4( eg_rv4_t::SCALE, m_CachedScale.GetCurrentValue() * m_GlobalScale );
		MainDisplayList->SetVec4( eg_rv4_t::ENT_PALETTE_0, m_Palette.GetCurrentValue() );
		SetupFrame( m_CreatedMesh , m_MeshState );
		
		SetupOverrides();

		m_CreatedMesh->Draw( m_MeshState );
	}
}

void EGSkelMeshComponent::ScriptExec( const struct egTimelineAction& Action )
{
	const eg_string_crc ActionAsCrc = eg_string_crc(Action.FnCall.FunctionName);

	switch_crc( ActionAsCrc )
	{
		case_crc("PlayAnimation"):
		{
			if( Action.FnCall.NumParms >= 4 )
			{
				PlayAnimation( Action.StrCrcParm(0) , Action.StrCrcParm(1) , Action.RealParm(2) , Action.RealParm(3) );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for PlayAnimation." );
			}
		} break;

		case_crc("SetBonePos"):
		{
			if( Action.FnCall.NumParms >= 4 )
			{
				eg_string_crc BoneCrc = Action.StrCrcParm(0);
				eg_vec3 BonePos( Action.RealParm(1) , Action.RealParm(2) , Action.RealParm(3) );
				eg_transform BonePose = eg_transform::BuildTranslation( BonePos );
				SetBone( BoneCrc , BonePose );
			}
			else
			{
				EGLogf( eg_log_t::Error , "Wrong number of parameters for SetBonePos." );
			}
		} break;

		default:
		{
			Super::ScriptExec( Action );
		} break;
	}
}

void EGSkelMeshComponent::OnPropChanged( const egRflEditor& ChangedProperty , eg_bool& bNeedsRebuildOut )
{
	Super::OnPropChanged( ChangedProperty , bNeedsRebuildOut );

	if( EGString_Equals( ChangedProperty.GetVarName() , "m_Skeletons" ) )
	{
		bNeedsRebuildOut = true;
	}
}

void EGSkelMeshComponent::SetupFrame( EGMesh* Mesh , EGMeshState& State ) const
{
	//There are two options, either we are transitioning,
	//or we are doing a regular animation.  If pCur->m_pPrevSkel
	//is not null then we are transitioning.
	if( State.m_AnmCur.SkelAnimCrc.IsNull() )
	{
		Mesh->SetupFrame( State ); //No animation so setup the default frame.
	}
	else if( 0 != State.m_AnmPrev.Skel )
	{
		Mesh->SetupFrame( State, State.m_AnmPrev.SkelAnimCrc, State.m_AnmPrev.Progress, m_CreatedSkeletons[State.m_AnmPrev.Skel].Skel, State.m_AnmCur.SkelAnimCrc, State.m_AnmCur.Progress, m_CreatedSkeletons[State.m_AnmCur.Skel].Skel );
		//m_pMesh->SetupFrame(m_AnmPrev.SkelAnimIndex, m_AnmPrev.Progress, pSkelList[m_AnmPrev.Skel-1]);
		//m_pMesh->TransitionTo(m_AnmCur.SkelAnimIndex, m_AnmCur.Progress, pSkelList[m_AnmCur.Skel-1]);
	}
	else if( 0 == State.m_AnmCur.Skel )
	{
		Mesh->SetupFrame( State ); //No skeleton so setup the default frame.
	}
	else
	{
		Mesh->SetupFrame( State, State.m_AnmCur.SkelAnimCrc, State.m_AnmCur.Progress, m_CreatedSkeletons[State.m_AnmCur.Skel].Skel );
	}
}

const EGSkel* EGSkelMeshComponent::GetSkelForMeshCompatibility() const
{
	const EGSkel* Out = nullptr;
	if( Out == nullptr )
	{
		for( const egNamedSkel& Skel : m_CreatedSkeletons )
		{
			Out = Skel.Skel;
			if( Out != nullptr )
			{
				break;
			}
		}
	}
	return Out;
}

eg_bool EGSkelMeshComponent::UpdateRenderAssetLoad()
{
	Super::UpdateRenderAssetLoad();

	assert( EGWorkerThreads_IsThisMainThread() || Engine_IsTool() );

	if( m_bSkelsReady )
	{
		return false;
	}

	if( !m_bReady )
	{
		return false;
	}


	eg_bool bReady = true;

	for( egNamedSkel& Skel : m_CreatedSkeletons )
	{
		bReady = bReady && ( nullptr == Skel.Skel || Skel.Skel->IsLoaded() );
	}

	if( !bReady )return false;

	const EGSkel* SkelForMeshCompatibility = GetSkelForMeshCompatibility();

	if( m_CreatedMesh && m_CreatedMesh->GetNumBones() > 0 )
	{
		if( SkelForMeshCompatibility )
		{
			m_CreatedMesh->MakeCompatibleWith( SkelForMeshCompatibility );
		}
	}

	m_bSkelsReady = true;
	return true;
}

eg_uint EGSkelMeshComponent::GetSkelIndexByName( eg_string_crc SkelName ) const
{
	for( eg_uint i = 0; i < m_CreatedSkeletons.LenAsUInt(); i++ )
	{
		if( m_CreatedSkeletons[i].Id == SkelName )
		{
			return i;
		}
	}
	return 0;
}

eg_bool EGSkelMeshComponent::ShouldDoAnimEvent( eg_uint SkelIndex, eg_string_crc AnimId, eg_real ProgressForEvent )
{
	eg_bool bDo = false;

	//Make sure the animation is correct, this is done by first making
	//sure we aren't transitioning, then making sure the correct skeleton
	//is active, and finally making sure the correct animation is playing.

	//Note that we don't need to make sure the params of pSE are in valid
	//range, because we did that when loading.

	const EGMeshState& State = m_MeshState;

	if(
		0 == State.m_AnmPrev.Skel
		&& State.m_AnmCur.Skel == SkelIndex
		&& State.m_AnmCur.SkelAnimCrc == AnimId
		)
	{
		const eg_real StartTime = State.m_AnmCur.LastProgress;
		const eg_real EndTime = State.m_AnmCur.Progress;

		//DebugText_MsgOverlay_AddText( DEBUG_TEXT_OVERLAY_CLIENT , EGString_Format("Progress: %.2f: (%.2f->%.2f)" , pSE->RunAtProgress , StartTime , EndTime ) );

		if( StartTime <= EndTime )
		{
			//This is the standard case when the current time and start time are
			//in order (it hasn't looped back around yet).
			if( EG_IsBetween( ProgressForEvent, StartTime, EndTime ) )
			{
				bDo = true;
			}
		}
		else
		{
			//In this case we actually looped back around so check if the play-time is one of the extremes.
			if( ProgressForEvent <= EndTime || ProgressForEvent >= StartTime )
			{
				bDo = true;
			}
		}
	}

	return bDo;
}

eg_transform EGSkelMeshComponent::GetTextNodeBonePose( eg_uint BoneId ) const
{
	eg_transform Out( CT_Default );

	const EGSkel* Skel = GetSkelForMeshCompatibility();

	if( Skel && Skel->IsLoaded() && BoneId > 0 )
	{
		Out = *Skel->GetBaseTransform( BoneId-1 );
	}

	return Out;
}

void EGSkelMeshComponent::SetAnimationNormalTime( eg_string_crc SkelName , eg_string_crc AnimationId , eg_real t )
{
	//We'll clamp some values, just so we don't go out of bounds.
	eg_uint nSkel = GetSkelIndexByName( SkelName );

	t = EG_Clamp( t , 0.f , 1.f );

	EGMeshState* Node = &m_MeshState;
	Node->m_IsPlaying = false;
	Node->m_AnmPrev.Skel = 0;
	Node->m_AnmNext.Skel = 0;
	Node->m_AnmCur.Reset();
	Node->m_AnmCur.Skel = nSkel;
	Node->m_AnmCur.SkelAnimCrc = AnimationId;
	Node->m_AnmCur.Progress = t;
}

void EGSkelMeshComponent::PlayAnimation( eg_string_crc SkelName, eg_string_crc AnimationId, eg_real fSpeed, eg_real fTransitionTime )
{
	//It isn't necessary to adjust the animation, because that is done
	//by the EGSkel class, don't need to clamp it either.
	eg_uint nSkel = GetSkelIndexByName( SkelName );

	EGMeshState* pCur = &m_MeshState;
	pCur->m_IsPlaying = true;
	if( ( pCur->m_AnmCur.Skel == nSkel ) && ( pCur->m_AnmCur.SkelAnimCrc == AnimationId ) )
	{
		//If we set the animation to the current animation,
		//then we need to insure that we aren't queing a new animation
		//and further, that we don't reset the animation, but we will
		//changed the speed (which won't take effect until the current
		//animation completes).
		pCur->m_AnmNext.Skel = 0;
		pCur->m_AnmNext.Speed = fSpeed;
	}
	else if( 0 != pCur->m_AnmPrev.Skel )
	{
		//If we are currently transitioning, then we'll que up
		//the new animation (that is we wait for the current
		//transition to complete, then we transition to the 
		//new animation (note that if the new animation is the
		//same animation we are transitioning to then the
		//case above will occur.
		pCur->m_AnmNext.Skel = nSkel;
		pCur->m_AnmNext.SkelAnimCrc = AnimationId;
		pCur->m_AnmNext.Speed = fSpeed;
	}
	else
	{
		//If this is just a regular transition then we
		//need only set the new values, and save the old values
		//for the transition:

		//Set the current values, to the old values:
		pCur->m_AnmPrev.Progress = pCur->m_AnmCur.Progress;
		pCur->m_AnmPrev.Skel = pCur->m_AnmCur.Skel;
		pCur->m_AnmPrev.SkelAnimCrc = pCur->m_AnmCur.SkelAnimCrc;

		//Set the new values
		pCur->m_AnmCur.Progress = 0;
		//We save the transition time as teh speed, because it will
		//be the speed during the transition.
		pCur->m_AnmCur.Speed = fTransitionTime;
		pCur->m_fElapsed = 0;
		//We'll save the future speed, the anim speed will change
		//to this after the transition is complete.
		pCur->m_AnmNext.Speed = fSpeed;
		pCur->m_AnmCur.SkelAnimCrc = AnimationId;
		pCur->m_AnmCur.Skel = nSkel;
	}
}
