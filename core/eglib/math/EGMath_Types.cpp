// (c) 2015 Beem Media

eg_bool eg_aabb::ContainsPoint( const eg_vec4& Point )const
{
	eg_aabb aabb2( Point , Point );
	return Intersect( aabb2 , nullptr );
}

eg_bool eg_aabb::Intersect(const eg_aabb& pBox2, eg_aabb* pIntersect)const
{
	const eg_aabb& pBox1 = *this;

	if(pBox1.Min.x > pBox2.Max.x)return false;
	if(pBox1.Max.x < pBox2.Min.x)return false;
	if(pBox1.Min.y > pBox2.Max.y)return false;
	if(pBox1.Max.y < pBox2.Min.y)return false;
	if(pBox1.Min.z > pBox2.Max.z)return false;
	if(pBox1.Max.z < pBox2.Min.z)return false;
	
	if(pIntersect)
	{
		(*pIntersect).Min.x=EG_Max(pBox1.Min.x, pBox2.Min.x);
		(*pIntersect).Max.x=EG_Min(pBox1.Max.x, pBox2.Max.x);
		(*pIntersect).Min.y=EG_Max(pBox1.Min.y, pBox2.Min.y);
		(*pIntersect).Max.y=EG_Min(pBox1.Max.y, pBox2.Max.y);
		(*pIntersect).Min.z=EG_Max(pBox1.Min.z, pBox2.Min.z);
		(*pIntersect).Max.z=EG_Min(pBox1.Max.z, pBox2.Max.z);
		pIntersect->Min.w = pIntersect->Max.w = 1;
	}
	return true;
}

eg_bool eg_aabb::IsValid() const
{
	return Min.x <= Max.x && Min.y <= Max.y && Min.z <= Max.z;
}

void eg_aabb::CreateFromVec4s( const eg_vec4* pVList , eg_size_t nNumVecs )
{
	if(!nNumVecs)
		return;
		
	/* Set the AABB equal to the first vector. */
	this->Max = pVList[0];
	this->Min = pVList[0];
	
	for(eg_uint i=1; i<nNumVecs; i++)
	{
		this->Min.x=EG_Min(pVList[i].x, this->Min.x);
		this->Min.y=EG_Min(pVList[i].y, this->Min.y);
		this->Min.z=EG_Min(pVList[i].z, this->Min.z);
		
		this->Max.x=EG_Max(pVList[i].x, this->Max.x);
		this->Max.y=EG_Max(pVList[i].y, this->Max.y);
		this->Max.z=EG_Max(pVList[i].z, this->Max.z);
	}

	this->Min.w = this->Max.w = 1;
}

void eg_aabb::AddPoint( const eg_vec4& pVec )
{
	eg_vec4 v[3];
	v[0] = this->Min;
	v[1] = this->Max;
	v[2] = pVec;
	CreateFromVec4s(v, 3);
}

void eg_aabb::AddBox( const eg_aabb& Box )
{
	eg_vec4 v[4];
	
	v[0]=this->Min;
	v[1]=this->Max;
	v[2]=Box.Min;
	v[3]=Box.Max;

	CreateFromVec4s( v , countof(v) );
}

eg_real eg_aabb::GetRadiusSq() const
{
	eg_vec4 RelativeCorner( GetWidth() * .5f , GetHeight() * .5f , GetDepth() * .5f , 0.f );
	eg_real RadiusSq = RelativeCorner.LenSqAsVec3();
	return RadiusSq;
}

void eg_aabb::Get8Corners( eg_vec4* OutArray , eg_size_t OutArraySize )const
{
	assert( OutArraySize >= 8 ); //Crash imminent.
	unused( OutArraySize );

	OutArray[0] = eg_vec4( Min.x , Min.y , Min.z , 1.f );
	OutArray[1] = eg_vec4( Max.x , Min.y , Min.z , 1.f );
	OutArray[2] = eg_vec4( Max.x , Max.y , Min.z , 1.f );
	OutArray[3] = eg_vec4( Min.x , Max.y , Min.z , 1.f );
	OutArray[4] = eg_vec4( Min.x , Min.y , Max.z , 1.f );
	OutArray[5] = eg_vec4( Max.x , Min.y , Max.z , 1.f );
	OutArray[6] = eg_vec4( Max.x , Max.y , Max.z , 1.f );
	OutArray[7] = eg_vec4( Min.x , Max.y , Max.z , 1.f );
}

