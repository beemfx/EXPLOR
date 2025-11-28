// (c) 2017 Beem Media

#include "EGComponent.h"
#include "EGAudioTypes.h"
#include "EGWeakPtr.h"
#include "EGCrcDb.h"
#include "EGSoundEventComponent.reflection.h"

class EGSkelMeshComponent;
class EGSoundPlayerComponent;

egreflect struct egSoundEventEvent
{
	egprop eg_string_crc SoundId       = CT_Clear;
	egprop eg_string_crc SkeletonId    = CT_Clear;
	egprop eg_string_crc AnimationId   = CT_Clear;
	egprop eg_real       RunAtProgress = 0.f;
};

egreflect class EGSoundEventComponent : public egprop EGComponent
{
	EG_CLASS_BODY( EGSoundEventComponent , EGComponent )
	EG_FRIEND_RFL( EGSoundEventComponent )

protected:

	struct egSoundEvent
	{
		eg_string_crc SoundName;
		eg_vec4       Offset;
		eg_string_crc SkeletonId;
		eg_string_crc AnimId;
		eg_real       RunAtProgress;
		eg_uint       nSkel;
	};

protected:

	egprop eg_string_crc              m_SoundPlayerTarget = EGCrcDb::StringToCrc("SoundPlayer0");
	egprop EGArray<egSoundEventEvent> m_SoundEvents;

protected:

	EGWeakPtr<EGSkelMeshComponent>    m_SkelMeshSource;
	EGWeakPtr<EGSoundPlayerComponent> m_SoundPlayerTargetPtr;
	EGArray<egSoundEvent>             m_CreatedSoundEvents;

protected:

	virtual void InitComponentFromDef( const egComponentInitData& InitData ) override;
	virtual void OnPostInitComponents() override;
	virtual void RelevantUpdate( eg_real DeltaTime ) override;

	virtual void RefreshEditableProperties();
};