// (c) 2018 Beem Media

#include "EGMs3DFile.h"
#include "EGFileData.h"
// #include <set>

EGMs3DFile::EGMs3DFile()
{
	
}

EGMs3DFile::~EGMs3DFile()
{
	
}

bool EGMs3DFile::LoadFromFile( const EGFileData& MemFile )
{
	MemFile.Read( &m_Header , sizeof(ms3d_header_t) );

	if ( !EGString_EqualsCount( m_Header.id, "MS3D000000", 10))
		return false;

	if (m_Header.version != 4)
		return false;

	// vertices
	MemFile.Read(&m_nNumVertices, sizeof(word));
	m_arrVertices.Resize(m_nNumVertices);
	MemFile.Read(m_arrVertices.GetArray(), m_nNumVertices*sizeof(ms3d_vertex_t));

	// triangles
	MemFile.Read(&m_nNumTriangles, sizeof(word));
	m_arrTriangles.Resize(m_nNumTriangles);
	MemFile.Read(m_arrTriangles.GetArray(), m_nNumTriangles*sizeof(ms3d_triangle_t));

	/*
	// edges
	std::set<unsigned int> setEdgePair;
	for (i = 0; i < m_arrTriangles.Len(); i++)
	{
		word a, b;
		a = m_arrTriangles[i].vertexIndices[0];
		b = m_arrTriangles[i].vertexIndices[1];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = m_arrTriangles[i].vertexIndices[1];
		b = m_arrTriangles[i].vertexIndices[2];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = m_arrTriangles[i].vertexIndices[2];
		b = m_arrTriangles[i].vertexIndices[0];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));
	}

	for(std::set<unsigned int>::iterator it = setEdgePair.begin(); it != setEdgePair.end(); ++it)
	{
		unsigned int EdgePair = *it;
		ms3d_edge_t Edge;
		Edge.edgeIndices[0] = (word) EdgePair;
		Edge.edgeIndices[1] = (word) ((EdgePair >> 16) & 0xFFFF);
		m_arrEdges.Append(Edge);
	}
	*/

	// groups
	MemFile.Read(&m_nNumGroups, sizeof(word));
	m_arrGroups.Resize(m_nNumGroups);
	for (word i = 0; i < m_nNumGroups; i++)
	{
		MemFile.Read(&m_arrGroups[i].flags, sizeof(byte));
		MemFile.Read(&m_arrGroups[i].name, 32*sizeof(char));
		MemFile.Read(&m_arrGroups[i].numtriangles, sizeof(word));
		m_arrGroups[i].triangleIndices.Resize( m_arrGroups[i].numtriangles );
		MemFile.Read(m_arrGroups[i].triangleIndices.GetArray(), m_arrGroups[i].numtriangles*sizeof(word));
		MemFile.Read(&m_arrGroups[i].materialIndex, sizeof(char));
	}

	// materials
	MemFile.Read(&m_nNumMaterials, sizeof(word));
	m_arrMaterials.Resize(m_nNumMaterials);
	MemFile.Read(m_arrMaterials.GetArray(), m_nNumMaterials*sizeof(ms3d_material_t));

	MemFile.Read(&m_fAnimationFPS, sizeof(float));
	MemFile.Read(&m_fCurrentTime, sizeof(float));
	MemFile.Read(&m_iTotalFrames, sizeof(int));

	// joints
	MemFile.Read(&m_nNumJoints, sizeof(word));
	m_arrJoints.Resize(m_nNumJoints);
	for (word i = 0; i < m_nNumJoints; i++)
	{
		MemFile.Read(&m_arrJoints[i].flags, sizeof(byte));
		MemFile.Read(&m_arrJoints[i].name, 32*sizeof(char));
		MemFile.Read(&m_arrJoints[i].parentName, 32*sizeof(char));
		MemFile.Read(&m_arrJoints[i].rotation, 3*sizeof(float));
		MemFile.Read(&m_arrJoints[i].position, 3*sizeof(float));
		MemFile.Read(&m_arrJoints[i].numKeyFramesRot, sizeof(word));
		MemFile.Read(&m_arrJoints[i].numKeyFramesTrans, sizeof(word));
		m_arrJoints[i].keyFramesRot.Resize(m_arrJoints[i].numKeyFramesRot);
		m_arrJoints[i].keyFramesTrans.Resize(m_arrJoints[i].numKeyFramesTrans);
		MemFile.Read(m_arrJoints[i].keyFramesRot.GetArray(), m_arrJoints[i].numKeyFramesRot*sizeof(ms3d_keyframe_rot_t));
		MemFile.Read(m_arrJoints[i].keyFramesTrans.GetArray(), m_arrJoints[i].numKeyFramesTrans*sizeof(ms3d_keyframe_pos_t));
	}

	MemFile.Read( &m_subVersionA , sizeof(m_subVersionA) );
	if( m_subVersionA == 1 )
	{
		auto ReadComments = [&MemFile]( int TargetCount , EGArray<ms3d_comment_t>& TargetArray , eg_bool bNoIndex = false ) -> void
		{
			TargetArray.Resize( TargetCount );
			for( int i=0; i<TargetCount; i++ )
			{
				ms3d_comment_t& NewComment = TargetArray[i];
				if( !bNoIndex )
				{
					MemFile.Read( &NewComment.index , sizeof(NewComment.index) );
				}
				MemFile.Read( &NewComment.commentLength , sizeof(NewComment.commentLength) );
				NewComment.comment.Resize( NewComment.commentLength + 1 );
				NewComment.comment[NewComment.commentLength] = '\0';
				MemFile.Read( NewComment.comment.GetArray() , sizeof(char)*NewComment.commentLength );		
			}
		};

		MemFile.Read( &m_nNumGroupComments , sizeof(m_nNumGroupComments) );
		ReadComments( m_nNumGroupComments , m_GroupComments );
		MemFile.Read( &m_nNumMaterialComments , sizeof(m_nNumMaterialComments) );
		ReadComments( m_nNumMaterialComments , m_MaterialComments );
		MemFile.Read( &m_nNumJointComments , sizeof(m_nNumJointComments) );
		ReadComments( m_nNumJointComments , m_JointComments );
		MemFile.Read( &m_nNumModelComments , sizeof(m_nNumModelComments) );
		ReadComments( m_nNumModelComments , m_ModelComments , true );
	}

	MemFile.Read( &m_subVersionB , sizeof(m_subVersionB) );
	if( m_subVersionB == 1 || m_subVersionB == 2 )
	{
		m_VertexExV3.Resize( m_nNumVertices );
		for( word i=0; i<m_nNumVertices; i++ )
		{
			MemFile.Read( &m_VertexExV3[i].boneIds , sizeof(m_VertexExV3[i].boneIds) );
			MemFile.Read( &m_VertexExV3[i].weights , sizeof(m_VertexExV3[i].weights) );
			if( m_subVersionB == 2 )
			{
				MemFile.Read( &m_VertexExV3[i].extra[0] , sizeof(m_VertexExV3[i].extra[0]) );
			}
			else
			{
				m_VertexExV3[i].extra[0] = 0;
			}

			m_VertexExV3[i].extra[1] = 0;
		}

		m_subVersionB = 3;
	}
	else if( m_subVersionB == 3 )
	{
		m_VertexExV3.Resize( m_nNumVertices );
		MemFile.Read( m_VertexExV3.GetArray() , m_nNumVertices*sizeof(ms3d_vertex_ex_v3_t) );
	}

	MemFile.Read( &m_SubVersionC , sizeof(m_SubVersionC) );
	if( m_SubVersionC == 1 )
	{
		m_JointEx.Resize( m_nNumJoints );
		MemFile.Read( m_JointEx.GetArray() , m_nNumJoints*sizeof(ms3d_joint_ex_t) );
	}

	MemFile.Read( &m_SubVersionD , sizeof(m_SubVersionD) );
	if( m_SubVersionD == 1 )
	{
		MemFile.Read( &m_ModelEx , sizeof(m_ModelEx) );
	}

	eg_size_t SizeRead = MemFile.Tell();
	eg_size_t TotalSize = MemFile.GetSize();
	if( SizeRead != TotalSize )
	{
		EGLogf( eg_log_t::Warning , "EGMs3DFile Warning: Read only %d bytes out of %d." , EG_To<eg_int>(SizeRead) , EG_To<eg_int>(TotalSize) );
	}

	if( m_arrVertices.Len() != m_VertexExV3.Len() )
	{
		EGLogf( eg_log_t::Error , "EGMs3DFile Error: Vertex extra data mismatch (%d and %d)." , EG_To<eg_int>(m_arrVertices.Len()) , EG_To<eg_int>(m_VertexExV3.Len()) );
	}

	return true;
}

