// (c) 2017 Beem Media

#include "EGSoundEventComponent.h"
#include "EGSkelMeshComponent.h"
#include "EGSoundPlayerComponent.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGSoundEventComponent )

void EGSoundEventComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );

	const EGSoundEventComponent* AsSoundEventDef = EGCast<EGSoundEventComponent>( InitData.Def );

	m_SkelMeshSource = EGCast<EGSkelMeshComponent>( InitData.Parent );
	if( !m_SkelMeshSource.IsValid() )
	{
		EGLogf( eg_log_t::Warning , "Sound Events should be children of SkelMesh Components." );
	}

	if( InitData.bIsClient )
	{
		if( AsSoundEventDef )
		{
			for( const egSoundEventEvent& SoundEventInfo : AsSoundEventDef->m_SoundEvents )
			{
				egSoundEvent NewSoundEvent;
				NewSoundEvent.SoundName = SoundEventInfo.SoundId;
				NewSoundEvent.SkeletonId = SoundEventInfo.SkeletonId;
				NewSoundEvent.AnimId = SoundEventInfo.AnimationId;
				NewSoundEvent.RunAtProgress = SoundEventInfo.RunAtProgress;
				if( m_SkelMeshSource.IsValid() )
				{
					NewSoundEvent.nSkel = m_SkelMeshSource->GetSkelIndexByName( NewSoundEvent.SkeletonId );
				}
				m_CreatedSoundEvents.Append( NewSoundEvent );
			}
		}
	}
}

void EGSoundEventComponent::OnPostInitComponents()
{
	const EGSoundEventComponent* AsSoundEventDef = EGCast<EGSoundEventComponent>( GetDef() );

	if( AsSoundEventDef && m_InitData.OwnerCompTree )
	{
		m_SoundPlayerTargetPtr = m_InitData.OwnerCompTree->GetComponentById<EGSoundPlayerComponent>( AsSoundEventDef->m_SoundPlayerTarget );
	}
}

void EGSoundEventComponent::RelevantUpdate( eg_real DeltaTime )
{
	Super::RelevantUpdate( DeltaTime );

	if( m_SkelMeshSource.IsValid() && m_SoundPlayerTargetPtr.IsValid() )
	{
		// Loop through all sound events and see if they should play.
		for( egSoundEvent& Event : m_CreatedSoundEvents )
		{
			if( m_SkelMeshSource->ShouldDoAnimEvent( Event.nSkel , Event.AnimId , Event.RunAtProgress ) )
			{
				if( !IsAudioMuted() )
				{
					m_SoundPlayerTargetPtr->QueueSound( Event.SoundName );
				}
			}
		}
	}
}

void EGSoundEventComponent::RefreshEditableProperties()
{
	Super::RefreshEditableProperties();

	SetPropEditable( "m_ParentJointId" , false );
	SetPropEditable( "m_ScaleVector" , false );
	SetPropEditable( "m_BasePose" , false );
}
