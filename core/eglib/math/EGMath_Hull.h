#pragma once


struct eg_hull6
{
	eg_plane Planes[6];

	eg_hull6()
	{

	}

	eg_hull6( eg_hull6& rhs )
	{
		*this = rhs;
	}

	eg_hull6( const eg_aabb& aabb )
	{
		*this = aabb;
	}

	void operator=( const eg_aabb& aabb )
	{
		Planes[0] = eg_plane( -1.f , 0.f , 0.f , aabb.Min.x );
		Planes[1] = eg_plane( 1.f , 0.f , 0.f , -aabb.Max.x );

		Planes[2] = eg_plane( 0.f , -1.f , 0.f , aabb.Min.y );
		Planes[3] = eg_plane( 0.f , 1.f , 0.f , -aabb.Max.y );

		Planes[4] = eg_plane( 0.f , 0.f , -1.f , aabb.Min.z );
		Planes[5] = eg_plane( 0.f , 0.f , 1.f , -aabb.Max.z );
	}

	eg_intersect_t ComputeIntersection( const eg_vec4& A , const eg_vec4& B , eg_vec4* HitAtoB , eg_vec4* HitBtoA )const
	{
		assert( A.w == 1.f && B.w == 1.f ); //These should be points, not vectors.

		const eg_uint NumPlanes = countof(Planes);

		//Compute if there is an intersection at all. If A and B are both on the
		//positive side of any single plane, then there is no intersection.

		eg_bool BInside = true;
		eg_bool AInside = true;

		eg_real DotAs[6];
		eg_real DotBs[6];

		assert( NumPlanes <= countof(DotAs) );
		assert( NumPlanes <= countof(DotBs) );

		for( eg_uint i=0; i<NumPlanes; i++ )
		{
			DotAs[i] = Planes[i].Dot( A );
			DotBs[i] = Planes[i].Dot( B );

			if( DotAs[i] > 0.f && DotBs[i] > 0.f )
			{
				return eg_intersect_t::NONE;
			}

			//If either point is on the positive side of any of the planes
			//then it lies on the outside of the hull.
			if( DotAs[i] > 0.f )
			{
				AInside = false;
			}

			if( DotBs[i] > 0.f )
			{
				BInside = false;
			}
		}

		if( AInside && BInside )
		{
			return eg_intersect_t::INSIDE;
		}

		//We could technically do this loop above but there is a greater chance
		//of the above loop short circuiting than this occurring, so this is a
		//performance boost.

		//Check where the line from A to B hits.
		eg_real ADist = 0.f;
		eg_vec4 AHit = A;
		eg_real BDist = 0.f;
		eg_vec4 BHit = B;

		for( eg_uint i=0; i<NumPlanes; i++ )
		{
			eg_vec4 Itc = Planes[i].InteresectLine( A , B );

			if( !AInside && DotAs[i] > 0 )
			{
				eg_real Dist = (Itc-A).LenSqAsVec3();
				//The plane that A hits where the dot product is positive that is
				//furthest from A is the hull surface.
				if( Dist > ADist )
				{
					ADist = Dist;
					AHit = Itc;
				}
			}

			if( !BInside && DotBs[i] > 0 )
			{
				eg_real Dist = (Itc-B).LenSqAsVec3();
				//The plane that B hits where the dot product is positive that is
				//furthest from B is the hull surface.
				if( Dist > BDist )
				{
					ADist = Dist;
					BHit = Itc;
				}
			}
		}

		if( HitAtoB )
		{
			*HitAtoB = AHit;
		}

		if( HitBtoA )
		{
			*HitBtoA = BHit;
		}

		return AInside != BInside ? eg_intersect_t::HIT : eg_intersect_t::THROUGH;
	}
};