void EGMs3DFile::Clear()
{
	m_arrVertices.Clear();
	m_arrTriangles.Clear();
	// m_arrEdges.Clear();
	m_arrGroups.Clear();
	m_arrMaterials.Clear();
	m_arrJoints.Clear();
}

int EGMs3DFile::GetNumVertices()
{
	return (int) m_arrVertices.Len();
}

void EGMs3DFile::GetVertexAt(int nIndex, ms3d_vertex_t **ppVertex)
{
	if (nIndex >= 0 && nIndex < (int) m_arrVertices.Len())
		*ppVertex = &m_arrVertices[nIndex];
}

int EGMs3DFile::GetNumTriangles()
{
	return (int) m_arrTriangles.Len();
}

void EGMs3DFile::GetTriangleAt(int nIndex, ms3d_triangle_t **ppTriangle)
{
	if (nIndex >= 0 && nIndex < (int) m_arrTriangles.Len())
		*ppTriangle = &m_arrTriangles[nIndex];
}

/*
int EGMs3DFile::GetNumEdges()
{
	return (int) m_arrEdges.Len();
}

void EGMs3DFile::GetEdgeAt(int nIndex, ms3d_edge_t **ppEdge)
{
	if (nIndex >= 0 && nIndex < (int) m_arrEdges.Len())
		*ppEdge = &m_arrEdges[nIndex];
}
*/

