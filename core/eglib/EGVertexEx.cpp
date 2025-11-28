// (c) 2017 Beem Media
#include "EGBase64.h"
#include "EGXMLBase.h"

eg_uint EGVertex_FromXmlTag(egv_vert_mesh* pV, eg_bool bBase64, const EGXmlAttrGetter& Getter , eg_bool *OutFoundTan /*= nullptr */)
{
	eg_uint nID = 0;

	egv_vert_mesh v;
	zero(&v);
	v.Weight0 = 1.f;
	v.Color0 = eg_color(1.f,1.f,1.f,1.f);
	v.Pos.w = 1.0f;

	nID = Getter.GetUInt( "id" );
	Getter.GetVec( "pos" , reinterpret_cast<eg_real*>(&v.Pos) , 3 , bBase64 );
	Getter.GetVec( "norm" , reinterpret_cast<eg_real*>(&v.Norm) , 3 , bBase64 );
	Getter.GetVec( "tan" , reinterpret_cast<eg_real*>(&v.Tan) , 4 , bBase64 );
	Getter.GetVec( "tex0" , reinterpret_cast<eg_real*>(&v.Tex0) , 2 , bBase64 );
	Getter.GetVec( "tex1" , reinterpret_cast<eg_real*>(&v.Tex1) , 2 , bBase64 );
	if( Getter.DoesAttributeExist( "color0" ) )
	{
		Getter.GetVec( "color0" , reinterpret_cast<eg_real*>(&v.Color0) , 4 , bBase64 );
	}
	v.Bone0 = Getter.GetUInt( "bone0" , 0 );
	if( Getter.DoesAttributeExist( "weight0" ) )
	{
		Getter.GetVec( "weight0" , &v.Weight0 , 1 , bBase64 );
		v.Bone1 = Getter.GetUInt( "bone1" , 0 );
		Getter.GetVec( "weight1" , &v.Weight1 , 1 , bBase64 );
	}

	//The weight should be near 1.f
	eg_real TotalWeight = v.Weight0 + v.Weight1;

	static eg_bool SHOWED_IMBALANCE_WARNING = false;
	if( (1.f-TotalWeight) > EG_SMALL_EPS && !SHOWED_IMBALANCE_WARNING )
	{
		SHOWED_IMBALANCE_WARNING = true;
		EGLogf( eg_log_t::Warning , __FUNCTION__" Warning: Found a vertex with imbalance weight." );
		v.Weight0 = 1.f - v.Weight1;
	}


	if( nullptr != OutFoundTan )
	{
		*OutFoundTan = Getter.DoesAttributeExist( "tan" ) || Getter.DoesAttributeExist( "tangent" );
	}

	static eg_bool SomethingBanned = false;

	static const eg_cpstr BanList[] =
	{
		"position",
		"normal",  
		"tangent", 
		"tc_tex",  
		"tc_lm",   
	};

	for( eg_uint i=0; !SomethingBanned && i<countof(BanList); i++ )
	{
		if( Getter.DoesAttributeExist( BanList[i] ) )
		{
			EGLogf( eg_log_t::Warning , __FUNCTION__" Warning: Attribute not supported %s." , BanList[i] );
			SomethingBanned = true;
		}
	}

	*pV = v;

	return nID;
}

