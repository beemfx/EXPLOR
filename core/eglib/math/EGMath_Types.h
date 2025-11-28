/*****************
*** Math Types ***
*****************/

struct eg_mat;
struct eg_vec2;
struct eg_vec3;
struct eg_vec4;
struct eg_quat;
struct eg_plane;
struct eg_color;
struct eg_aabb;
struct eg_real4;
struct eg_transform;

enum class eg_intersect_t
{
	NONE    , // Did not intersect at all.
	HIT     , // One point is outside, the other is inside.
	INSIDE  , // Both points are inside.
	THROUGH , // Both points are outside, but the line segment passed through.
};

#include <DirectXMath.h>
#include "EGVector.h"
#include "EGIntVector.h"
#include "EGMatrix.h"
#include "EGQuaternion.h"
#include "EGTransform.h"
#include "EGPlane.h"
#include "EGMath_Color.h"
#include "EGTransformMult.h"

struct eg_aabb
{
	eg_vec4 Min;
	eg_vec4 Max;

	eg_aabb() = default;
	eg_aabb( eg_ctor_t Ct )
	: Min( Ct )
	, Max( Ct )
	{
	}

	eg_aabb( const eg_vec4& InMin , const eg_vec4& InMax ): Min( InMin ) , Max( InMax ){ }

	void    MakeZeroBox(){ Min = eg_vec4(0,0,0,1.f); Max = eg_vec4(0,0,0,1.f); }
	eg_bool ContainsPoint( const eg_vec4& Point )const;
	eg_bool Intersect(const eg_aabb& pBox2, eg_aabb* pIntersect)const;
	eg_bool IsValid() const;
	void    CreateFromVec4s( const eg_vec4* pVList , eg_size_t nNumVecs );
	void    AddBox( const eg_aabb& Box );
	void    AddPoint( const eg_vec4& pVec );
	eg_real GetWidth()const{ return Max.x - Min.x; }
	eg_real GetHeight()const{ return Max.y - Min.y; }
	eg_real GetDepth()const{ return Max.z - Min.z; }
	eg_vec4 GetCenter()const{ eg_vec4 Out( (Min.x+Max.x)*0.5f , (Min.y+Max.y)*0.5f , (Min.z+Max.z)*0.5f , 1.f ); return Out; }
	eg_real GetRadiusSq()const;
	void Get8Corners( eg_vec4* OutArray , eg_size_t OutArraySize )const;
	inline eg_intersect_t ComputeIntersection( const eg_vec4& A , const eg_vec4& B , eg_vec4* HitAtoB , eg_vec4* HitBtoA )const;

	enum class eg_axis_t
	{
		None ,
		All ,
		X_Neg ,
		X_Pos ,
		Y_Neg ,
		Y_Pos ,
		Z_Neg ,
		Z_Pos ,
	};

	struct egIntersectRes
	{
		eg_intersect_t HitType = eg_intersect_t::NONE;
		eg_vec3 HitPos = CT_Clear;
		eg_axis_t HitAxis = eg_axis_t::None;
		eg_bool IsHit() const { return HitType != eg_intersect_t::NONE; }
	};

	egIntersectRes GetRayIntersection( const eg_vec3& Org , const eg_vec3& Dir ) const;
};

#include "EGMath_Hull.h"

eg_intersect_t eg_aabb::ComputeIntersection( const eg_vec4& A , const eg_vec4& B , eg_vec4* HitAtoB , eg_vec4* HitBtoA )const
{
	// TODO: Can be optimized.
	eg_hull6 Hull = *this;
	return Hull.ComputeIntersection( A , B , HitAtoB , HitBtoA );
}

//eg_real4 exists so that structs with 4 floats can be put in a union.
struct eg_real4
{
	eg_real f1,f2,f3,f4;

	void operator=(const eg_color& rhs){ f1 = rhs.r; f2 = rhs.g; f3=rhs.b; f4=rhs.a;}
	void operator=(const eg_vec4& rhs){ f1 = rhs.x; f2 = rhs.y; f3=rhs.z; f4=rhs.w;}
	void operator=(const eg_plane& rhs){ f1 = rhs.a; f2 = rhs.b; f3=rhs.c; f4=rhs.d;}
	void operator=(const eg_quat& rhs){ f1 = rhs.x; f2 = rhs.y; f3=rhs.z; f4=rhs.w;}
	void CopyTo(eg_color* lhs)const{lhs->r = f1; lhs->g = f2; lhs->b = f3; lhs->a = f4; }
	void CopyTo(eg_vec4* lhs)const{lhs->x = f1; lhs->y = f2; lhs->z = f3; lhs->w = f4; }
	void CopyTo(eg_plane* lhs)const{lhs->a = f1; lhs->b = f2; lhs->c = f3; lhs->d = f4; }
	void CopyTo(eg_quat* lhs)const{lhs->x = f1; lhs->y = f2; lhs->z = f3; lhs->w = f4; }
};