int EGMs3DFile::GetNumGroups()
{
	return (int) m_arrGroups.Len();
}

void EGMs3DFile::GetGroupAt(int nIndex, ms3d_group_t **ppGroup)
{
	if (nIndex >= 0 && nIndex < (int) m_arrGroups.Len())
		*ppGroup = &m_arrGroups[nIndex];
}

int EGMs3DFile::GetNumMaterials()
{
	return (int) m_arrMaterials.Len();
}

void EGMs3DFile::GetMaterialAt(int nIndex, ms3d_material_t **ppMaterial)
{
	if (nIndex >= 0 && nIndex < (int) m_arrMaterials.Len())
		*ppMaterial = &m_arrMaterials[nIndex];
}

int EGMs3DFile::GetNumJoints()
{
	return (int) m_arrJoints.Len();
}

void EGMs3DFile::GetJointAt(int nIndex, ms3d_joint_t **ppJoint)
{
	if (nIndex >= 0 && nIndex < (int) m_arrJoints.Len())
		*ppJoint = &m_arrJoints[nIndex];
}

int EGMs3DFile::FindJointByName(const char* lpszName)
{
	for (size_t i = 0; i < m_arrJoints.Len(); i++)
	{
		if (!strcmp(m_arrJoints[i].name, lpszName))
			return static_cast<int>(i);
	}

	return -1;
}

eg_cpstr EGMs3DFile::GetCommentForGroup( eg_size_t GroupIndex ) const
{
	for( const ms3d_comment_t& Comment : m_GroupComments )
	{
		if( Comment.index == GroupIndex )
		{
			return Comment.comment.GetArray();
		}
	}

	return "";
}

eg_cpstr EGMs3DFile::GetCommentForMaterial( eg_size_t MaterialIndex ) const
{
	for( const ms3d_comment_t& Comment : m_MaterialComments )
	{
		if( Comment.index == MaterialIndex )
		{
			return Comment.comment.GetArray();
		}
	}

	return "";
}

eg_cpstr EGMs3DFile::GetCommentForJoint( eg_size_t JointIndex ) const
{
	for( const ms3d_comment_t& Comment : m_JointComments )
	{
		if( Comment.index == JointIndex )
		{
			return Comment.comment.GetArray();
		}
	}

	return "";
}

eg_cpstr EGMs3DFile::GetCommentForModel() const
{
	return m_ModelComments.IsValidIndex( 0 ) ? m_ModelComments[0].comment.GetArray() : "";
}