eg_string_big EGVertex_ToXmlTag( const egv_vert_mesh& Vert , eg_uint Id , eg_bool Base64 )
{
	eg_string_big strLine;

	if(!Base64)
	{
		strLine = EGString_Format("\t<vertex id=\"%u\" pos=\"%g %g %g\" norm=\"%g %g %g\" tan=\"%g %g %g %g\" tex0=\"%g %g\" tex1=\"%g %g\" color0=\"%g %g %g %g\" bone0=\"%u\" weight0=\"%g\" bone1=\"%u\" weight1=\"%g\"/>\r\n",
			Id, Vert.Pos.x, Vert.Pos.y, Vert.Pos.z,
			Vert.Norm.x, Vert.Norm.y, Vert.Norm.z,
			Vert.Tan.x, Vert.Tan.y, Vert.Tan.z,Vert.Tan.w,
			Vert.Tex0.x, Vert.Tex0.y,
			Vert.Tex1.x, Vert.Tex1.y,
			Vert.Color0.r, Vert.Color0.g, Vert.Color0.b, Vert.Color0.a,
			Vert.Bone0,
			Vert.Weight0,
			Vert.Bone1,
			Vert.Weight1);
	}
	else
	{
		strLine = EGString_Format("\t<vertex id=\"%u\" pos=\"%s\" norm=\"%s\" tan=\"%s\" tex0=\"%s\" tex1=\"%s\" color0=\"%s\" bone0=\"%u\" weight0=\"%s\" bone1=\"%u\" weight1=\"%s\"/>\r\n",
			Id, 
			*EGBase64_Encode(&Vert.Pos.x, sizeof(eg_real)*3),
			*EGBase64_Encode(&Vert.Norm.x, sizeof(eg_real)*3),
			*EGBase64_Encode(&Vert.Tan.x, sizeof(eg_real)*4),
			*EGBase64_Encode(&Vert.Tex0.x, sizeof(eg_real)*2),
			*EGBase64_Encode(&Vert.Tex1.x, sizeof(eg_real)*2),
			*EGBase64_Encode(&Vert.Color0.r, sizeof(eg_real)*4),
			Vert.Bone0,
			*EGBase64_Encode(&Vert.Weight0,sizeof(eg_real)*1),
			Vert.Bone1,
			*EGBase64_Encode(&Vert.Weight1,sizeof(eg_real)*1));
	}
	return strLine;
}