eg_aabb::egIntersectRes eg_aabb::GetRayIntersection( const eg_vec3& Org , const eg_vec3& Dir ) const
{
	egIntersectRes Res;

	// In the interest of not screwing the code up I've copied it directly and modified it
	// as necessary to compile it in the library.

	static const eg_int NUMDIM = 3;
	static const eg_int RIGHT = 0;
	static const eg_int LEFT = 1;
	static const eg_int MIDDLE = 2;
	typedef int bbl_wooalgo_plane_t;
	typedef int bbl_wooalgo_quadrant_t;
	typedef eg_real bbl_wooalgo_vec_t[NUMDIM];

	bbl_wooalgo_vec_t minB , maxB;		/*box */
	bbl_wooalgo_vec_t origin , dir;		/*ray */
	bbl_wooalgo_vec_t coord;				/* hit point */

	minB[0] = Min.x;
	minB[1] = Min.y;
	minB[2] = Min.z;

	maxB[0] = Max.x;
	maxB[1] = Max.y;
	maxB[2] = Max.z;

	origin[0] = Org.x;
	origin[1] = Org.y;
	origin[2] = Org.z;

	dir[0] = Dir.x;
	dir[1] = Dir.y;
	dir[2] = Dir.z;

	coord[0] = 0.f;
	coord[1] = 0.f;
	coord[2] = 0.f;

	auto AndrewWooAlogorithm = [&minB , &maxB , &origin , &dir , &coord , &Res]() -> eg_bool
		{
			eg_bool inside = true;
			bbl_wooalgo_quadrant_t quadrant[NUMDIM];
			bbl_wooalgo_plane_t i;
			bbl_wooalgo_plane_t whichPlane;
			bbl_wooalgo_vec_t maxT;
			bbl_wooalgo_vec_t candidatePlane;

			/* Find candidate planes; this loop can be avoided if
			rays cast all from the eye(assume perspective view) */
			for( i = 0; i < NUMDIM; i++ )
			{
				if( origin[i] < minB[i] )
				{
					quadrant[i] = LEFT;
					candidatePlane[i] = minB[i];
					inside = false;
				}
				else if( origin[i] > maxB[i] )
				{
					quadrant[i] = RIGHT;
					candidatePlane[i] = maxB[i];
					inside = false;
				}
				else
				{
					quadrant[i] = MIDDLE;
				}
			}

			/* Ray origin inside bounding box */
			if( inside )
			{
				coord[0] = origin[0];
				coord[1] = origin[1];
				coord[2] = origin[2];
				Res.HitType = eg_intersect_t::INSIDE;
				Res.HitAxis = eg_axis_t::All;
				return (true);
			}


			/* Calculate T distances to candidate planes */
			for( i = 0; i < NUMDIM; i++ )
			{
				if( quadrant[i] != MIDDLE && dir[i] != 0.f )
				{
					maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
				}
				else
				{
					maxT[i] = -1.f;
				}
			}

			/* Get largest of the maxT's for final choice of intersection */
			whichPlane = 0;
			for( i = 1; i < NUMDIM; i++ )
			{
				if( maxT[whichPlane] < maxT[i] )
				{
					whichPlane = i;
				}
			}

			/* Check final candidate actually inside box */
			if( maxT[whichPlane] < 0.f )
			{
				return (false);
			}

			for( i = 0; i < NUMDIM; i++ )
			{
				if( whichPlane != i )
				{
					coord[i] = origin[i] + maxT[whichPlane] * dir[i];
					if( coord[i] < minB[i] || coord[i] > maxB[i] )
					{
						return (false);
					}
				}
				else
				{
					coord[i] = candidatePlane[i];
				}
			}
			Res.HitType = eg_intersect_t::HIT;
			assert( quadrant[whichPlane] != MIDDLE );
			switch( whichPlane )
			{
			case 0:
				Res.HitAxis = quadrant[0] == LEFT ? eg_axis_t::X_Neg : eg_axis_t::X_Pos;
				break;
			case 1:
				Res.HitAxis = quadrant[1] == LEFT ? eg_axis_t::Y_Neg : eg_axis_t::Y_Pos;
				break;
			case 2:
				Res.HitAxis = quadrant[2] == LEFT ? eg_axis_t::Z_Neg : eg_axis_t::Z_Pos;
				break;
			}
			return (true);				/* ray hits box */
		};

	eg_bool bHadIntersection = AndrewWooAlogorithm();
	if( bHadIntersection )
	{
		Res.HitPos.x = coord[0];
		Res.HitPos.y = coord[1];
		Res.HitPos.z = coord[2];
	}

	assert( (bHadIntersection || Res.HitType == eg_intersect_t::NONE) && (Res.HitType != eg_intersect_t::NONE || !bHadIntersection) );

	return Res;

	// Original Code:

#if 0
/*
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/

#include "GraphicsGems.h"

#define NUMDIM	3
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

		char HitBoundingBox(minB, maxB, origin, dir, coord)
			double minB[NUMDIM], maxB[NUMDIM];		/*box */
		double origin[NUMDIM], dir[NUMDIM];		/*ray */
		double coord[NUMDIM];				/* hit point */
		{
			char inside = TRUE;
			char quadrant[NUMDIM];
			register int i;
			int whichPlane;
			double maxT[NUMDIM];
			double candidatePlane[NUMDIM];

			/* Find candidate planes; this loop can be avoided if
			rays cast all from the eye(assume perpsective view) */
			for (i = 0; i < NUMDIM; i++)
				if (origin[i] < minB[i]) {
					quadrant[i] = LEFT;
					candidatePlane[i] = minB[i];
					inside = FALSE;
				}
				else if (origin[i] > maxB[i]) {
					quadrant[i] = RIGHT;
					candidatePlane[i] = maxB[i];
					inside = FALSE;
				}
				else {
					quadrant[i] = MIDDLE;
				}

			/* Ray origin inside bounding box */
			if (inside) {
				coord = origin;
				return (TRUE);
			}


			/* Calculate T distances to candidate planes */
			for (i = 0; i < NUMDIM; i++)
				if (quadrant[i] != MIDDLE && dir[i] != 0.)
					maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
				else
					maxT[i] = -1.;

			/* Get largest of the maxT's for final choice of intersection */
			whichPlane = 0;
			for (i = 1; i < NUMDIM; i++)
				if (maxT[whichPlane] < maxT[i])
					whichPlane = i;

			/* Check final candidate actually inside box */
			if (maxT[whichPlane] < 0.) return (FALSE);
			for (i = 0; i < NUMDIM; i++)
				if (whichPlane != i) {
					coord[i] = origin[i] + maxT[whichPlane] * dir[i];
					if (coord[i] < minB[i] || coord[i] > maxB[i])
						return (FALSE);
				}
				else {
					coord[i] = candidatePlane[i];
				}
			return (TRUE);				/* ray hits box */
		}
#endif
}