void EGMs3DFile::GetGroupTriangles( eg_size_t GroupIndex, EGArray<ms3d_full_triangle_t>& TrianglesOut, EGArray<ms3d_full_vertex_t>& VertexesOut ) const
{
	TrianglesOut.Clear( false );
	VertexesOut.Clear( false );

	auto BuildVertex = [this]( const ms3d_triangle_t& Triangle , word index ) -> ms3d_full_vertex_t
	{
		ms3d_full_vertex_t Out;
		Out.vertex = m_arrVertices[Triangle.vertexIndices[index]];
		Out.ex = m_VertexExV3[Triangle.vertexIndices[index]];
		EGMem_Copy( &Out.normal , &Triangle.vertexNormals[index] , sizeof(Out.normal) );
		Out.s = Triangle.s[index];
		Out.t = Triangle.t[index];
		return Out;
	};

	if( m_arrGroups.IsValidIndex( GroupIndex ) )
	{
		const ms3d_group_t& Group = m_arrGroups[GroupIndex];
		for( const word& TriangleIndex : Group.triangleIndices )
		{
			const ms3d_triangle_t& Triangle = m_arrTriangles[TriangleIndex];

			ms3d_full_triangle_t NewTriangle;
			for( word i=0; i<3; i++ )
			{
				NewTriangle.indexes[i] = VertexesOut.LenAs<word>();
				VertexesOut.Append( BuildVertex( Triangle , i ) );
			}
			TrianglesOut.Append( NewTriangle );
		}
	}

	//
	// Merge duplicate vertexes:
	//

	auto IsVertexNearlyEqual = []( const ms3d_full_vertex_t& v1 , const ms3d_full_vertex_t& v2 ) -> eg_bool
	{
		eg_bool bEqual = true;

		auto CheckExact = [&bEqual]( const auto& var1 , const auto& var2 ) -> void
		{
			if( var1 != var2 )
			{
				bEqual = false;
			}
		};

		auto CheckClose = [&bEqual]( const eg_real& var1 , const eg_real& var2 ) -> void
		{
			if( !EG_IsEqualEps( var1 , var2 , EG_SMALL_NUMBER ) )
			{
				bEqual = false;
			}
		};

		// Check integer values:
		CheckExact( v1.vertex.boneId , v2.vertex.boneId );
		CheckExact( v1.ex.boneIds[0] , v2.ex.boneIds[0] );
		CheckExact( v1.ex.boneIds[1] , v2.ex.boneIds[1] );
		CheckExact( v1.ex.boneIds[2] , v2.ex.boneIds[2] );
		CheckExact( v1.ex.weights[0] , v2.ex.weights[0] );
		CheckExact( v1.ex.weights[1] , v2.ex.weights[1] );
		CheckExact( v1.ex.weights[2] , v2.ex.weights[2] );

		// Check reals:
		CheckClose( v1.vertex.vertex[0] , v2.vertex.vertex[0] );
		CheckClose( v1.vertex.vertex[1] , v2.vertex.vertex[1] );
		CheckClose( v1.vertex.vertex[2] , v2.vertex.vertex[2] );
		CheckClose( v1.normal[0] , v2.normal[0] );
		CheckClose( v1.normal[1] , v2.normal[1] );
		CheckClose( v1.normal[2] , v2.normal[2] );
		CheckClose( v1.s , v2.s );
		CheckClose( v1.t , v2.t );
		
		return bEqual;
	};

	auto CombineVertex = [&VertexesOut,&TrianglesOut]( word TargetIdx , word SourceIdx ) -> void
	{
		// Fix the index in the triangles:
		const word LastIndex = VertexesOut.LenAs<word>() - 1;

		for( ms3d_full_triangle_t& Triangle : TrianglesOut )
		{
			for( word& Index : Triangle.indexes )
			{
				if( Index == SourceIdx )
				{
					Index = TargetIdx;
				}
				else if( Index == LastIndex )
				{
					// We are gonna swap the last vertex with
					// the one being removed. So those triangles
					// need to be correct as well.
					Index = SourceIdx;
				}
			}
		}

		VertexesOut[SourceIdx] = VertexesOut[LastIndex];
		VertexesOut.DeleteByIndex( LastIndex );
	};

	EGLogf( eg_log_t::Verbose , "Checking for duplicate vertexes..." );

	const word OriginalVertexCount = VertexesOut.LenAs<word>();

	for( word i=0; i < VertexesOut.LenAs<word>(); i++ ) // VertexesOut.Len() will shrink.
	{
		for( word CompareIdx=i+1; CompareIdx<VertexesOut.LenAs<word>(); CompareIdx++ ) // VertexesOut.Len() will shrink.
		{
			if( IsVertexNearlyEqual( VertexesOut[i] , VertexesOut[CompareIdx] ) )
			{
				// EGLogf( eg_log_t::General , "Duplicate vertex found %d and %d" , i , CompareIdx );
				CombineVertex( i , CompareIdx );
			}
		}
	}

	const word FinalVertexCount = VertexesOut.LenAs<word>();
	if( OriginalVertexCount != FinalVertexCount )
	{
		EGLogf( eg_log_t::General , "Reduced %d -> %d vertexes." , OriginalVertexCount , FinalVertexCount );
	}
	else
	{
		EGLogf( eg_log_t::Verbose , "All %d vertexes were unique." , OriginalVertexCount );
	}
}