void EGVertex_ComputeTangents(egv_vert_mesh* aV, const egv_index* aI, eg_uint NumVerts, eg_uint NumTris)
{
	//This is a fairly complex algorithm.

	//The general idea is for each vertex, find the triangled attached to it
	//and compute the tangent and binormal from there.

	//This is taken from Lengyel 153

	//Debug stuff
	#if 0
	::EGLogf(eg_log_t::General , ("The first triangle for %s is:"), m_strFilename.String());
	::EGLogf(eg_log_t::General , ("Indexes: %i %i %i", m_pIndexes[0], m_pIndexes[1], m_pIndexes[2]);
	for(eg_uint i=0; i<3; i++)
	{
		eg_uint index = m_pIndexes[i];
		::EGLogf(eg_log_t::General , ("v%i: P(%g %g %g) N(%g %g %g) T(%g %g)"), 
			index, 
			m_pVerts[index].v3Pos.x, m_pVerts[index].v3Pos.y, m_pVerts[index].v3Pos.z,
			m_pVerts[index].v3Norm.x, m_pVerts[index].v3Norm.y, m_pVerts[index].v3Norm.z,
			m_pVerts[index].v2Tex.x, m_pVerts[index].v2Tex.y);
	}
	#endif


	for(eg_uint v=0; v<NumVerts; v++)
	{
		eg_vec4 v4Tan = eg_vec4(0,0,0,1);

		eg_vec4 P(0,0,0, 1);
		eg_vec2 Sp(0,0);
		eg_vec4 Q(0,0,0, 1);
		eg_vec2 Sq(0,0);

		eg_vec4 T(0,0,0,0);
		eg_vec4 B(0,0,0,0);

		const eg_vec4 E(aV[v].Pos);
		const eg_vec2 Se(aV[v].Tex0);
		const eg_vec4 N(aV[v].Norm);

		//The first step is to find the triangle attached to this vertex.
		//We're only finding the first triangle, we should find all the
		//triangles so we can average the tangents for all the triangles
		//sharing that vertex.
		eg_bool bFound = false;
		for(eg_uint t=0; t<NumTris && !bFound; t++)
		{
			eg_uint i1 = aI[t*3+0];
			eg_uint i2 = aI[t*3+1];
			eg_uint i3 = aI[t*3+2];

			if(i1 == v)
			{
				eg_vec4 V2(aV[i2].Pos);
				eg_vec2 T2(aV[i2].Tex0);
				eg_vec4 V3(aV[i3].Pos);
				eg_vec2 T3(aV[i3].Tex0);

				P  = V2 - E;
				Sp = T2 - Se;
				Q  = V3 - E;
				Sq = T3 - Se;
				bFound = true;
			}
			else if(i2 == v)
			{
				eg_vec4 V1(aV[i1].Pos);
				eg_vec2 T1(aV[i1].Tex0);
				eg_vec4 V3(aV[i3].Pos);
				eg_vec2 T3(aV[i3].Tex0);

				P  = V1 - E;
				Sp = T1 - Se;
				Q  = V3 - E;
				Sq = T3 - Se;
				bFound = true;
			}
			else if(i3 == v)
			{
				eg_vec4 V1(aV[i1].Pos);
				eg_vec2 T1(aV[i1].Tex0);
				eg_vec4 V2(aV[i2].Pos);
				eg_vec2 T2(aV[i2].Tex0);

				P  = V1 - E;
				Sp = T1 - Se;
				Q  = V2 - E;
				Sq = T2 - Se;
				bFound = true;
			}
		}

		//We now have the triangle
		if(bFound)
		{
			//Since we have an <s,t> matrix to invert, we comput 1/det<s,t>
			const float& s1 = Sp.x;
			const float& t1 = Sp.y;
			const float& s2 = Sq.x;
			const float& t2 = Sq.y;

			float d = s1*t2 - s2*t1;
			if(0.000001f < EG_Abs(d))
			{
				d = 1.0f/d;

				#if 0
				if(v == 0)
				{
					::EGLogf(eg_log_t::General, ("d = %g"), d);
				}
				#endif

				T.x = d*(P.x*t2 - Q.x*t1);
				T.y = d*(P.y*t2 - Q.y*t1);
				T.z = d*(P.z*t2 - Q.z*t1);
				T.w = 0;

				B.x = d*(Q.x*s1 - P.x*s2);
				B.y = d*(Q.y*s1 - P.y*s2);
				B.z = d*(Q.z*s1 - P.z*s2);
				B.w = 0;

				#if 0
				if(v==0)
				{
					::EGLogf(eg_log_t::General, ("Tangent Space for un-norm %s:"), m_strFilename.String());
					::EGLogf(eg_log_t::General, ("\t[%f %f %f;"), T.x, T.y, T.z);
					::EGLogf(eg_log_t::General, ("\t%f %f %f;"), B.x, B.y, B.z);
					::EGLogf(eg_log_t::General, ("\t%f %f %f]"), N.x, N.y, N.z);
				}
				#endif

				//Now we use the graham-schmidt algorithm to orthogonalize T and B
				T = T - N*N.Dot(T);
				T.NormalizeThisAsVec3();
				B = B - N*N.Dot(B) - T*T.Dot(B);
				B.NormalizeThisAsVec3();

				//Now to test the results we print out the first vertex's matrix
				//to see if the results are sound.
				//Well we now want the determinant of the above matrix, so let's
				//just use D3D math.
				eg_mat Md(T.x, T.y, T.z, 0,
							 B.x, B.y, B.z, 0,
							 N.x, N.y, N.z, 0,
							 0,   0,   0,   1);

				//m is computed by the determinant.
				eg_real m = Md.GetDeterminant();

				#if 0
				if(v==0)
				{
					::EGLogf(eg_log_t::General, ("Tangent Space for %s (%f):"), m_strFilename.String(), m);
					::EGLogf(eg_log_t::General, ("\t[%f %f %f;"), T.x, T.y, T.z);
					::EGLogf(eg_log_t::General, ("\t%f %f %f;"), B.x, B.y, B.z);
					::EGLogf(eg_log_t::General, ("\t%f %f %f]"), N.x, N.y, N.z);
				}
				#endif

				//We are now ready to copy the Tangent.
				aV[v].Tan.x = T.x;
				aV[v].Tan.y = T.y;
				aV[v].Tan.z = T.z;
				aV[v].Tan.w = m;
			}
			else
			{
				//No inverse, so no tangent can be computed. (probably because
				//the texture coordinates were the same for two of the vertices,
				//in which case a tangent wouldn't mean anything anyway.
				aV[v].Tan.x = 0;
				aV[v].Tan.y = 0;
				aV[v].Tan.z = 0;
				aV[v].Tan.w = 1;
			}
		}

	}
}