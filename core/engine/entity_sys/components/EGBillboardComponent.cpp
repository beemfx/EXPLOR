// (c) 2020 Beem Media. All Rights Reserved.

#include "EGBillboardComponent.h"
#include "EGRenderer.h"
#include "EGEnt.h"

EG_CLASS_DECL( EGBillboardComponent )

void EGBillboardComponent::InitComponentFromDef( const egComponentInitData& InitData )
{
	Super::InitComponentFromDef( InitData );
	const EGBillboardComponent* DefAsBillboard = EGCast<EGBillboardComponent>(InitData.Def);
	m_BillboardType = DefAsBillboard->m_BillboardType;
}

eg_transform EGBillboardComponent::GetPose( eg_string_crc JointName ) const
{
	eg_transform CameraPose = CT_Default;

	const eg_mat BbWV = MainDisplayList ? MainDisplayList->GetLastViewTF() : CT_Default;
	const EGEnt* EntOwner = GetOwner<EGEnt>();
	const eg_transform OwnerPose = EntOwner ? EntOwner->GetPose() : CT_Default;

	if( m_BillboardType == eg_billboarding_t::AboutYAxis )
	{
		eg_vec4 RotationVector = eg_vec4(0.f,0.f,-1.f,0.f) * BbWV * OwnerPose;
		RotationVector.y = 0.f;
		RotationVector.NormalizeThisAsVec3();
		eg_angle Rotation = EGMath_atan2( RotationVector.x , RotationVector.z );

		return eg_transform::BuildRotationY( -Rotation ) * Super::GetPose( JointName );
	}

	return Super::GetPose( JointName ); // Not implemented.
}