void EGMs3DFile::GetKeyFrameTimes( EGArray<eg_real>& FrameTimesOut ) const
{
	FrameTimesOut.Clear( false );

	for( int i=0; i < m_iTotalFrames; i++ )
	{
		eg_real FrameTime = (i+1.f)/m_fAnimationFPS;
		FrameTimesOut.Append( FrameTime );
	}

	/*
	for( const ms3d_joint_t& Joint : m_arrJoints )
	{
		for( const ms3d_keyframe_rot_t& Rot : Joint.keyFramesRot )
		{
			FrameTimesOut.AppendUnique( Rot.time );
		}

		for( const ms3d_keyframe_pos_t& Pos : Joint.keyFramesTrans )
		{
			FrameTimesOut.AppendUnique( Pos.time );
		}
	}
	*/

	FrameTimesOut.Sort();
}

void EGMs3DFile::GetKeyFrameAtTime( eg_real FrameTime, EGArray<ms3d_full_keyframe_t>& KeyFrameOut ) const
{
	const eg_size_t NumJoints = m_arrJoints.Len();
	KeyFrameOut.Resize( NumJoints );
	for( eg_size_t i=0; i<NumJoints; i++ )
	{
		const ms3d_joint_t& SourceJoint = m_arrJoints[i];
		ms3d_full_keyframe_t& TargetJoint = KeyFrameOut[i];
		GetTransformByFrame( SourceJoint , FrameTime , TargetJoint );
	}
}

float EGMs3DFile::GetAnimationFPS()
{
	return m_fAnimationFPS;
}

float EGMs3DFile::GetCurrentTime()
{
	return m_fCurrentTime;
}

int EGMs3DFile::GetTotalFrames()
{
	return m_iTotalFrames;
}

template<class KeyFrameTarget_T>
eg_ivec2 EGMs3DFile_GetFrameAlogrithm( const EGArray<KeyFrameTarget_T>& KeyFrames , eg_real FrameTime )
{
	eg_ivec2 Out( CT_Clear );

	for( eg_int KeyIdx = 0; KeyIdx < KeyFrames.LenAs<eg_int>(); KeyIdx++ )
	{
		if( KeyFrames.IsValidIndex( KeyIdx + 1 ) )
		{
			const KeyFrameTarget_T& KeyA = KeyFrames[KeyIdx];
			const KeyFrameTarget_T& KeyB = KeyFrames[KeyIdx+1];
			if( FrameTime < KeyA.time )
			{
				Out.x = KeyIdx;
				Out.y = KeyIdx;
				break;
			}
			else if( (KeyA.time <= FrameTime && FrameTime < KeyB.time) )
			{
				Out.x = KeyIdx;
				Out.y = KeyIdx+1;
				break;
			}
		}
		else
		{
			assert( FrameTime >= KeyFrames[KeyIdx].time );
			Out.x = KeyIdx;
			Out.y = KeyIdx;
		}
	}

	return Out;
}

