// (c) 2017 Beem Media

#pragma once

#include "EGEntObj.h"

class EGDebugSphere : public EGObject
{
	EG_CLASS_BODY( EGDebugSphere , EGObject )

private:

	EGEntObj m_EntObj;
	eg_real  m_Radius = 1.f;

public:

	EGDebugSphere();
	~EGDebugSphere();

	void SetupSphere( eg_real Radius );
	void SetColor( const eg_color& InColor );
	void SetPose( const eg_transform& Pose );
	void Update( eg_real DeltaTime );
	void Draw();
	void DrawDebugShape( eg_real Radius , const eg_transform& Pose , const eg_color& Color );
};

class EGDebugBox : public EGObject
{
	EG_CLASS_BODY( EGDebugBox , EGObject )

private:

	EGEntObj m_EntObj;
	eg_vec3  m_BoxSize;

public:

	EGDebugBox();
	~EGDebugBox();

	void SetupBox( const eg_vec3& Size );
	void SetupBox( eg_real WidthX , eg_real HeightY , eg_real DepthZ );
	void SetColor( const eg_color& InColor );
	void SetPose( const eg_transform& Pose );
	void Update( eg_real DeltaTime );
	void Draw();
	void DrawDebugShape( const eg_vec3& Size , const eg_transform& Pose , const eg_color& Color );
};

class EGDebugCylinder : public EGObject
{
	EG_CLASS_BODY( EGDebugCylinder , EGObject )

private:

	EGEntObj m_EntObj;
	eg_real  m_Height;
	eg_real  m_Radius;

public:

	EGDebugCylinder();
	~EGDebugCylinder();

	void SetupCylinder( eg_real Height , eg_real Radius );
	void SetColor( const eg_color& InColor );
	void SetPose( const eg_transform& Pose );
	void Update( eg_real DeltaTime );
	void Draw();
	void DrawDebugShape( eg_real Height , eg_real Radius , const eg_transform& Pose , const eg_color& Color );
};

class EGDebugCapsule : public EGObject
{
	EG_CLASS_BODY( EGDebugCapsule , EGObject )

private:

	EGEntObj m_EntObj;
	eg_real  m_Height;
	eg_real  m_Radius;

public:

	EGDebugCapsule();
	~EGDebugCapsule();

	void SetupCapsule( eg_real Height , eg_real Radius );
	void SetColor( const eg_color& InColor );
	void SetPose( const eg_transform& Pose );
	void Update( eg_real DeltaTime );
	void Draw();
	void DrawDebugShape( eg_real Height , eg_real Radius , const eg_transform& Pose , const eg_color& Color );
};

class EGDebugShapes
{
private:

	static EGDebugShapes s_Inst;

	mutable EGDebugBox* m_Box = nullptr;
	mutable EGDebugSphere* m_Sphere = nullptr;
	mutable EGDebugCylinder* m_Cylinder = nullptr;
	mutable EGDebugCapsule* m_Capsule = nullptr;
	eg_bool m_bInited = false;

public:

	static EGDebugShapes& Get() { return s_Inst; }

	void Init();
	void Deinit();
	void Update( eg_real DeltaTime );

	EGDebugBox* GetBox() const;
	EGDebugSphere* GetSphere() const;
	EGDebugCylinder* GetCylinder() const;
	EGDebugCapsule* GetCapsule() const;

private:

	void ReleaseShapes();
};
