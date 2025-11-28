// (c) 2017 Beem Media

#include "EGDebugShapes.h"

EG_CLASS_DECL( EGDebugSphere )

EGDebugSphere::EGDebugSphere()
{
	m_EntObj.Init( eg_crc("SphereR1") );
}

EGDebugSphere::~EGDebugSphere()
{
	m_EntObj.Deinit();
}

void EGDebugSphere::SetupSphere( eg_real Radius )
{
	m_Radius = EG_Abs( Radius );
	m_EntObj.SetScale( m_Radius );
}

void EGDebugSphere::SetColor( const eg_color& InColor )
{
	m_EntObj.SetPalette( InColor.ToVec4() );
}

void EGDebugSphere::SetPose( const eg_transform& Pose )
{
	m_EntObj.SetPose( Pose );
}

void EGDebugSphere::Update( eg_real DeltaTime )
{
	m_EntObj.Update( DeltaTime );
}

void EGDebugSphere::Draw()
{
	m_EntObj.Draw();
}

void EGDebugSphere::DrawDebugShape( eg_real Radius , const eg_transform& Pose , const eg_color& Color )
{
	SetupSphere( Radius );
	SetPose( Pose );
	SetColor( Color );
	Draw();
}

///////////////////////////////////////////////////////////////////////////////

EG_CLASS_DECL( EGDebugBox )

EGDebugBox::EGDebugBox()
{
	m_EntObj.Init( eg_crc("Box1x1x1") );
}

EGDebugBox::~EGDebugBox()
{
	m_EntObj.Deinit();
}

void EGDebugBox::SetupBox( const eg_vec3& Size )
{
	m_BoxSize = Size;
	m_EntObj.SetScaleVec( eg_vec4( Size , 1.f ) );
}

void EGDebugBox::SetupBox( eg_real WidthX, eg_real HeightY, eg_real DepthZ )
{
	SetupBox( eg_vec3( WidthX , HeightY , DepthZ ) );
}

void EGDebugBox::SetColor( const eg_color& InColor )
{
	m_EntObj.SetPalette( InColor.ToVec4() );
}

void EGDebugBox::SetPose( const eg_transform& Pose )
{
	m_EntObj.SetPose( Pose );
}

void EGDebugBox::Update( eg_real DeltaTime )
{
	m_EntObj.Update( DeltaTime );
}

void EGDebugBox::Draw()
{
	m_EntObj.Draw();
}

void EGDebugBox::DrawDebugShape( const eg_vec3& Size , const eg_transform& Pose , const eg_color& Color )
{
	SetupBox( Size );
	SetPose( Pose );
	SetColor( Color );
	Draw();
}

///////////////////////////////////////////////////////////////////////////////

EG_CLASS_DECL( EGDebugCylinder )

EGDebugCylinder::EGDebugCylinder()
{
	m_EntObj.Init( eg_crc("CylinderH1R1") );
}

EGDebugCylinder::~EGDebugCylinder()
{
	m_EntObj.Deinit();
}

void EGDebugCylinder::SetupCylinder( eg_real Height , eg_real Radius )
{
	m_Height = Height;
	m_Radius = Radius;
	m_EntObj.SetScaleVec( eg_vec4( m_Radius , m_Height , m_Radius , 1.f ) );
}

void EGDebugCylinder::SetColor( const eg_color& InColor )
{
	m_EntObj.SetPalette( InColor.ToVec4() );
}

void EGDebugCylinder::SetPose( const eg_transform& Pose )
{
	m_EntObj.SetPose( Pose );
}

void EGDebugCylinder::Update( eg_real DeltaTime )
{
	m_EntObj.Update( DeltaTime );
}

void EGDebugCylinder::Draw()
{
	m_EntObj.Draw();
}

void EGDebugCylinder::DrawDebugShape( eg_real Height , eg_real Radius , const eg_transform& Pose , const eg_color& Color )
{
	SetupCylinder( Height , Radius );
	SetPose( Pose );
	SetColor( Color );
	Draw();
}

///////////////////////////////////////////////////////////////////////////////