void EGMs3DFile::GetPositionByFrame( const ms3d_joint_t& Joint , eg_real FrameTime , ms3d_full_keyframe_t& TargetKeyFrame )
{
	static const ms3d_keyframe_pos_t DEFAULT_FIRST_KEY = { 1.0f , {0,0,0} };
	eg_ivec2 FrameIdxs = EGMs3DFile_GetFrameAlogrithm( Joint.keyFramesTrans , FrameTime );
	const ms3d_keyframe_pos_t* FirstKey = Joint.keyFramesTrans.IsValidIndex(FrameIdxs.x) ? &Joint.keyFramesTrans[FrameIdxs.x] : &DEFAULT_FIRST_KEY;
	const ms3d_keyframe_pos_t* LastKey  = Joint.keyFramesTrans.IsValidIndex(FrameIdxs.y) ? &Joint.keyFramesTrans[FrameIdxs.y] : &DEFAULT_FIRST_KEY;

	if( FirstKey == LastKey )
	{
		//assert_message( false , "Key out of range." );
		TargetKeyFrame.position[0] = LastKey->position[0];
		TargetKeyFrame.position[1] = LastKey->position[1];
		TargetKeyFrame.position[2] = LastKey->position[2];
		return;
	}

	// 3) Compute the interpolation
	assert( FirstKey->time <= LastKey->time ) // Start and end key were the same.
	const eg_real s = FrameTime - FirstKey->time;
	const eg_real e = LastKey->time - FirstKey->time;

	assert( ( s == 0 && e == 0 ) || ( s != 0 || e != 0 ) ); // One or more times were incorrect.

	const eg_real t = 0 == s ? 0 : s / e;

	assert( 0 <= t && t <= 1.0f ); // Final time out of range.

	// KeyOut->fTime = FrameTime;
	TargetKeyFrame.position[0] = EGMath_Lerp( t, FirstKey->position[0], LastKey->position[0] );
	TargetKeyFrame.position[1] = EGMath_Lerp( t, FirstKey->position[1], LastKey->position[1] );
	TargetKeyFrame.position[2] = EGMath_Lerp( t, FirstKey->position[2], LastKey->position[2] );
}

void EGMs3DFile::GetRotationByFrame( const ms3d_joint_t& Joint , eg_real FrameTime , ms3d_full_keyframe_t& TargetKeyFrame )
{
	static const ms3d_keyframe_rot_t DEFAULT_FIRST_KEY = { 1.0f , {0,0,0} };
	eg_ivec2 FrameIdxs = EGMs3DFile_GetFrameAlogrithm( Joint.keyFramesRot , FrameTime );
	const ms3d_keyframe_rot_t* FirstKey = Joint.keyFramesRot.IsValidIndex(FrameIdxs.x) ? &Joint.keyFramesRot[FrameIdxs.x] : &DEFAULT_FIRST_KEY;
	const ms3d_keyframe_rot_t* LastKey  = Joint.keyFramesRot.IsValidIndex(FrameIdxs.y) ? &Joint.keyFramesRot[FrameIdxs.y] : &DEFAULT_FIRST_KEY;

	if( FirstKey == LastKey )
	{
		//assert_message( false , "Key out of range." );
		TargetKeyFrame.rotation[0] = LastKey->rotation[0];
		TargetKeyFrame.rotation[1] = LastKey->rotation[1];
		TargetKeyFrame.rotation[2] = LastKey->rotation[2];
		return;
	}

	// 3) Compute the interpolation frame
	assert( FirstKey->time <= LastKey->time ); // Start and end key were the same.
	const eg_real s = FrameTime - FirstKey->time;
	const eg_real e = LastKey->time - FirstKey->time;

	assert( ( s == 0 && e == 0 ) || ( s != 0 || e != 0 ) ); // One or more times were incorrect.

	const eg_real t = 0 == s ? 0 : s / e;

	assert( 0 <= t && t <= 1.0f ); // Final time out of range.

	TargetKeyFrame.rotation[0] = EGMath_Lerp( t, FirstKey->rotation[0], LastKey->rotation[0] );
	TargetKeyFrame.rotation[1] = EGMath_Lerp( t, FirstKey->rotation[1], LastKey->rotation[1] );
	TargetKeyFrame.rotation[2] = EGMath_Lerp( t, FirstKey->rotation[2], LastKey->rotation[2] );
}

void EGMs3DFile::GetTransformByFrame( const ms3d_joint_t& Joint, eg_real FrameTime, ms3d_full_keyframe_t& TargetKeyFrame )
{
	GetPositionByFrame( Joint , FrameTime , TargetKeyFrame );
	GetRotationByFrame( Joint , FrameTime , TargetKeyFrame );
}