EG_CLASS_DECL( EGDebugCapsule )

EGDebugCapsule::EGDebugCapsule()
{
	m_EntObj.Init( eg_crc("CapsuleH3R1") );
}

EGDebugCapsule::~EGDebugCapsule()
{
	m_EntObj.Deinit();
}

void EGDebugCapsule::SetupCapsule( eg_real Height , eg_real Radius )
{
	m_Height = Height;
	m_Radius = Radius;
	m_EntObj.SetScaleVec( eg_vec4( m_Radius , m_Radius , m_Radius , 1.f ) );
	eg_real Extent = ((-.5f)) + ( (m_Height*.5f - m_Radius) / (m_Radius) );
	m_EntObj.SetCustomBone( eg_crc("Base") , eg_crc("Top") , eg_transform::BuildTranslation( 0.f , Extent , 0.f ) );
	m_EntObj.SetCustomBone( eg_crc("Base") , eg_crc("Bottom") , eg_transform::BuildTranslation( 0.f , -Extent , 0.f ) );
}

void EGDebugCapsule::SetColor( const eg_color& InColor )
{
	m_EntObj.SetPalette( InColor.ToVec4() );
}

void EGDebugCapsule::SetPose( const eg_transform& Pose )
{
	m_EntObj.SetPose( Pose );
}

void EGDebugCapsule::Update( eg_real DeltaTime )
{
	m_EntObj.Update( DeltaTime );
}

void EGDebugCapsule::Draw()
{
	m_EntObj.Draw();
}

void EGDebugCapsule::DrawDebugShape( eg_real Height , eg_real Radius , const eg_transform& Pose , const eg_color& Color )
{
	SetupCapsule( Height , Radius );
	SetPose( Pose );
	SetColor( Color );
	Draw();
}

///////////////////////////////////////////////////////////////////////////////

EGDebugShapes EGDebugShapes::s_Inst;

EGDebugBox* EGDebugShapes::GetBox() const
{
	assert( m_bInited );
	if( nullptr == m_Box && m_bInited )
	{
		m_Box = EGNewObject<EGDebugBox>( eg_mem_pool::System );
	}

	return m_Box;
}

EGDebugSphere* EGDebugShapes::GetSphere() const
{
	assert( m_bInited );
	if( nullptr == m_Sphere && m_bInited )
	{
		m_Sphere = EGNewObject<EGDebugSphere>( eg_mem_pool::System );
	}

	return m_Sphere;
}

EGDebugCylinder* EGDebugShapes::GetCylinder() const
{
	assert( m_bInited );
	if( nullptr == m_Cylinder && m_bInited )
	{
		m_Cylinder = EGNewObject<EGDebugCylinder>( eg_mem_pool::System );
	}

	return m_Cylinder;
}

EGDebugCapsule* EGDebugShapes::GetCapsule() const
{
	assert( m_bInited );
	if( nullptr == m_Capsule && m_bInited )
	{
		m_Capsule = EGNewObject<EGDebugCapsule>( eg_mem_pool::System );
	}

	return m_Capsule;
}

void EGDebugShapes::ReleaseShapes()
{
	EG_SafeRelease( m_Sphere );
	EG_SafeRelease( m_Box );
	EG_SafeRelease( m_Cylinder );
	EG_SafeRelease( m_Capsule );
}

void EGDebugShapes::Init()
{
	m_bInited = true;
}

void EGDebugShapes::Deinit()
{
	ReleaseShapes();

	assert( nullptr == m_Sphere );
	assert( nullptr == m_Box );
	assert( nullptr == m_Cylinder );
	assert( nullptr == m_Capsule );

	m_bInited = false;
}

void EGDebugShapes::Update( eg_real DeltaTime )
{
	if( m_Sphere )
	{
		m_Sphere->Update( DeltaTime );
	}

	if( m_Box )
	{
		m_Box->Update( DeltaTime );
	}

	if( m_Cylinder )
	{
		m_Cylinder->Update( DeltaTime );
	}

	if( m_Capsule )
	{
		m_Capsule->Update( DeltaTime );
	}
}


