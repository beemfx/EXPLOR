// (c) 2018 Beem Media

#include "EGGcxRegion.h"
#include "EGParse.h"
#include "EGBase64.h"
#include "EGTGAWriter.h"
#include "EGGcxBuildData.h"
#include "EGFileData.h"
#include "EGXMLBase.h"
#include "EGCrcDb.h"
#include "EGPath2.h"
#include "EGOsFile.h"
#include "EGCompress.h"

void EGGcxRegion::Clear()
{
	m_Floors.Clear();
	m_Name = "";
	m_LowestFloor = 0;
	m_MapMinX = 0;
	m_MapMinY = 0;
	m_MapMaxX = 0;
	m_MapMaxY = 0;
	m_MapWidth = 0;
	m_MapHeight = 0;
	m_RegionWidth = 0;
	m_RegionHeight = 0;
	m_GridShape = gcx_grid_t::UNKNOWN;
	m_Origin = gcx_tile_origin::UNKNOWN;
	m_SourceDir = "";
}

void EGGcxRegion::Init( eg_uint NewFloorsCount, eg_int NewLowestFloor, gcx_grid_t NewGridShape )
{
	Clear();

	m_Floors.Resize( NewFloorsCount );
	m_LowestFloor = NewLowestFloor;
	m_GridShape = NewGridShape;

	for( eg_uint i = 0; i < NewFloorsCount; i++ )
	{
		m_Floors[i].SetIndex( m_LowestFloor + i );
	}
}

void EGGcxRegion::ComputeMapRegions()
{
	if( m_Floors.Len() > 0 )
	{
		m_MapMinX = m_MapMaxX = m_Floors[0].x0 - 1;
		m_MapMaxY = m_MapMaxY = m_Floors[0].y0 - 1;

		for( const gcxFloor& Floor : m_Floors )
		{
			eg_int MinX = Floor.x0 - 1;
			eg_int MaxX = Floor.x0 + Floor.Width;
			eg_int MinY = Floor.y0 - 1;
			eg_int MaxY = Floor.y0 + Floor.Height;

			m_MapMinX = EG_Min( m_MapMinX, MinX );
			m_MapMaxX = EG_Max( m_MapMaxX, MaxX );

			m_MapMinY = EG_Min( m_MapMinY, MinY );
			m_MapMaxY = EG_Max( m_MapMaxY, MaxY );
		}

	}
	else
	{
		m_MapMinX = m_MapMaxX = 0;
		m_MapMinY = m_MapMaxY = 0;
	}

	m_MapWidth = m_MapMaxX - m_MapMinX + 1;
	m_MapHeight = m_MapMaxY - m_MapMinY + 1;
	m_RegionWidth = ( m_MapWidth / MAP_REGION_TILE_DIM ) + ((m_MapWidth%MAP_REGION_TILE_DIM) == 0 ? 0 : 1);
	m_RegionHeight = ( m_MapHeight / MAP_REGION_TILE_DIM ) + ((m_MapHeight%MAP_REGION_TILE_DIM) == 0 ? 0 : 1);
}

void EGGcxRegion::AddMapEntity( eg_cpstr EntDef, eg_cpstr Extra, const eg_vec4& Pos, const eg_vec4& Rot )
{
	gcxTagEntity Tag;
	Tag.EntDef = EntDef;
	Tag.ExtraStr = Extra;
	Tag.Pos = Pos;
	Tag.Rot = Rot;
	m_MapEnts.Append( Tag );
}

void EGGcxRegion::AddNoteTag( eg_int x, eg_int y, eg_cpstr Type, eg_cpstr Name )
{
	if( EG_IsBetween<eg_int>( m_CurFloorIndex, 0, static_cast<eg_int>( m_Floors.Len() ) ) )
	{
		gcxFloor* Floor = &m_Floors[m_CurFloorIndex];
		eg_aabb Bounds = Floor->GetTileBounds( x, y );
		gcxTag NoteTag;
		NoteTag.Pos = Bounds.GetCenter();
		NoteTag.Rot = eg_vec4( 0, 0, 0, 0 );
		NoteTag.Type = Type;
		eg_d_string ExtraInfo = Name;
		NoteTag.ExtraInfoDataSize = ExtraInfo.LenAs<eg_uint>()*sizeof(eg_char);
		NoteTag.ExtraInfo = *EGBase64_Encode( *ExtraInfo , ExtraInfo.Len()*sizeof(eg_char) );
		m_Tags.Append( NoteTag );
	}
}

void EGGcxRegion::AddFunctionStringTag( eg_int FloorArrayIndex , eg_int x, eg_int y, const eg_string_big& CallStr )
{
	if( EG_IsBetween<eg_int>( FloorArrayIndex, 0, static_cast<eg_int>( m_Floors.Len() ) ) )
	{
		gcxFloor& Floor = m_Floors[FloorArrayIndex];
		eg_aabb Bounds = Floor.GetTileBounds( x, y );
		gcxTag CallTag;
		CallTag.Pos = Bounds.GetCenter();
		CallTag.Rot = eg_vec4( 0, 0, 0, 0 );
		CallTag.Type = "FUNCTION";
		CallTag.ExtraInfoDataSize = static_cast<eg_uint>( CallStr.Len() * sizeof( eg_char ) );
		CallTag.ExtraInfo = *EGBase64_Encode( CallStr.String(), CallStr.Len() * sizeof( eg_char ) );
		m_Tags.Append( CallTag );
	}
}

void EGGcxRegion::AddFunctionCallTag( eg_int Floor, eg_int x, eg_int y, const egParseFuncInfoAsEgStrings& CallInfo )
{
	// Form a string for the function call:
	eg_string_big FunctionCall( CT_Clear );
	if( CallInfo.SystemName.Len() > 0 )
	{
		FunctionCall.Append( CallInfo.SystemName );
		FunctionCall.Append( '.' );
	}

	FunctionCall.Append( CallInfo.FunctionName );
	FunctionCall.Append( '(' );
	for( eg_size_t i = 0; i < CallInfo.NumParms; i++ )
	{
		FunctionCall.Append( '\"' );
		FunctionCall.Append( CallInfo.Parms[i] );
		FunctionCall.Append( '\"' );
		if( i != ( CallInfo.NumParms - 1 ) )
		{
			FunctionCall.Append( ',' );
		}
	}
	FunctionCall.Append( ')' );

	eg_uint IndexOfFloor = FloorIndexToArrayIndex( Floor );
	if( EG_IsBetween<eg_int>( IndexOfFloor, 0, static_cast<eg_int>( m_Floors.Len() ) ) )
	{
		gcxFloor* Floor = &m_Floors[IndexOfFloor];
		eg_aabb Bounds = Floor->GetTileBounds( x, y );
		gcxTag CallTag;
		CallTag.Pos = Bounds.GetCenter();
		CallTag.Rot = eg_vec4( 0, 0, 0, 0 );
		CallTag.Type = "FUNCTION";
		static_assert( sizeof( eg_char ) == 1, "This should be single byte characters" );
		CallTag.ExtraInfoDataSize = static_cast<eg_uint>( FunctionCall.Len() * sizeof( eg_char ) );
		CallTag.ExtraInfo = *EGBase64_Encode( FunctionCall.String(), FunctionCall.Len() * sizeof( eg_char ) );
		m_Tags.Append( CallTag );
	}
}

eg_uint EGGcxRegion::GetEMapRegionForTilePos( eg_int TileX, eg_int TileY ) const
{
	assert( m_MapMinX <= TileX && TileX <= m_MapMaxX );
	assert( m_MapMinY <= TileY && TileY <= m_MapMaxY );

	eg_int TileRegionX = ( TileX - m_MapMinX ) / MAP_REGION_TILE_DIM;
	eg_int TileRegionY = ( TileY - m_MapMinY ) / MAP_REGION_TILE_DIM;

	return EMapRegionCoordToIndex( TileRegionX, TileRegionY );
}

void EGGcxRegion::GetTileBoundsForEMapRegion( eg_uint RegionIndex, eg_ivec2& MinTileOut, eg_ivec2& MaxTileOut )
{
	eg_ivec2 RegionCoord = EMapRegionIndexToCoord( RegionIndex );
	MinTileOut.x = m_MapMinX + RegionCoord.x * MAP_REGION_TILE_DIM;
	MinTileOut.y = m_MapMinY + RegionCoord.y * MAP_REGION_TILE_DIM;
	MaxTileOut.x = EG_Min<eg_int>( MinTileOut.x + MAP_REGION_TILE_DIM - 1 , m_MapMaxX );
	MaxTileOut.y = EG_Min<eg_int>( MinTileOut.y + MAP_REGION_TILE_DIM - 1 , m_MapMaxY );
}

eg_uint EGGcxRegion::EMapRegionCoordToIndex( eg_uint TileRegionX, eg_uint TileRegionY ) const
{
	eg_uint Region = TileRegionY*static_cast<eg_uint>( m_RegionWidth ) + TileRegionX;
	assert( 0 <= Region && Region < GetEMapRegionCount() );
	return Region;
}

eg_ivec2 EGGcxRegion::EMapRegionIndexToCoord( eg_uint Region ) const
{
	assert( m_MapMaxX >= m_MapMinX && m_MapMaxY >= m_MapMinY );

	return eg_ivec2( ( Region % m_RegionWidth ) , ( Region / m_RegionWidth ) );
}

eg_uint EGGcxRegion::GetEMapRegionCount() const
{
	assert( m_MapMaxX >= m_MapMinX && m_MapMaxY >= m_MapMinY );

	return m_RegionWidth*m_RegionHeight;
}

void EGGcxRegion::AddWorldBox( eg_int TileX, eg_int TileY, const eg_aabb& Bounds, eg_string_crc Material )
{
	gcxWorldGeometry NewGeometry;
	NewGeometry.Type = gcxWorldGeometry::gcx_t::BOX;
	NewGeometry.Bounds = Bounds;
	NewGeometry.Material = Material;
	NewGeometry.Region = GetEMapRegionForTilePos( TileX, TileY );

	// If any bound is nearly zero then don't add it.
	const eg_bool bNoWidth = EG_IsEqualEps( NewGeometry.Bounds.GetWidth() , 0.f );
	const eg_bool bNoHeight = EG_IsEqualEps( NewGeometry.Bounds.GetHeight() , 0.f );
	const eg_bool bNoDepth = EG_IsEqualEps( NewGeometry.Bounds.GetDepth() , 0.f );

	if( bNoWidth || bNoHeight || bNoDepth )
	{
		return;
	}

	m_WorldGeometry.Append( NewGeometry );
}

void EGGcxRegion::WriteBox( const eg_aabb& Bounds, eg_string_crc Material )
{
	assert( Bounds.IsValid() );
	assert( Bounds.GetWidth() > 0.f && Bounds.GetHeight() > 0.f && Bounds.GetDepth() > 0.f );
	m_EMapGeoBrushes.Append( gcxBrush() );
	const eg_int HullIdx = EG_To<eg_int>(m_EMapGeoBrushes.Len() - 1);
	m_EMapGeoBrushes[m_EMapGeoBrushes.Len() - 1].Bounds = Bounds;

	// The visible geometry is build face by face.
	{
		egv_vert_mesh FaceVerts[600];
		eg_int NumVerts = 0;
		eg_transform FaceTrans;
		eg_mat MatrixTrans;
		eg_vec2 TxAdjust = EGGcxBuildData::Get().GetInfoForMaterial( eg_string_crc(Material) ).TextureUVScale;

#define COMMIT_VERTEXES \
					MatrixTrans = eg_mat::BuildTransform(FaceTrans); \
					EGMatrix_TransformArray( &FaceVerts[0].Pos , sizeof(FaceVerts[0]) , &FaceVerts[0].Pos , sizeof(FaceVerts[0]), MatrixTrans , NumVerts ); \
					EGMatrix_TransformArray( &FaceVerts[0].Norm , sizeof(FaceVerts[0]) , &FaceVerts[0].Norm , sizeof(FaceVerts[0]), MatrixTrans , NumVerts ); \
					EGMatrix_TransformArray( &FaceVerts[0].Tan , sizeof(FaceVerts[0]) , &FaceVerts[0].Tan , sizeof(FaceVerts[0]), MatrixTrans , NumVerts ); \
					ComputeTxCoords( FaceVerts , NumVerts , TxAdjust ); \
					for( eg_size_t i=0; i<NumVerts; i++ ){ m_EMapVertexes.Append( FaceVerts[i] ); m_EMapVertexesHullIdxs.Append( HullIdx ); }

		//S:
		NumVerts = CreateFace( Bounds.GetWidth(), Bounds.GetHeight(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.TranslateThis( Bounds.Min.x, Bounds.Min.y, Bounds.Min.z );
		COMMIT_VERTEXES

			//N:
			NumVerts = CreateFace( Bounds.GetWidth(), Bounds.GetHeight(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.RotateYThis( EG_Deg( 180.f ) );
		FaceTrans.TranslateThis( Bounds.Max.x, Bounds.Min.y, Bounds.Max.z );
		COMMIT_VERTEXES

			//E:
			NumVerts = CreateFace( Bounds.GetDepth(), Bounds.GetHeight(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.RotateYThis( EG_Deg( -90.f ) );
		FaceTrans.TranslateThis( Bounds.Max.x, Bounds.Min.y, Bounds.Min.z );
		COMMIT_VERTEXES

			//W:
			NumVerts = CreateFace( Bounds.GetDepth(), Bounds.GetHeight(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.RotateYThis( EG_Deg( 90.f ) );
		FaceTrans.TranslateThis( Bounds.Min.x, Bounds.Min.y, Bounds.Max.z );
		COMMIT_VERTEXES

			//Top:
			NumVerts = CreateFace( Bounds.GetWidth(), Bounds.GetDepth(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.RotateXThis( EG_Deg( 90.f ) );
		FaceTrans.TranslateThis( Bounds.Min.x, Bounds.Max.y, Bounds.Min.z );
		COMMIT_VERTEXES
			//Bottom:
			NumVerts = CreateFace( Bounds.GetWidth(), Bounds.GetDepth(), FaceVerts, countof( FaceVerts ) );
		FaceTrans = eg_transform::BuildIdentity();
		FaceTrans.RotateXThis( EG_Deg( -90.f ) );
		FaceTrans.TranslateThis( Bounds.Min.x, Bounds.Min.y, Bounds.Max.z );
		COMMIT_VERTEXES

			//EGLogf( "Created a faced with %u verts." , NumVerts );

#undef COMMIT_VERTEXES
	}
}

void EGGcxRegion::ComputeTxCoords( egv_vert_mesh* V, eg_size_t Count, const eg_vec2& TxAdjust )
{
	//Compute texture coords:
	const eg_real TEX_SCALEU = TxAdjust.x;
	const eg_real TEX_SCALEV = TxAdjust.y;

	for( eg_uint i = 0; i < Count; i++ )
	{
		if( EG_Abs( V[i].Norm.z ) > 0.99f )
		{
			const eg_real UMul = V[i].Norm.z < 0 ? 1.f : -1.f;
			const eg_real VMul = -1.f;
			V[i].Tex0 = eg_vec2( UMul*V[i].Pos.x*TEX_SCALEU, VMul*V[i].Pos.y*TEX_SCALEV );
		}
		else if( EG_Abs( V[i].Norm.x ) > 0.99f )
		{
			const eg_real UMul = V[i].Norm.x > 0 ? 1.f : -1.f;
			const eg_real VMul = -1.f;
			V[i].Tex0 = eg_vec2( UMul*V[i].Pos.z*TEX_SCALEU, VMul*V[i].Pos.y*TEX_SCALEV );
		}
		else if( EG_Abs( V[i].Norm.y ) > 0.99f )
		{
			V[i].Tex0 = eg_vec2( V[i].Pos.x*TEX_SCALEU, V[i].Pos.z*TEX_SCALEV );
		}

		V[i].Weight0 = 1.f;
	}
}

void EGGcxRegion::WriteDoorFrame( eg_int TileX, eg_int TileY, const eg_vec4& Min, const eg_vec4& Max, eg_string_crc MaterialType, eg_real DoorWidth, eg_real DoorHeight, eg_real BottomAdjustment )
{
	const eg_real DOOR_HEIGHT = DoorHeight;
	const eg_real DOOR_WIDTH = DoorWidth;


	// Top needs to be divided into three segments so the vertexes line up so lighting doesn't look weird.
	const eg_aabb TopBounds = { Min + eg_vec4( 0,DOOR_HEIGHT,0,0 ) , Max };

	if( Max.x - Min.x > Max.z - Min.z )
	{
		// Wall runs along X axis

		// Top
		{
			eg_aabb TopLeft = TopBounds;
			TopLeft.Max.x = TopLeft.Min.x + (TopBounds.GetWidth() - DOOR_WIDTH) / 2.f;
			AddWorldBox( TileX, TileY, TopLeft, MaterialType );
			eg_aabb TopRight = TopBounds;
			TopRight.Min.x = TopRight.Min.x + (TopBounds.GetWidth() + DOOR_WIDTH) / 2.f;
			AddWorldBox( TileX , TileY , TopRight , MaterialType );
			eg_aabb TopMid = TopBounds;
			TopMid.Min.x = TopLeft.Max.x;
			TopMid.Max.x = TopRight.Min.x;
			AddWorldBox( TileX , TileY , TopMid , MaterialType );
		}

		eg_aabb Bounds = { Min , Max };
		Bounds.Min.y += BottomAdjustment;

		// Left side
		Bounds.Max.y = DOOR_HEIGHT;
		Bounds.Max.x = Bounds.Min.x + ( Bounds.GetWidth() - DOOR_WIDTH ) / 2.f;
		AddWorldBox( TileX, TileY, Bounds, MaterialType );
		// Right side
		Bounds.Max.x = Max.x;
		Bounds.Min.x = Bounds.Min.x + ( Bounds.GetWidth() + DOOR_WIDTH ) / 2.f;
		AddWorldBox( TileX, TileY, Bounds, MaterialType );
	}
	else
	{
		// Wall runs along z axis

		// Top
		{
			eg_aabb TopLeft = TopBounds;
			TopLeft.Max.z = TopLeft.Min.z + (TopBounds.GetDepth() - DOOR_WIDTH) / 2.f;
			AddWorldBox( TileX, TileY, TopLeft, MaterialType );
			eg_aabb TopRight = TopBounds;
			TopRight.Min.z = TopRight.Min.z + (TopBounds.GetDepth() + DOOR_WIDTH) / 2.f;
			AddWorldBox( TileX , TileY , TopRight , MaterialType );
			eg_aabb TopMid = TopBounds;
			TopMid.Min.z = TopLeft.Max.z;
			TopMid.Max.z = TopRight.Min.z;
			AddWorldBox( TileX , TileY , TopMid , MaterialType );
		}

		eg_aabb Bounds = { Min , Max };
		Bounds.Min.y += BottomAdjustment;

		// Left side
		Bounds.Max.y = DOOR_HEIGHT;
		Bounds.Max.z = Bounds.Min.z + ( Bounds.GetDepth() - DOOR_WIDTH ) / 2.f;
		AddWorldBox( TileX, TileY, Bounds, MaterialType );
		// Right side
		Bounds.Max.z = Max.z;
		Bounds.Min.z = Bounds.Min.z + ( Bounds.GetDepth() + DOOR_WIDTH ) / 2.f;
		AddWorldBox( TileX, TileY, Bounds, MaterialType );
	}
}

void EGGcxRegion::WriteWall( eg_int TileX, eg_int TileY, gcx_edge_t Type , const eg_vec4& Min , const eg_vec4& Max , const gcxBuildWallData* WallData , eg_real BottomAdjustment , eg_string_crc Material )
{
	if( WallData && IsDoor( Type ) )
	{
		eg_real DoorWidth = WallData->StandardDoorData.Width;
		eg_real DoorHeight = WallData->StandardDoorData.Height;
		if( IsHiddenDoor( Type ) )
		{
			DoorWidth = WallData->SecretDoorData.Width;
			DoorHeight = WallData->SecretDoorData.Height;
		}
		WriteDoorFrame( TileX, TileY, Min, Max, Material, DoorWidth, DoorHeight, BottomAdjustment );
	}
	else
	{
		eg_aabb Bounds = { Min , Max };
		Bounds.Min.y += BottomAdjustment;
		AddWorldBox( TileX, TileY, Bounds, Material );
	}
}

eg_bool EGGcxRegion::HasSouthOrEastSideTorch( gcx_edge_t Type , eg_bool bDoorsHaveTorches )
{
	return Type == gcx_edge_t::TORCH_FACING_RD || Type == gcx_edge_t::TORCH_DOUBLE_SIDED || ( IsDoor( Type ) && bDoorsHaveTorches );
}

eg_bool EGGcxRegion::HasNorthOrWestSideTorch( gcx_edge_t Type , eg_bool bDoorsHaveTorches )
{
	return Type == gcx_edge_t::TORCH_FACING_LU || Type == gcx_edge_t::TORCH_DOUBLE_SIDED || ( IsDoor( Type ) && bDoorsHaveTorches );
}

eg_bool EGGcxRegion::IsDoor( gcx_edge_t Type )
{
	return Type == gcx_edge_t::DOOR || Type == gcx_edge_t::LOCKED_DOOR || Type == gcx_edge_t::EMPTY_DOOR_FRAME || IsHiddenDoor( Type );
}

eg_bool EGGcxRegion::IsHiddenDoor( gcx_edge_t Type )
{
	return Type == gcx_edge_t::HIDDEN_DOOR || Type == gcx_edge_t::SECRET_DOOR;
}

eg_int EGGcxRegion::CreateFace( eg_real Width, eg_real Height, egv_vert_mesh* Out, eg_int OutSize )
{
	EGMem_Set( Out, 0, OutSize * sizeof( egv_vert_mesh ) );

	auto CreateQuad = [&Out]( eg_int StartVertex, eg_vec2& QuadMin, eg_vec2& QuadMax ) -> void
	{
		eg_vec4 Min( QuadMin.x, QuadMin.y, 0, 1.f );
		eg_vec4 Max( QuadMax.x, QuadMax.y, 0.f, 1.f );
		egv_vert_mesh* V = &Out[StartVertex];
		V[0].Pos = eg_vec4( Min.x, Min.y, Min.z, 1.f );
		V[1].Pos = eg_vec4( Min.x, Max.y, Min.z, 1.f );
		V[2].Pos = eg_vec4( Max.x, Max.y, Min.z, 1.f );
		V[3].Pos = eg_vec4( Max.x, Max.y, Min.z, 1.f );
		V[4].Pos = eg_vec4( Max.x, Min.y, Min.z, 1.f );
		V[5].Pos = eg_vec4( Min.x, Min.y, Min.z, 1.f );
		for( eg_uint i = 0; i < 6; i++ )
		{
			V[0 + i].Norm = eg_vec4( 0, 0, -1.f, 0 );
		}
	};

	if( OutSize >= 6 * 4 )
	{
		CreateQuad( 0 * 6, eg_vec2( 0, 0 ), eg_vec2( Width*.5f, Height*.5f ) );
		CreateQuad( 1 * 6, eg_vec2( Width*.5f, 0 ), eg_vec2( Width, Height*.5f ) );
		CreateQuad( 2 * 6, eg_vec2( 0, Height*.5f ), eg_vec2( Width*.5f, Height ) );
		CreateQuad( 3 * 6, eg_vec2( Width*.5f, Height*.5f ), eg_vec2( Width, Height ) );
	}

	return 6 * 4;
}

void EGGcxRegion::GenerateVertexLighting_HandleVertex( egv_vert_mesh& v , eg_int VertexHullIdx )
{
	v.Color0 = eg_color( 0.f, 0.f, 0.f, 0.f );

	eg_bool bDoDotProduct = true;
	eg_bool bDoHeurHullCheck = true;

	for( eg_uint li = 0; li < m_Lights.Len(); li++ )
	{
		const gcxLight& Light = m_Lights[li];
		const eg_real RangeSq = Light.Range*Light.Range;

		//Point A will be the position of the vertex, moved just a centimeter along it's normal,
		//that way it won't intersect the hull that it is part of.
		eg_vec4 VertPos = v.Pos;
		//Point B is just the position of the light.
		const eg_vec4 LightPos = Light.Pose.GetPosition();

		eg_vec4 LightVec = LightPos - VertPos;
		
		//First off, if the vertex is out of range, then don't light it.
		eg_real DistSq = ( LightVec ).LenSqAsVec3();
		if( DistSq > RangeSq )
		{
			//Don't light if out of range.
		}
		else
		{
			eg_bool IsLightVisible = true;

			eg_real Brightness = 1.f;

			if( IsLightVisible && bDoDotProduct )
			{
				eg_vec4 NormV = v.Norm;
				eg_vec4 LightDir = LightVec;
				LightDir.NormalizeThisAsVec3();
				eg_real DotProduct = LightDir.Dot( NormV );

				eg_bool bSimpleDotProduct = true;

				if( bSimpleDotProduct )
				{
					if( DotProduct <= 0.f )
					{
						IsLightVisible = false;
					}
				}
				else
				{
					Brightness = EG_Clamp( LightDir.Dot( NormV ), 0.f, 1.f );
				}
			}

			if( IsLightVisible && bDoHeurHullCheck )
			{
				// For a hull check check with the vertex 1cm closer to the light
				// that way we won't miss the hull we are attached to.
				eg_vec4 HullCheckVertexPos = VertPos;
				HullCheckVertexPos += LightVec.GetNormalized()*.001f;
				// We'll actually check to see if there is a collision between the center of the
				// hull we are part of and any other hull, that will be the determination
				// if we use this light for calculation.
				eg_aabb HullBounds = m_EMapGeoBrushes[VertexHullIdx].Bounds;
				eg_vec4 HullCenterPos = m_EMapGeoBrushes[VertexHullIdx].Bounds.GetCenter();

				// We only do hull checks for ground and ceiling. (Walls are pretty much
				// solved by the dot product.)
				if( true ) //EG_Abs(v.Norm.y) > .5f )// !EG_IsBetweenExclusive( LightPos.y , HullBounds.Min.y , HullBounds.Max.y ) )
				{
					// Heuristic Hull Check
					const eg_int NumHulls =  m_EMapGeoBrushes.LenAs<eg_int>();

					for( eg_int HullIdx = 0; IsLightVisible && HullIdx < NumHulls; HullIdx++ )
					{
						if( HullIdx == VertexHullIdx )
						{
							// Do not compute against hull we are a part of.
							continue;
						}

						const gcxBrush& Brush = m_EMapGeoBrushes[HullIdx];

						const eg_real DistSqToBrush = (Brush.Bounds.GetCenter() - VertPos).LenSqAsVec3();
						if( DistSqToBrush > (2.f*RangeSq) )
						{
							// If the brush is certainly too far away from the light skip it.
							continue;
						}

						// We Know something about our geometry, so we'll actually only check
						// against walls. Which means we can skip anything that is above or
						// below the light.
						if( EG_IsBetweenExclusive( LightPos.y , Brush.Bounds.Min.y , Brush.Bounds.Max.y ) )
						{
							const eg_bool bVertexSawLight = eg_intersect_t::NONE == Brush.Hull.ComputeIntersection( HullCheckVertexPos, LightPos, nullptr, nullptr );
							const eg_bool bHullSawLight = eg_intersect_t::NONE == Brush.Hull.ComputeIntersection( HullCenterPos, LightPos, nullptr, nullptr );
							if( !bHullSawLight && !bVertexSawLight )
							{
								IsLightVisible = false;
							}
						}
					}
				}
			}

			if( IsLightVisible )
			{
				//Should do some kind of falloff.
				v.Color0 = eg_color( v.Color0.ToVec4() + ( Light.Color.ToVec4()*( 1.f - DistSq / RangeSq )*Brightness ) );
			}
		}
	}

	assert( v.Color0.r >= 0.f && v.Color0.g >= 0.f && v.Color0.b >= 0.f );
	v.Color0.r = EG_Min( v.Color0.r, 1.f );
	v.Color0.g = EG_Min( v.Color0.g, 1.f );
	v.Color0.b = EG_Min( v.Color0.b, 1.f );
	v.Color0.a = 1.f;
}

void EGGcxRegion::GenerateVertexLighting()
{
	//First thing to do is generate all the hulls:
	EGLogf( eg_log_t::General, "Generating hulls..." );
	for( eg_uint i = 0; i < m_EMapGeoBrushes.Len(); i++ )
	{
		m_EMapGeoBrushes[i].Hull = m_EMapGeoBrushes[i].Bounds;
	}

	//Now, for each vertex, compare it to all lights.
	EGLogf( eg_log_t::General, "Computing vertex lights..." );

	/*
	for( eg_uint i=0; i<countof(MtrlVerts); i++ )
	{
	assert( m_Verts[MtrlVerts[i].Material].Size() % sizeof(egv_vert_mesh) == 0 ); //Didn't actually write verts?
	MtrlVerts[i].Verts = const_cast<egv_vert_mesh*>(reinterpret_cast<const egv_vert_mesh*>(m_Verts[MtrlVerts[i].Material].GetMem()));
	MtrlVerts[i].VertsCount = m_Verts[MtrlVerts[i].Material].Size()/sizeof(egv_vert_mesh);
	VertsCount += MtrlVerts[i].VertsCount;
	for( eg_uint vi=0; vi<m_Verts;
	*/
	if( m_EMapVertexes.Len() == m_EMapVertexesHullIdxs.Len() )
	{
		for( eg_uint mv = 0; mv < m_EMapVertexes.Len(); mv++ )
		{
			GenerateVertexLighting_HandleVertex( m_EMapVertexes[mv] , m_EMapVertexesHullIdxs[mv]);
		}
	}
	else
	{
		EGLogf( eg_log_t::General , "Not all vertexes had hulls." );
	}
}

void EGGcxRegion::GenerateAutoMap( eg_cpstr Filename , EGGcxSaveCb& SaveCb )
{
	EGLogf( eg_log_t::General, "Generating Automap \"%s\"", Filename );

	eg_int FloorArrayIndex = FloorIndexToArrayIndex( 0 );

	if( !m_Floors.IsValidIndex( FloorArrayIndex ) )
	{
		EGLogf( eg_log_t::Error, "Could not generate automap, no ground floor." );
		return;
	}

	const gcxFloor* Floor = &m_Floors[FloorArrayIndex];

	if( !Floor->HasData() )
	{
		EGLogf( eg_log_t::Error, "Could not generate automap, no data for ground floor." );
		return;
	}

	EGTgaWriter::egPixel NoWalkPixel = eg_color32( 0, 0, 0, 0 );
	// EGTgaWriter::egPixel WallPixel = eg_color32( 232, 178, 14 );

	eg_int xMin = 0;
	eg_int yMin = 0;
	eg_int xMax = 0;
	eg_int yMax = 0;

	Floor->GetEffectiveBounds( &xMin, &xMax, &yMin, &yMax );

	eg_int Width = xMax - xMin + 1;
	eg_int Height = yMax - yMin + 1;

	static const eg_int AUTOMAP_TILE_WIDTH = 16;

	const eg_uint ImageWidth = AUTOMAP_TILE_WIDTH * Width;
	const eg_uint ImageHeight = AUTOMAP_TILE_WIDTH * Height;

	EGTgaWriter TgaFile( ImageWidth, ImageHeight );

	enum class gcx_what
	{
		NONE,
		RIGHT_WALL,
		BOTTOM_WALL,
		RIGHT_DOOR,
		BOTTOM_DOOR,
		RIGHT_FRAME,
		BOTTOM_FRAME,
	};

	auto DrawWall = [&TgaFile]( eg_int StartX, eg_int StartY, gcx_what What , const EGTgaWriter::egPixel& WallPixel )
	{
		eg_recti TileRect;
		TileRect.left = StartX;
		TileRect.right = StartX + AUTOMAP_TILE_WIDTH - 1;
		TileRect.bottom = StartY;
		TileRect.top = StartY - AUTOMAP_TILE_WIDTH;

		switch( What )
		{
		case gcx_what::NONE:
		{

		} break;

		case gcx_what::RIGHT_WALL:
		{
			for( eg_int PixelY = TileRect.top - 1; PixelY <= TileRect.bottom; PixelY++ )
			{
				TgaFile.WritePixel( TileRect.right, PixelY, WallPixel );
				TgaFile.WritePixel( TileRect.right - 1, PixelY, WallPixel );
			}
		} break;

		case gcx_what::BOTTOM_WALL:
		{
			for( eg_int PixelX = TileRect.left - 1; PixelX <= TileRect.right; PixelX++ )
			{
				TgaFile.WritePixel( PixelX, TileRect.bottom, WallPixel );
				TgaFile.WritePixel( PixelX, TileRect.bottom - 1, WallPixel );
			}
		} break;

		case gcx_what::RIGHT_DOOR:
		case gcx_what::RIGHT_FRAME:
		{
			for( eg_int PixelY = TileRect.top - 1; PixelY <= TileRect.bottom; PixelY++ )
			{
				eg_int DoorTopStart = TileRect.top + AUTOMAP_TILE_WIDTH / 3;
				eg_int DoorBottomStart = TileRect.bottom - AUTOMAP_TILE_WIDTH / 3 - 1;

				if( EG_IsBetween( PixelY, DoorTopStart, DoorBottomStart ) )
				{
					if( PixelY == DoorTopStart || PixelY == DoorBottomStart )
					{
						if( What != gcx_what::RIGHT_FRAME )
						{
							TgaFile.WritePixel( TileRect.right + 1, PixelY, WallPixel );
							TgaFile.WritePixel( TileRect.right - 2, PixelY, WallPixel );
						}
					}
					else
					{
						continue;
					}
				}

				TgaFile.WritePixel( TileRect.right, PixelY, WallPixel );
				TgaFile.WritePixel( TileRect.right - 1, PixelY, WallPixel );
			}

		} break;

		case gcx_what::BOTTOM_DOOR:
		case gcx_what::BOTTOM_FRAME:
		{
			for( eg_int PixelX = TileRect.left - 1; PixelX <= TileRect.right; PixelX++ )
			{
				eg_int DoorLeftStart = TileRect.left + AUTOMAP_TILE_WIDTH / 3 - 1;
				eg_int DoorRightStart = TileRect.right - AUTOMAP_TILE_WIDTH / 3 - 1;

				if( EG_IsBetween( PixelX, DoorLeftStart, DoorRightStart ) )
				{
					if( PixelX == DoorLeftStart || PixelX == DoorRightStart )
					{
						if( What != gcx_what::BOTTOM_FRAME )
						{
							TgaFile.WritePixel( PixelX, TileRect.bottom - 2, WallPixel );
							TgaFile.WritePixel( PixelX, TileRect.bottom + 1, WallPixel );
						}
					}
					else
					{
						continue;
					}
				}

				TgaFile.WritePixel( PixelX, TileRect.bottom, WallPixel );
				TgaFile.WritePixel( PixelX, TileRect.bottom - 1, WallPixel );
			}
		} break;
		}
	};

	auto DrawTile = [&TgaFile]( eg_uint StartX, eg_uint StartY, const gcxBuildMaterialData& MtrlType, eg_bool bCanPassThrough )
	{
		EGTgaWriter::egPixel PixelColor1 = MtrlType.AutomapGroundColor1;
		EGTgaWriter::egPixel PixelColor2 = MtrlType.AutomapGroundColor2;

		eg_int PixelAdjust = bCanPassThrough ? 0 : 1;

		for( eg_int PixelY = 0; PixelY < AUTOMAP_TILE_WIDTH + PixelAdjust * 2; PixelY++ )
		{
			for( eg_int PixelX = 0; PixelX < AUTOMAP_TILE_WIDTH + PixelAdjust * 2; PixelX++ )
			{
				eg_int x = StartX + PixelX - PixelAdjust;
				eg_int y = StartY - PixelY + PixelAdjust;
				eg_bool bAlt = x % 2 == 0 && y % 2 == 1;

				if( 0 <= x && x < EG_To<eg_int>( TgaFile.GetWidth() ) && 0 <= y && y < EG_To<eg_int>( TgaFile.GetHeight() ) )
				{
					TgaFile.WritePixel( x, y, bAlt && bCanPassThrough ? PixelColor1 : PixelColor2 );
				}
			}
		}
	};

	for( eg_uint x = 0; x < ImageWidth; x++ )
	{
		for( eg_uint y = 0; y < ImageHeight; y++ )
		{
			eg_bool bDark = x % 2 == 0 && y % 2 == 1;
			eg_bool bIsPath = false;
			TgaFile.WritePixel( x, y, NoWalkPixel );
		}
	}

	enum class gcx_pass_t
	{
		GROUND,
		GROUND_BLOCKED,
		WALLS,
	};
	auto DoPass = [&xMin, &xMax, &yMin, &yMax, &Floor, &DrawWall, &DrawTile, &NoWalkPixel, &TgaFile]( gcx_pass_t PassType )
	{
		for( eg_int x = xMin - 1; x <= xMax; x++ )
		{
			for( eg_int y = yMin; y <= yMax + 1; y++ )
			{
				eg_uint GraphicXStart = ( x - xMin )*AUTOMAP_TILE_WIDTH;
				eg_uint GraphicYStart = ( yMax - y + 1 )*AUTOMAP_TILE_WIDTH - 1;

				const gcxTile& Tile = Floor->GetTile( x, y, true );

				switch( PassType )
				{
				case gcx_pass_t::GROUND:
				case gcx_pass_t::GROUND_BLOCKED:
				{
					const gcxBuildTileData& GroundInfo = EGGcxBuildData::Get().GetInfoForTerrain( Tile.Terrain );
					const gcxBuildMaterialData& MtrlType = EGGcxBuildData::Get().GetInfoForMaterial( eg_string_crc(GroundInfo.GroundMaterial) );

					if( GroundInfo.bHasGround )
					{
						const eg_bool bCanPassThrough = CanPassThrough( GroundInfo.TerrainTypeEnum );
						if( PassType == gcx_pass_t::GROUND && bCanPassThrough )
						{
							DrawTile( GraphicXStart, GraphicYStart, MtrlType, bCanPassThrough );
						}
						else if( PassType == gcx_pass_t::GROUND_BLOCKED && !bCanPassThrough )
						{
							DrawTile( GraphicXStart, GraphicYStart, MtrlType, bCanPassThrough );
						}
					}
				} break;

				case gcx_pass_t::WALLS:
				{
					if( Tile.Right != gcx_edge_t::NONE )
					{
						EGTgaWriter::egPixel WallPixel = EGGcxBuildData::Get().GetInfoForWall( Tile.RightColor ).MapWallColor;
						gcx_what What = gcx_what::NONE;
						if( IsDoor( Tile.Right ) && !IsHiddenDoor( Tile.Right ) )
						{
							if( Tile.Right == gcx_edge_t::EMPTY_DOOR_FRAME )
							{
								What = gcx_what::RIGHT_FRAME;
							}
							else
							{
								What = gcx_what::RIGHT_DOOR;
							}
						}
						else
						{
							What = gcx_what::RIGHT_WALL;
						}
						DrawWall( GraphicXStart + 1, GraphicYStart + 1, What , WallPixel );
					}

					if( Tile.Bottom != gcx_edge_t::NONE )
					{
						EGTgaWriter::egPixel WallPixel = EGGcxBuildData::Get().GetInfoForWall( Tile.BottomColor ).MapWallColor;
						gcx_what What = gcx_what::NONE;
						if( IsDoor( Tile.Bottom ) && !IsHiddenDoor( Tile.Bottom ) )
						{
							if( Tile.Bottom == gcx_edge_t::EMPTY_DOOR_FRAME )
							{
								What = gcx_what::BOTTOM_FRAME;
							}
							else
							{
								What = gcx_what::BOTTOM_DOOR;
							}
						}
						else
						{
							What = gcx_what::BOTTOM_WALL;
						}
						DrawWall( GraphicXStart + 1, GraphicYStart + 1, What , WallPixel );
					}
				} break;
				}
			}
		}
	};

	DoPass( gcx_pass_t::GROUND );
	DoPass( gcx_pass_t::GROUND_BLOCKED );
	DoPass( gcx_pass_t::WALLS );

	EGFileData TgaDataFile( eg_file_data_init_t::HasOwnMemory );
	TgaDataFile.Write( TgaFile.GetTGA() , TgaFile.GetSize() );
	eg_bool bSucc = SaveCb( Filename , TgaDataFile );
	assert( bSucc );
}

eg_bool EGGcxRegion::CanPassThrough( gcx_edge_t TileType )
{
	return TileType == gcx_edge_t::NONE || IsDoor( TileType );
}

eg_bool EGGcxRegion::CanPassThrough( gcx_terrain_t TerrainType )
{
	return EGGcxBuildData::Get().GetInfoForTerrain( TerrainType ).bCanWalkOn;
}

void EGGcxRegion::GenerateGraphs()
{
	for( eg_uint FloorIndex = 0; FloorIndex < m_Floors.Len(); FloorIndex++ )
	{
		const gcxFloor& Floor = m_Floors[FloorIndex];

		if( !Floor.HasData() )continue;

		eg_int xMin = 0;
		eg_int yMin = 0;
		eg_int xMax = 0;
		eg_int yMax = 0;

		Floor.GetEffectiveBounds( &xMin, &xMax, &yMin, &yMax );

		eg_int Width = xMax - xMin + 1;
		eg_int Height = yMax - yMin + 1;

		eg_uint FirstVertexForFloor = static_cast<eg_uint>( m_NavGraph.Len() );

		auto CoordToGraphIndex = [FirstVertexForFloor, xMin, yMin, Width, Height]( eg_int x, eg_int y )
		{
			eg_uint IndexOut = FirstVertexForFloor + ( ( x - xMin )*(Height)+( y - yMin ) ) + 1;
			return IndexOut;
		};

		for( eg_int x = xMin; x <= xMax; x++ )
		{
			for( eg_int y = yMin; y <= yMax; y++ )
			{
				const gcxBuildTileData& GroundInfo = EGGcxBuildData::Get().GetInfoForTerrain( Floor.GetTerrainType( x, y ) );

				eg_aabb BoundsTile = Floor.GetTileBounds( x, y );

				eg_uint TileId = CoordToGraphIndex( x, y );
				assert( TileId == ( m_NavGraph.Len() + 1 ) );

				gcxGraphVertex V( CT_Default );
				V.Pos = BoundsTile.GetCenter();
				V.Pos.y = GroundInfo.TopOffset + GroundInfo.NavGraphOffset;
				V.Id = TileId;

				// Now see if the vertex is connected

				// First can only pass through if this is terrain that can be passed through.

				if( GroundInfo.bCanWalkOn )
				{
					//N:
					if( y < yMax && CanPassThrough( Floor.GetN( x, y ) ) && CanPassThrough( Floor.GetTerrainType( x, y + 1 ) ) )
					{
						V.Edges[V.EdgesCount] = CoordToGraphIndex( x, y + 1 );
						V.EdgesCount++;
					}

					//E:
					if( x < xMax && CanPassThrough( Floor.GetE( x, y ) ) && CanPassThrough( Floor.GetTerrainType( x + 1, y ) ) )
					{
						V.Edges[V.EdgesCount] = CoordToGraphIndex( x + 1, y );
						V.EdgesCount++;
					}

					//S:
					if( y > yMin && CanPassThrough( Floor.GetS( x, y ) ) && CanPassThrough( Floor.GetTerrainType( x, y - 1 ) ) )
					{
						V.Edges[V.EdgesCount] = CoordToGraphIndex( x, y - 1 );
						V.EdgesCount++;
					}

					//W:
					if( x > xMin && CanPassThrough( Floor.GetW( x, y ) ) && CanPassThrough( Floor.GetTerrainType( x - 1, y ) ) )
					{
						V.Edges[V.EdgesCount] = CoordToGraphIndex( x - 1, y );
						V.EdgesCount++;
					}
				}

				m_NavGraph.Append( V );
			}
		}
	}
}

void EGGcxRegion::FORMAT_VEC4_AS_VEC3( eg_cpstr8 Fmt, const eg_vec4& v, eg_bool bBase64, eg_string_base& strLine )
{
	eg_string_big s;

	if( bBase64 )
	{
		s = EGString_Format( "%s", *EGBase64_Encode( &v, sizeof( eg_real ) * 3 ) );
	}
	else
	{
		s = EGString_Format( "%g %g %g", v.x, v.y, v.z );
	}

	strLine = EGString_Format( Fmt, s.String() );
}

void EGGcxRegion::HandleXmlTag( const eg_string_base& Tag, const EGXmlAttrGetter& Atts )
{
	if( "region" == Tag )
	{
		eg_uint       FloorCount = Atts.GetUInt( "floors" );
		eg_int        LowestFloor = Atts.GetInt( "lowest_floor" );
		eg_string_big GridShapeStr = Atts.GetString( "grid_shape" );
		gcx_grid_t    GridShape = gcx_grid_t::UNKNOWN;

		if( "square" == GridShapeStr )
		{
			GridShape = gcx_grid_t::SQUARE;
		}
		else
		{
			assert( false ); // Not a valid grid type for this compiler.
		}

		Init( FloorCount, LowestFloor, GridShape );
	}
	else if( "setup" == Tag )
	{
		eg_string_big Origin = Atts.GetString( "origin" );
		if( "bl" == Origin )
		{
			m_Origin = gcx_tile_origin::BOTTOM_LEFT;
		}
	}
	else if( "floor" == Tag )
	{
		m_CurFloorIndex = Atts.GetInt( "index" );
		eg_uint ArrayIndex = FloorIndexToArrayIndex( m_CurFloorIndex );
		assert( 0 <= ArrayIndex && ArrayIndex < m_Floors.Len() );
	}
	else if( "tiles" == Tag )
	{
		//Nothing to do.
	}
	else if( "bounds" == Tag )
	{
		eg_uint FloorIndex = FloorIndexToArrayIndex( m_CurFloorIndex );
		//<bounds x0="14" y0="0" width="1" height="1" />
		eg_int x0 = Atts.GetInt( "x0" );
		eg_int y0 = Atts.GetInt( "y0" );
		eg_int Width = Atts.GetInt( "width" );
		eg_int Height = Atts.GetInt( "height" );
		m_Floors[FloorIndex].SetBounds( x0, y0, Width, Height );
	}
	else if( "row" == Tag )
	{
		eg_uint FloorIndex = FloorIndexToArrayIndex( m_CurFloorIndex );
		m_CurRowIndex = Atts.GetInt( "y" );
		m_CurColIndex = m_Floors[FloorIndex].x0;
	}
	else if( "t" == Tag )
	{
		gcxTile NewTile;
		NewTile.Clear();
		NewTile.Right = static_cast<gcx_edge_t>( Atts.GetInt( "r" ) );
		NewTile.Bottom = static_cast<gcx_edge_t>( Atts.GetInt( "b" ) );
		NewTile.RightColor = static_cast<eg_uint>( Atts.GetInt( "rc" ) );
		NewTile.BottomColor = static_cast<eg_uint>( Atts.GetInt( "bc" ) );
		NewTile.Terrain = static_cast<gcx_terrain_t>( Atts.GetInt( "t" ) );
		NewTile.MarkerType = static_cast<gcx_marker_t>( Atts.GetInt( "m" ) );
		NewTile.DarkEffect = static_cast<gcx_dark_effect_t>( Atts.GetInt( "d" ) );
		NewTile.CeilingType = static_cast<gcx_ceiling_t>( Atts.GetInt( "c" ) );
		NewTile.bHasCustomTile = false;
		if( Atts.DoesAttributeExist( "tcc" ) )
		{
			NewTile.bHasCustomTile = true;
			NewTile.CustomTileIndex = static_cast<eg_uint>( Atts.GetUInt( "tcc" ) );
		}
		else if( Atts.DoesAttributeExist( "mcc" ) )
		{
			NewTile.bHasCustomTile = true;
			NewTile.CustomTileIndex = static_cast<eg_uint>( Atts.GetUInt( "mcc" ) );
		}
		//EGLogf( "Found Tile at %i.(%i,%i)->(%i,%i)." , m_CurFloorIndex , m_CurColIndex , m_CurRowIndex , NewTile.Right,NewTile.Bottom );
		eg_uint FloorIndex = FloorIndexToArrayIndex( m_CurFloorIndex );
		m_Floors[FloorIndex].SetTile( m_CurColIndex, m_CurRowIndex, NewTile );
		m_CurColIndex++;
	}
	else if( "note" == Tag )
	{
		gcxNote NewNote;
		NewNote.TileCoord.x = Atts.GetInt( "x" );
		NewNote.TileCoord.y = Atts.GetInt( "y" );
		m_Floors[m_CurFloorIndex].Notes.Append( NewNote );
	}
}

void EGGcxRegion::HandleXmlData( const eg_string_base& Tag, eg_cpstr Data, eg_uint Len )
{
	if( "note" == Tag )
	{
		gcxFloor& Floor = m_Floors[m_CurFloorIndex];
		gcxNote& Note = Floor.Notes.Last();
		Note.NoteBody = Data;
		EGStringEx_TrimPadding( Note.NoteBody );
	}
	else if( "name" == Tag )
	{
		m_Name = GetCleanName( Data , Len );
	}
}

void EGGcxRegion::HandleSetDoorKey( const egParseFuncInfoAsEgStrings& CallInfo, eg_int FloorIndex, const gcxNote& RawNote )
{
	gcxFloor& Floor = m_Floors[FloorIndexToArrayIndex( FloorIndex )];
	eg_ivec2 DoorTile = RawNote.TileCoord;

	if( CallInfo.NumParms >= 2 )
	{
		const eg_string_big& DirFromTile = CallInfo.Parms[0];
		const eg_string_big& VarName = CallInfo.Parms[1];
		eg_int LockLevel = CallInfo.NumParms >= 3 ? CallInfo.Parms[2].ToInt() : 0;

		eg_string DirFromTileCap = DirFromTile;
		DirFromTileCap.ConvertToUpper();

		eg_bool bGoodDoor = true;
		eg_bool bRightNotDown = true;
		switch_crc( eg_string_crc( DirFromTileCap ) )
		{
			case_crc( "NORTH" ) :
				DoorTile.y++;
				bRightNotDown = false;
			break;
			case_crc( "EAST" ) :
				bRightNotDown = true;
			break;
			case_crc( "SOUTH" ) :
				bRightNotDown = false;
			break;
			case_crc( "WEST" ) :
				DoorTile.x--;
				bRightNotDown = true;
			break;
		default:
			bGoodDoor = false;
			break;
		}

		if( bGoodDoor )
		{
			if( Floor.DoesTileHaveRealData( DoorTile.x, DoorTile.y ) )
			{
				gcxTile& Tile = Floor.GetTile( DoorTile.x, DoorTile.y );
				if( bRightNotDown )
				{
					if( IsDoor( Tile.Right ) )
					{
						Tile.ExRightExtraCrc = EGCrcDb::StringToCrc( VarName );
						Tile.ExRightExtraInt = LockLevel;
					}
					else
					{
						bGoodDoor = false;
					}
				}
				else
				{
					if( IsDoor( Tile.Bottom ) )
					{
						Tile.ExBottomExtraCrc = EGCrcDb::StringToCrc( VarName );
						Tile.ExBottomExtraInt = LockLevel;
					}
					else
					{
						bGoodDoor = false;
					}
				}
			}
			else
			{
				bGoodDoor = false;
			}
		}

		if( !bGoodDoor )
		{
			EGLogf( eg_log_t::Warning, "HandleSetDoorKey: No door where specified (no door, bad direction, etc)." );
		}
	}
}

void EGGcxRegion::HandleNoteFunctionCall( const egParseFuncInfoAsEgStrings& CallInfo, eg_int FloorIndex, const gcxNote& RawNote )
{
	// Could clean up the RawNote since it probably has a bunch of padding around it.
	if( "gcx" == CallInfo.SystemName || "map" == CallInfo.SystemName )
	{
		if( CallInfo.FunctionName.EqualsI( "SetDoorKey" ) )
		{
			HandleSetDoorKey( CallInfo, FloorIndex, RawNote );
		}
		else
		{
			AddFunctionCallTag( FloorIndex, RawNote.TileCoord.x, RawNote.TileCoord.y, CallInfo );
		}
	}
}

void EGGcxRegion::HandlePropertyFunctionCall( const egParseFuncInfoAsEgStrings& CallInfo, eg_int FloorIndex, const gcxNote& RawNote )
{
	unused( FloorIndex , RawNote );

	// Could clean up the RawNote since it probably has a bunch of padding around it.
	if( "properties" == CallInfo.SystemName )
	{
		eg_string_crc FnAsCrc = eg_string_crc( CallInfo.FunctionName );
		switch_crc( FnAsCrc )
		{
			case_crc( "SetTerrain" ) :
			{
				EGLogf( eg_log_t::Warning, "SetTerrain not supported." );
			} break;
			case_crc( "SetScript" ) :
			{
				EGLogf( eg_log_t::Warning, "SetScript not supported." );
			} break;
			case_crc( "SetDayNightCycle" ) :
			{
				EGLogf( eg_log_t::Warning, "SetDayNightCycle not supported." );
			} break;
			case_crc( "SetSkybox" ) :
			{
				EGLogf( eg_log_t::Warning, "SetSkybox not supported." );
			} break;
			case_crc( "SetBuildData" ) :
			{
				EGLogf( eg_log_t::Warning, "SetBuildData no longer supported, put build data in the .BuildConfig file." );
			} break;
		}
	}
}

void EGGcxRegion::ProcessNotes()
{
	for( const gcxFloor& Floor : m_Floors )
	{
		for( const gcxNote& Note : Floor.Notes )
		{
			EGParse_ProcessFnCallScript( *Note.NoteBody, Note.NoteBody.Len(), [this, &Note, &Floor]( const egParseFuncInfo& InfoStr )->void
			{
				egParseFuncInfoAsEgStrings Info( InfoStr );
				HandleNoteFunctionCall( Info, Floor.Index, Note );
			} );
		}
	}
}

eg_d_string EGGcxRegion::GetNoteAt( eg_int FloorIndex , eg_int x , eg_int y ) const
{
	eg_d_string Out = "";

	const gcxFloor* Floor = nullptr;
	for( const gcxFloor& CompareFloor : m_Floors )
	{
		if( CompareFloor.Index == FloorIndex )
		{
			Floor = &CompareFloor;
		}
	}

	if( Floor )
	{
		for( const gcxNote& Note : Floor->Notes )
		{
			if( Note.TileCoord.x == x && Note.TileCoord.y == y )
			{
				Out = Note.NoteBody;
			}
		}
	}

	return Out;
}

void EGGcxRegion::ProcessProperties()
{
	for( const gcxFloor& Floor : m_Floors )
	{
		for( const gcxNote& Note : Floor.Notes )
		{
			EGParse_ProcessFnCallScript( *Note.NoteBody, Note.NoteBody.Len(), [this, &Note, &Floor]( const egParseFuncInfo& InfoStr )->void
			{
				egParseFuncInfoAsEgStrings Info( InfoStr );
				HandlePropertyFunctionCall( Info, Floor.Index, Note );
			} );
		}
	}
}

void EGGcxRegion::ProcessMarkers()
{
	for( gcxFloor& Floor : m_Floors )
	{
		auto CalculateAutoTransition = []( const gcxFloor& Floor , eg_int x , eg_int y , gcxTile& Tile ) -> void
		{
			// If there is an arrow pointing at this tile then it is an auto
			// transition and the transition is in the direction specified.
			const gcxTile UpTile = Floor.GetTile( x , y + 1 , true );
			const gcxTile DownTile = Floor.GetTile( x , y - 1 , true );
			const gcxTile RightTile = Floor.GetTile( x + 1 , y , true );
			const gcxTile LeftTile = Floor.GetTile( x - 1 , y , true );

			if( UpTile.MarkerType == gcx_marker_t::ARROW_DOWN )
			{
				Tile.ExAutoTransition.bHasAutoTransition = true;
				Tile.ExAutoTransition.Directon = gcx_direction::DOWN;
			}
			else if( DownTile.MarkerType == gcx_marker_t::ARROW_UP )
			{
				Tile.ExAutoTransition.bHasAutoTransition = true;
				Tile.ExAutoTransition.Directon = gcx_direction::UP;
			}
			else if( LeftTile.MarkerType == gcx_marker_t::ARROW_RIGHT )
			{
				Tile.ExAutoTransition.bHasAutoTransition = true;
				Tile.ExAutoTransition.Directon = gcx_direction::RIGHT;
			}
			else if( RightTile.MarkerType == gcx_marker_t::ARROW_LEFT )
			{
				Tile.ExAutoTransition.bHasAutoTransition = true;
				Tile.ExAutoTransition.Directon = gcx_direction::LEFT;
			}
		};

		for( eg_int x = Floor.x0; x < ( Floor.x0 + Floor.Width ); x++ )
		{
			for( eg_int y = Floor.y0; y < ( Floor.y0 + Floor.Height ); y++ )
			{
				gcxTile& Tile = Floor.GetTile( x , y );

				switch( Tile.MarkerType )
				{
					case gcx_marker_t::START:
					case gcx_marker_t::EXIT:
					case gcx_marker_t::STAIRS_UP:
					case gcx_marker_t::STAIRS_DOWN:
					case gcx_marker_t::LADDER_UP:
					case gcx_marker_t::LADDER_DOWN:
					{
						CalculateAutoTransition( Floor , x , y , Tile );
					} break;
					case gcx_marker_t::SHAPE_CROSS:
					{
						Tile.bIsSafe = true;
					} break;
					default:
					{
						// Nothing to do for this marker type, cosmetic in editor only.
					} break;
				}
			}
		}
	}
}

eg_bool EGGcxRegion::SaveToDir( eg_cpstr OutDir , EGGcxSaveCb& SaveCb , EGGcxWriteMtrlCb& WriteMaterialCb , gcxExtraMapProps& ExtraPropsOut , eg_bool bCompress )
{
	EGFileData File( eg_file_data_init_t::HasOwnMemory );
	eg_d_string CleanOutDir = EGPath2_CleanPath( OutDir , '\\' );
	EGOsFile_CreateDirectory( *CleanOutDir );
	SaveAsXmlEmap( *CleanOutDir, File , SaveCb , WriteMaterialCb , ExtraPropsOut );

	eg_string_big FinalFilename = EGString_Format( "%s%s.%s", *CleanOutDir, *m_Name , bCompress ? "emapc" : "emap" );
	FinalFilename.ConvertToLower();

	if( bCompress )
	{
		EGArray<eg_byte> FileData;
		FileData.Append( File.GetDataAs<eg_byte>() , File.GetSize() );
		if( FileData.Len() != File.GetSize() )
		{
			return false;
		}

		File.Clear();

		eg_bool bCompressed = EGCompress_CompressData( FileData , FileData );
		if( bCompressed )
		{
			File.Write( FileData.GetArray(), FileData.Len() );
			if( File.GetSize() != FileData.Len() )
			{
				return false;
			}
		}
		else
		{
			return false;
		}

#if 0
		// A simple test to make sure our decompression works.
		if( true )
		{
			EGArray<eg_byte> TestDecompress;
			eg_bool bDecompressed = EGCompress_DecompressData( FileData, TestDecompress );
			if( bDecompressed )
			{
				eg_d_string TestFinalFilename = EGString_Format( "%s%s_decompress.emap", *CleanOutDir, *m_Name );
				EGFileData TestFile;
				TestFile.Write( TestDecompress.GetArray(), TestDecompress.Len() );
				SaveCb( *TestFinalFilename, TestFile );
			}
		}
#endif
	}

	return SaveCb( FinalFilename , File );
}

void EGGcxRegion::AddTorch( eg_uint Region, const eg_vec4& TorchPos, const eg_real RotAboutY, const gcxBuildWallData& ThisWallInfo )
{
	eg_transform TorchPose = eg_transform::BuildRotationY( EG_Deg(RotAboutY) );
	TorchPose.TranslateThis( TorchPos.XYZ_ToVec3() );
	gcxLight NewLight( Region, TorchPose, eg_color( eg_color32( ThisWallInfo.TorchColor ) ), ThisWallInfo.TorchRange );
	m_Lights.Append( NewLight );
	if( ThisWallInfo.TorchEntityDefinition.Path.Len() > 0 )
	{
		AddMapEntity( *ThisWallInfo.TorchEntityDefinition.Path, "", TorchPos, eg_vec4( 0.f, RotAboutY, 0.f, 0.f ) );
	}
}

eg_bool EGGcxRegion::SaveAsXmlEmap( eg_cpstr OutDir, EGFileData& Out , EGGcxSaveCb& SaveCb , EGGcxWriteMtrlCb& WriteMaterialCb , gcxExtraMapProps& ExtraPropsOut )
{
	gcxExtraMapProps Properties;

	// Set up some properties:
	{
		eg_string MapFolder = OutDir;
		MapFolder.ClampEnd( 1 ); // Get rid of trailing slash
		MapFolder.SetToFilenameFromPathNoExt( MapFolder );

		eg_string_big LevelPath = EGString_Format( "%s", *m_Name );
		LevelPath.ConvertToLower();
		eg_string_big MiniMapPath = EGString_Format( "%s_automap", *m_Name );
		MiniMapPath.ConvertToLower();

		Properties.Name = m_Name;
		Properties.Level = LevelPath;
		Properties.MiniMap = MiniMapPath;
		Properties.TileSize = eg_vec2( EGGcxBuildData::Get().GetWallLength() , EGGcxBuildData::Get().GetWallLength() );
	}

	eg_aabb Bounds( CT_Default );

	m_WorldGeometry.Clear();

	ComputeMapRegions();

	for( eg_uint FloorIndex = 0; FloorIndex < m_Floors.Len(); FloorIndex++ )
	{
		gcxFloor* Floor = &m_Floors[FloorIndex];

		if( !Floor->HasData() )
		{
			continue;
		}

		eg_int xMinEf = 0;
		eg_int xMaxEf = 0;
		eg_int yMinEf = 0;
		eg_int yMaxEf = 0;
		Floor->GetEffectiveBounds( &xMinEf, &xMaxEf, &yMinEf, &yMaxEf );

		if( Floor->Index == 0 )
		{
			Properties.LowerLeftOrigin = eg_vec2( static_cast<eg_real>( xMinEf ), static_cast<eg_real>( yMinEf ) );
			Properties.MapSize = eg_vec2( static_cast<eg_real>( xMaxEf - xMinEf + 1 ), static_cast<eg_real>( yMaxEf - yMinEf + 1 ) );
		}

		for( eg_int x = Floor->x0; x < ( Floor->x0 + Floor->Width + 1 ); x++ )
		{
			for( eg_int y = Floor->y0; y < ( Floor->y0 + Floor->Height + 1 ); y++ )
			{
				const eg_aabb BoundsTile = Floor->GetTileBounds( x, y );

				// Add a pillar if this corner touches any wall
				if( Floor->TouchesWall( x, y ) ) //This is the lower left of the tile
				{
					eg_uint PillarType = Floor->GetPillarType( x, y );
					const eg_real WALL_HEIGHT = EGGcxBuildData::Get().GetInfoForWall( PillarType ).Height;
					const eg_real PILLAR_THICKNESS = EGGcxBuildData::Get().GetInfoForWall( PillarType ).PillarThickness;
					if( !EG_IsEqualEps( PILLAR_THICKNESS , 0 ) )
					{
						eg_vec4 WallMin = BoundsTile.Min + eg_vec4( -PILLAR_THICKNESS*.5f, 0, -PILLAR_THICKNESS*.5f, 0.f );
						eg_vec4 WallMax = WallMin + eg_vec4( PILLAR_THICKNESS, WALL_HEIGHT, PILLAR_THICKNESS, 0 );
						WriteWall( x, y, gcx_edge_t::WALL, WallMin, WallMax, &EGGcxBuildData::Get().GetInfoForWall( PillarType ), Floor->GetPillarLowerOffset( x, y ) , EGGcxBuildData::Get().GetInfoForWall( PillarType ).PillarMaterial );
					}
				}

				const gcxTile& Tile = Floor->GetTile( x, y, true );

				if( Tile.Bottom > gcx_edge_t::NONE )
				{
					const gcxBuildWallData& ThisWallInfo = EGGcxBuildData::Get().GetInfoForWall( Tile.BottomColor );

					const eg_real WALL_LENGTH = EGGcxBuildData::Get().GetWallLength();
					const eg_real WALL_HEIGHT = ThisWallInfo.Height;
					const eg_real PILLAR_THICKNESS = ThisWallInfo.PillarThickness;
					const eg_real WALL_THICKNESS = ThisWallInfo.WallThickness;
					const eg_real TORCH_DIST_FROM_WALL = ThisWallInfo.TorchDistanceFromWall;
					const eg_color TORCH_COLOR = eg_color(ThisWallInfo.TorchColor);
					const eg_real TORCH_RANGE = ThisWallInfo.TorchRange;

					eg_aabb WallBounds;
					WallBounds.Min = BoundsTile.Min + eg_vec4( PILLAR_THICKNESS*0.5f, 0, -WALL_THICKNESS*0.5f, 0.f );
					WallBounds.Max = WallBounds.Min + eg_vec4( WALL_LENGTH - PILLAR_THICKNESS, WALL_HEIGHT, WALL_THICKNESS, 0 );
					WriteWall( x, y, Tile.Bottom, WallBounds.Min, WallBounds.Max, &ThisWallInfo, Floor->GetBottomWallLowerOffset( x, y ) , ThisWallInfo.PillarMaterial );

					if( HasSouthOrEastSideTorch( Tile.Bottom , ThisWallInfo.bDoorsHaveTorches ) ) //Torch on south side
					{
						eg_vec4 TorchPos = WallBounds.GetCenter();
						TorchPos.z -= ( WALL_THICKNESS*.5f + TORCH_DIST_FROM_WALL );
						if( IsDoor( Tile.Bottom ) )
						{
							TorchPos.y = ThisWallInfo.GetTorchDistanceFromGroundForDoor( Tile.Bottom );
						}
						else
						{
							TorchPos.y = ThisWallInfo.TorchDistanceFromGround;
						}

						AddTorch( GetEMapRegionForTilePos( x, y ), TorchPos, 0.f, ThisWallInfo );
					}

					if( HasNorthOrWestSideTorch( Tile.Bottom , ThisWallInfo.bDoorsHaveTorches ) ) //Torch on North side
					{
						eg_vec4 TorchPos = WallBounds.GetCenter();
						TorchPos.z += ( WALL_THICKNESS*.5f + TORCH_DIST_FROM_WALL );
						if( IsDoor( Tile.Bottom ) )
						{
							TorchPos.y = ThisWallInfo.GetTorchDistanceFromGroundForDoor( Tile.Bottom );
						}
						else
						{
							TorchPos.y = ThisWallInfo.TorchDistanceFromGround;
						}

						AddTorch( GetEMapRegionForTilePos( x, y ), TorchPos, 180.f, ThisWallInfo );
					}

					if( IsDoor( Tile.Bottom ) && Tile.Bottom != gcx_edge_t::EMPTY_DOOR_FRAME )
					{
						const eg_bool bHiddenDoor = IsHiddenDoor( Tile.Bottom );
						const eg_real DOOR_WIDTH = IsHiddenDoor( Tile.Bottom ) ? ThisWallInfo.SecretDoorData.Width : ThisWallInfo.StandardDoorData.Width;
						const eg_real DOOR_OFFSET = ( WALL_LENGTH - DOOR_WIDTH - PILLAR_THICKNESS )*.5f;
						
						eg_vec4 Pos = WallBounds.Min;
						Pos.x += DOOR_OFFSET;
						Pos.z += WALL_THICKNESS*.5f;
						eg_vec4 Rot = eg_vec4( 0, 0, 0, 0 );
						eg_string DoorLockedStr = "";
						eg_bool bWasSetAsLocked = false;
						if( Tile.Bottom == gcx_edge_t::LOCKED_DOOR )
						{
							DoorLockedStr += "SetLocked( true );";
							bWasSetAsLocked = true;
						}
						if( Tile.ExBottomExtraCrc.IsNotNull() )
						{
							// If it has a key it should be locked.
							if( !bWasSetAsLocked )
							{
								DoorLockedStr += "SetLocked( true );";
							}
							DoorLockedStr += EGString_Format( "SetKeyId( %s );", EGCrcDb::CrcToString( Tile.ExBottomExtraCrc ).String() );
						}
						if( Tile.ExBottomExtraInt > 0 )
						{
							DoorLockedStr += EGString_Format( "SetLockLevel( %d );", Tile.ExBottomExtraInt );
						}
						AddMapEntity( bHiddenDoor ? *ThisWallInfo.SecretDoorData.EntityDefinition.Path : *ThisWallInfo.StandardDoorData.EntityDefinition.Path, DoorLockedStr, Pos, Rot );
					}
				}

				if( Tile.Right > gcx_edge_t::NONE )
				{
					const gcxBuildWallData& ThisWallInfo = EGGcxBuildData::Get().GetInfoForWall( Tile.RightColor );

					const eg_real WALL_LENGTH = EGGcxBuildData::Get().GetWallLength();
					const eg_real WALL_HEIGHT = ThisWallInfo.Height;
					const eg_real PILLAR_THICKNESS = ThisWallInfo.PillarThickness;
					const eg_real WALL_THICKNESS = ThisWallInfo.WallThickness;
					const eg_real TORCH_DIST_FROM_WALL = ThisWallInfo.TorchDistanceFromWall;
					const eg_color TORCH_COLOR = eg_color(ThisWallInfo.TorchColor);
					const eg_real TORCH_RANGE = ThisWallInfo.TorchRange;

					eg_aabb WallBounds;
					WallBounds.Min = BoundsTile.Min + eg_vec4( WALL_LENGTH - WALL_THICKNESS*0.5f, 0, PILLAR_THICKNESS*0.5f, 0 );
					WallBounds.Max = WallBounds.Min + eg_vec4( WALL_THICKNESS, WALL_HEIGHT, WALL_LENGTH - PILLAR_THICKNESS, 0 );
					WriteWall( x, y, Tile.Right, WallBounds.Min, WallBounds.Max, &ThisWallInfo, Floor->GetRightWallLowerOffset( x, y ) , ThisWallInfo.PillarMaterial );

					if( HasSouthOrEastSideTorch( Tile.Right , ThisWallInfo.bDoorsHaveTorches ) ) //Torch on east side
					{
						eg_vec4 TorchPos = WallBounds.GetCenter();
						TorchPos.x += ( WALL_THICKNESS*.5f + TORCH_DIST_FROM_WALL );
						if( IsDoor( Tile.Right ) )
						{
							TorchPos.y = ThisWallInfo.GetTorchDistanceFromGroundForDoor( Tile.Right );
						}
						else
						{
							TorchPos.y = ThisWallInfo.TorchDistanceFromGround;
						}

						AddTorch( GetEMapRegionForTilePos( x, y ), TorchPos, 270.f, ThisWallInfo );
					}

					if( HasNorthOrWestSideTorch( Tile.Right , ThisWallInfo.bDoorsHaveTorches ) ) //Torch on west side
					{
						eg_vec4 TorchPos = WallBounds.GetCenter();
						TorchPos.x -= ( WALL_THICKNESS*.5f + TORCH_DIST_FROM_WALL );
						if( IsDoor( Tile.Right ) )
						{
							TorchPos.y = ThisWallInfo.GetTorchDistanceFromGroundForDoor( Tile.Right );
						}
						else
						{
							TorchPos.y = ThisWallInfo.TorchDistanceFromGround;
						}

						AddTorch( GetEMapRegionForTilePos( x, y ), TorchPos, 90.f, ThisWallInfo );
					}

					if( IsDoor( Tile.Right ) && Tile.Right != gcx_edge_t::EMPTY_DOOR_FRAME  )
					{
						const eg_bool bHiddenDoor = IsHiddenDoor( Tile.Right );
						const eg_real DOOR_WIDTH = IsHiddenDoor( Tile.Right ) ? ThisWallInfo.SecretDoorData.Width : ThisWallInfo.StandardDoorData.Width;
						const eg_real DOOR_OFFSET = ( WALL_LENGTH - DOOR_WIDTH - PILLAR_THICKNESS )*.5f;
						
						eg_vec4 Pos = WallBounds.Min;
						Pos.z += DOOR_OFFSET;
						Pos.x += WALL_THICKNESS*.5f;
						eg_vec4 Rot = eg_vec4( 0, -90.f, 0, 0 );
						eg_string DoorLockedStr;
						eg_bool bWasSetAsLocked = false;
						if( Tile.Right == gcx_edge_t::LOCKED_DOOR )
						{
							DoorLockedStr += "SetLocked( true );";
							bWasSetAsLocked = true;
						}
						if( Tile.ExRightExtraCrc.IsNotNull() )
						{
							// If it has a key it should be locked.
							if( !bWasSetAsLocked )
							{
								DoorLockedStr += "SetLocked( true );";
							}
							DoorLockedStr += EGString_Format( "SetKeyId( %s );", EGCrcDb::CrcToString( Tile.ExRightExtraCrc ).String() );;
						}
						if( Tile.ExRightExtraInt > 0 )
						{
							DoorLockedStr += EGString_Format( "SetLockLevel( %d );", Tile.ExRightExtraInt );
						}
						AddMapEntity( bHiddenDoor ? *ThisWallInfo.SecretDoorData.EntityDefinition.Path : *ThisWallInfo.StandardDoorData.EntityDefinition.Path, DoorLockedStr, Pos, Rot );
					}
				}

				if( Tile.MarkerType > gcx_marker_t::NONE )
				{
					if( gcx_marker_t::MONSTER == Tile.MarkerType )
					{
						AddNoteTag( x, y, "monster", *GetNoteAt( FloorIndex , x , y ) );
					}
				}

				if( Tile.ExAutoTransition.bHasAutoTransition )
				{		
					// AddFunctionStringTag( FloorIndex , x , y , EGString_Format( "map.CreateAutoTransition(%d,%d)" , static_cast<eg_int>(Tile.MarkerType) , static_cast<eg_int>(Tile.ExAutoTransition.Directon) ) );
				}

				if( Tile.bIsSafe || ( EGGcxBuildData::Get().GetAnyTileWithMarkerIsSafe() && Tile.MarkerType != gcx_marker_t::NONE ) )
				{
					if( Tile.MarkerType != gcx_marker_t::MONSTER || EGGcxBuildData::Get().GetTilesWithMonstersAreSafe() )
					{
						AddFunctionStringTag( FloorIndex , x , y , "map.SetSafeSpot()" );
					}
				}

				// Floor and roof only if within the effective bounds

				if( ( xMinEf <= x && x <= xMaxEf ) && ( yMinEf <= y && y <= yMaxEf ) )
				{
					const gcxBuildTileData& GroundInfo = EGGcxBuildData::Get().GetInfoForTerrain( Tile.Terrain );

					//Put in ground
					if( GroundInfo.bHasGround )
					{
						eg_vec4 WallMin = BoundsTile.Min;
						eg_vec4 WallMax = BoundsTile.Max;
						WallMax.y = BoundsTile.Min.y + GroundInfo.TopOffset;
						WallMin.y = BoundsTile.Min.y + GroundInfo.BottomOffset;

						WriteWall( x, y, gcx_edge_t::WALL, WallMin, WallMax, nullptr , 0.f, GroundInfo.GroundMaterial );
					}

					//Put in roof (may need to change this if we do multi-level maps)
					if( Tile.CeilingType == gcx_ceiling_t::HAS_CEILING || EGGcxBuildData::Get().GetAllTilesHaveCeilings() )
					{
						eg_vec4 WallMin = BoundsTile.Min;
						eg_vec4 WallMax = BoundsTile.Max;
						WallMin.y = WallMax.y = GroundInfo.CeilingHeight;
						if( GroundInfo.CeilingThickness >= 0.f )
						{
							WallMax.y += GroundInfo.CeilingThickness;
						}
						else
						{
							WallMin.y += GroundInfo.CeilingThickness;
						}
						WriteWall( x, y, gcx_edge_t::WALL, WallMin, WallMax, nullptr, 0.f, GroundInfo.CeilingMaterial);
					}
				}
			}
		}
	}

	GenerateWorldGeometry();

	// Calculate whole world bounds:
	{
		Bounds = CT_Default;
		eg_bool bGotFirst = false;
		for( gcxEMapRegion& Region : m_EMapRegions )
		{
			if( !bGotFirst )
			{
				bGotFirst = true;
				Bounds = Region.Bounds;
			}
			else
			{
				Bounds.AddBox( Region.Bounds );
			}
		}
	}

	// Compute tangents for the world geometry.
	for( const gcxEMapSegment& Segment : m_EMapSegments )
	{
		const eg_size_t VertsCount = Segment.TriangleCount * 3;
		egv_index* Indexes = new egv_index[VertsCount];
		for( eg_size_t in = 0; in < VertsCount; in++ )
		{
			Indexes[in] = static_cast<egv_index>( in );
		}
		EGVertex_ComputeTangents( &m_EMapVertexes.GetArray()[Segment.FirstVertex], Indexes, static_cast<eg_uint>( VertsCount ), static_cast<eg_uint>( Segment.TriangleCount ) );
		delete[] Indexes;
	}

	GenerateVertexLighting();

	eg_string_big AutoMapFilename = EGString_Format( "%s_automap", *m_Name );
	eg_string_big AutoMapFilenameFull = EGString_Format( "%s%s.tga", OutDir, AutoMapFilename.String(), *m_Name );
	AutoMapFilename.ConvertToLower();
	AutoMapFilenameFull.ConvertToLower();
	GenerateAutoMap( AutoMapFilenameFull , SaveCb );


	GenerateGraphs();

	eg_string_big Line;

	auto WRITE = [&Out]( const eg_string_big& Line )
	{
		Out.Write( Line.String(), Line.Len() );
	};

	WRITE( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );

	Line = "";
	Line += EGString_Format( "<emap\r\n" );
	Line += EGString_Format( "\tversion=\"2\"\r\n" );
	Line += EGString_Format( "\tformat=\"base64\"\r\n" );
	Line += EGString_Format( "\tname=\"%s\"\r\n", *m_Name );
	Line += EGString_Format( ">\r\n" );
	WRITE( Line );

	WRITE( "\r\n" );

	Line = EGString_Format( "\t<bounds min=\"%s\" max=\"%s\"/>\r\n", *EGBase64_Encode( &Bounds.Min, sizeof( eg_real ) * 3 ) , *EGBase64_Encode( &Bounds.Max, sizeof( eg_real ) * 3 ) );
	WRITE( Line );

	WRITE( "\r\n" );

	{
		EGArray<gcxBuildMaterialData> AllMtrls;
		EGGcxBuildData::Get().GetAllMaterials( AllMtrls );

		eg_uint i = 0;
		for( const gcxBuildMaterialData& MtrlInfo : AllMtrls )
		{
			Line = WriteMaterialCb( i , MtrlInfo );
			i++;
			WRITE( Line );
		}
	}

	WRITE( "\r\n" );

	for( eg_size_t i = 0; i < m_EMapSegments.Len(); i++ )
	{
		const gcxEMapSegment& Segment = m_EMapSegments[i];
		Line = EGString_Format( "\t<segment id=\"%u\" first_vertex=\"%u\" triangles=\"%u\" material=\"%u\"/>\r\n", i + 1, ( Segment.FirstVertex + 1 ), Segment.TriangleCount, ( static_cast<eg_uint>( Segment.MtrlIndex ) + 1 ) );
		WRITE( Line );
	}

	WRITE( "\r\n" );

	for( eg_size_t i = 0; i < m_EMapPortals.Len(); i++ )
	{
		const gcxEMapPortal& Portal = m_EMapPortals[i];

		Line = EGString_Format( "\t<portal id=\"%u\" to=\"%u\" radius=\"1\" terminal=\"true\" always_visible=\"true\" />\r\n"
			, ( i + 1 )
			, ( Portal.ToRegion + 1 ) );

		WRITE( Line );
	}

	WRITE( "\r\n" );

	for( eg_size_t i = 0; i < m_EMapRegions.Len(); i++ )
	{
		const gcxEMapRegion& Region = m_EMapRegions[i];
		Line = "";
		Line += EGString_Format( "\t<region id=\"%u\" name=\"%s\" first_segment=\"%u\" segments=\"%u\"\r\n", ( i + 1 ), EGString_Format( "region%u", Region.Name ).String(), ( Region.FirstSegment + 1 ), Region.NumSegments );
		Line += EGString_Format( "\t\tfirst_geo_block=\"%u\" geo_blocks=\"%u\"\r\n", ( Region.FirstGeoBrush + 1 ), Region.NumGeoBrushes );
		Line += EGString_Format( "\t\tfirst_light=\"%u\" lights=\"%u\"\r\n", ( Region.FirstLight + 1 ), Region.NumLights );
		Line += EGString_Format( "\t\tfirst_portal=\"%u\" portals=\"%u\"\r\n", ( Region.FirstPortal + 1 ), Region.NumPortals );
		Line += EGString_Format( "\t\tambient=\"%s\"\r\n", *EGBase64_Encode( &eg_color( 1.f, 1.f, 1.f, 1.f ), sizeof( eg_color ) ) );
		Line += EGString_Format( "\t\tbounds_min=\"%s\" bounds_max=\"%s\"/>\r\n", *EGBase64_Encode( &Region.Bounds.Min, sizeof( eg_real ) * 3 ) , *EGBase64_Encode( &Region.Bounds.Max, sizeof( eg_real ) * 3 ) );
		WRITE( Line );
	}

	WRITE( "\r\n" );

	////////////////////////
	/// Write the graphs ///
	////////////////////////

	Line = EGString_Format( "\t<graph id=\"1\" name=\"nav\" type=\"path\" vertexes=\"%u\">\r\n", m_NavGraph.Len() );
	WRITE( Line );

	for( eg_size_t i = 0; i < m_NavGraph.Len(); i++ )
	{
		const gcxGraphVertex& V = m_NavGraph[i];
		Line = EGString_Format( "\t\t<vertex id=\"%u\" pos=\"%s\">\r\n", V.Id, *EGBase64_Encode( &V.Pos, sizeof( eg_real ) * 3 ) );
		for( eg_uint e = 0; e < V.EdgesCount && e < countof( V.Edges ); e++ )
		{
			Line += EGString_Format( "\t\t\t<edge to=\"%u\"/>\r\n", V.Edges[e] );
		}
		Line += "\t\t</vertex>\r\n";
		WRITE( Line );
	}

	Line = EGString_Format( "\t</graph>\r\n", m_NavGraph.Len() );
	WRITE( Line );

	/////////////////////////
	/// Write the lights ///
	/////////////////////////

	Line = "\r\n";
	WRITE( Line );

	for( eg_uint i = 0; i < m_EMapLights.Len(); i++ )
	{
		const gcxLight& Light = m_EMapLights[i];
		float color[4];
		color[0] = Light.Color.r;
		color[1] = Light.Color.g;
		color[2] = Light.Color.b;
		color[3] = Light.Color.a;

		eg_string_big sV;
		FORMAT_VEC4_AS_VEC3( "%s", Light.Pose.GetPosition(), true, sV );

		Line = EGString_Format( "\t<light id=\"%u\" pos=\"%s\" color=\"%s\" range=\"%s\"/>\n",
			i + 1,
			sV.String(),
			*EGBase64_Encode( &color, sizeof( float ) * 4 ),
			*EGBase64_Encode( &Light.Range, sizeof( float ) * 1 ) );

		WRITE( Line );
	}

	Line = "\r\n";
	WRITE( Line );

	/////////////////////////
	/// Write the tags ///
	/////////////////////////

	Line = "\r\n";
	WRITE( Line );

	eg_uint TagIndex = 1;
	for( eg_uint i = 0; i < m_Tags.Len(); i++, TagIndex++ )
	{
		eg_string_big sPos;
		FORMAT_VEC4_AS_VEC3( "%s", m_Tags[i].Pos, true, sPos );
		eg_string_big sRot;
		FORMAT_VEC4_AS_VEC3( "%s", m_Tags[i].Rot, false, sRot );

		eg_string_big ExtraInfoStr = "info";

		if( "entity" == m_Tags[i].Type )
		{
			ExtraInfoStr = "entity";
		}

		Line = EGString_Format( "\t<tag id=\"%u\" pos=\"%s\" rotation=\"%s\" type=\"%s\" data_size=\"%u\" %s=\"%s\"/>\n",
			TagIndex,
			*sPos,
			*sRot,
			*m_Tags[i].Type,
			m_Tags[i].ExtraInfoDataSize,
			*ExtraInfoStr,
			*m_Tags[i].ExtraInfo );

		WRITE( Line );
	}

	for( eg_uint i = 0; i < m_MapEnts.Len(); i++, TagIndex++ )
	{
		eg_string_big sPos;
		FORMAT_VEC4_AS_VEC3( "%s", m_MapEnts[i].Pos, true, sPos );
		eg_string_big sRot;
		FORMAT_VEC4_AS_VEC3( "%s", m_MapEnts[i].Rot, false, sRot );

		Line = EGString_Format( "\t<tag id=\"%u\" pos=\"%s\" rotation=\"%s\" type=\"entity\" entity=\"%s\" init_string=\"%s\"/>\n",
			TagIndex,
			*sPos,
			*sRot,
			m_MapEnts[i].EntDef.String(),
			m_MapEnts[i].ExtraStr.String() );

		WRITE( Line );
	}

	WRITE( "\r\n" );

	for( eg_size_t i = 0; i < m_EMapVertexes.Len(); i++ )
	{
		const egv_vert_mesh& Vert = m_EMapVertexes[i];
		Line = EGVertex_ToXmlTag( Vert, static_cast<eg_uint>( i + 1 ), true );
		WRITE( Line );
	}

	WRITE( "\r\n" );

	WRITE( "\t<geo_brushes>\r\n" );

	for( eg_uint i = 0; i < m_EMapGeoBrushes.Len(); i++ )
	{
		const eg_aabb& BrushAabb = m_EMapGeoBrushes[i].Bounds;
		Line = EGString_Format( "\t\t<aabb min=\"%s\" max=\"%s\"/>\r\n", *EGBase64_Encode( &BrushAabb.Min, sizeof( eg_real ) * 3 ) , *EGBase64_Encode( &BrushAabb.Max, sizeof( eg_real ) * 3 ) );
		WRITE( Line );
	}

	WRITE( "\t</geo_brushes>\r\n" );

	WRITE( "\r\n" );

	Line = "</emap>\r\n";

	WRITE( Line );

	ExtraPropsOut = Properties;

	return true;
}

void EGGcxRegion::SetCustomTiles( const EGArray<gcxCustomTile>& CustomTiles )
{
	auto IndexToCustomTile = [&CustomTiles]( eg_uint Index ) -> gcx_terrain_t
	{
		for( const gcxCustomTile& CustomTile : CustomTiles )
		{
			if( CustomTile.Index == Index )
			{
				return CustomTile.TerrainType;
			}
		}
		return gcx_terrain_t::DEFAULT;
	};

	for( gcxFloor& Floor : m_Floors )
	{
		if( Floor.HasData() )
		{
			for( eg_int i = 0; i < Floor.Width*Floor.Height; i++ )
			{
				gcxTile& Tile = Floor.GetTileByIndex( i );
				if( Tile.bHasCustomTile )
				{
					if( Tile.Terrain != gcx_terrain_t::DEFAULT )
					{
						// EGLogf( eg_log_t::General , __FUNCTION__ ": A non-custom terrain was replaced with a custom one %s (%u)." , *m_Name , i );
					}
					Tile.Terrain = IndexToCustomTile( Tile.CustomTileIndex );
				}
			}
		}
	}
}

eg_string_big EGGcxRegion::GetCleanName( eg_cpstr Data, eg_size_t Len )
{
	eg_string_big CleanName;

	for( eg_uint i = 0; i < Len; i++ )
	{
		eg_char c = Data[i];
		if( c != '\r' && c != '\n' && c != '\t' )
		{
			CleanName.Append( c );
		}
	}

	return CleanName;
}



void EGGcxRegion::GenerateWorldGeometry()
{
	m_EMapGeoBrushes.Clear();
	m_EMapVertexes.Clear();
	m_EMapVertexesHullIdxs.Clear();
	m_EMapSegments.Clear();
	m_EMapRegions.Clear();
	m_EMapLights.Clear();

	const eg_uint NumMapRegions = GetEMapRegionCount();
	const eg_size_t NumGeoms = m_WorldGeometry.Len();
	const eg_real WallLength = EGGcxBuildData::Get().GetWallLength();
	const eg_vec2 RegionHeightDims = EGGcxBuildData::Get().GetRegionHeightDims();

	for( eg_uint region = 0; region<NumMapRegions; region++ )
	{
		eg_ivec2 RegionPos = EMapRegionIndexToCoord( region );

		gcxEMapRegion NewRegion( CT_Default );
		NewRegion.Name = region;
		eg_size_t FirstRegionVertex = m_EMapVertexes.Len();
		NewRegion.FirstLight = static_cast<eg_uint>(m_EMapLights.Len());
		NewRegion.FirstSegment = static_cast<eg_uint>(m_EMapSegments.Len());
		NewRegion.FirstGeoBrush = static_cast<eg_uint>(m_EMapGeoBrushes.Len());
		NewRegion.FirstPortal = static_cast<eg_uint>(m_EMapPortals.Len());

		// Raster segments and physics geometry:
		{
			EGArray<gcxBuildMaterialData> AllMtrls;
			EGGcxBuildData::Get().GetAllMaterials( AllMtrls );

			for( eg_size_t i=0; i<AllMtrls.Len(); i++ )
			{
				const gcxBuildMaterialData& MtrlInfo = AllMtrls[i];

				eg_string_crc MtrlType = MtrlInfo.MaterialId;
				eg_uint MtrlIndex = static_cast<eg_uint>(i);

				EGArray<gcxWorldGeometry> GeomForThisRegionAndMaterial;
				for( const gcxWorldGeometry& Geom : m_WorldGeometry )
				{
					if( Geom.Region == region && Geom.Material == MtrlType )
					{
						GeomForThisRegionAndMaterial.Append( Geom );
					}
				}

				// We have all the geometry for this region and material, so let's write it.
				gcxEMapSegment NewSegment( CT_Default );
				NewSegment.FirstVertex = static_cast<eg_uint>(m_EMapVertexes.Len());
				NewSegment.MtrlIndex = MtrlIndex;
				eg_size_t StartVertex = m_EMapVertexes.Len();

				for( const gcxWorldGeometry& Geom : GeomForThisRegionAndMaterial )
				{
					WriteBox( Geom.Bounds , Geom.Material );
				}

				eg_size_t NumVertexes = m_EMapVertexes.Len() - StartVertex;
				assert( (NumVertexes%3) == 0 ); // Something other than triangles added?
				NewSegment.TriangleCount = static_cast<eg_uint>(NumVertexes/3);
				if( NewSegment.TriangleCount > 0 )
				{
					m_EMapSegments.Append( NewSegment );
				}
			}
		}

		// Lights:
		for( const gcxLight& Light : m_Lights )
		{
			if( Light.Region == region )
			{
				m_EMapLights.Append( Light );
			}
		}

		// Portals:
		{
			// There are 8 regions around this one:
			eg_int RegionsMinX = RegionPos.x-1;
			eg_int RegionsMinY = RegionPos.y-1;
			eg_int RegionsMaxX = RegionPos.x+1;
			eg_int RegionsMaxY = RegionPos.y+1;

			for( eg_int rx=RegionsMinX; rx<=RegionsMaxX; rx++ )
			{
				for( eg_int ry=RegionsMinY; ry<=RegionsMaxY; ry++ )
				{
					eg_bool bInXRange = EG_IsBetween<eg_int>( rx , 0 , m_RegionWidth-1);
					eg_bool bInYRange = EG_IsBetween<eg_int>( ry , 0 , m_RegionHeight-1);
					eg_bool bNotThis = !(RegionPos.x==rx && RegionPos.y==ry);
					if( bInXRange && bInYRange && bNotThis )
					{
						gcxEMapPortal NewPortal;
						NewPortal.ToRegion = EMapRegionCoordToIndex( rx , ry );
						m_EMapPortals.Append( NewPortal );
					}
				}
			}
		}

		NewRegion.NumLights = static_cast<eg_uint>(m_EMapLights.Len() - NewRegion.FirstLight);
		NewRegion.NumGeoBrushes = static_cast<eg_uint>(m_EMapGeoBrushes.Len() - NewRegion.FirstGeoBrush);
		NewRegion.NumSegments = static_cast<eg_uint>(m_EMapSegments.Len() - NewRegion.FirstSegment);
		NewRegion.NumPortals = static_cast<eg_uint>(m_EMapPortals.Len() - NewRegion.FirstPortal);

		eg_ivec2 MinTile(0,0);
		eg_ivec2 MaxTile(0,0);
		GetTileBoundsForEMapRegion( region , MinTile , MaxTile );

		NewRegion.Bounds.Min.x = MinTile.x * WallLength;
		NewRegion.Bounds.Max.x = (MaxTile.x + 1) * WallLength;
		NewRegion.Bounds.Min.y = RegionHeightDims.x;
		NewRegion.Bounds.Max.y = RegionHeightDims.y;
		NewRegion.Bounds.Min.z = MinTile.y * WallLength;
		NewRegion.Bounds.Max.z = (MaxTile.y + 1) * WallLength;
		NewRegion.Bounds.Min.w = 1.f;
		NewRegion.Bounds.Max.w = 1.f;

		m_EMapRegions.Append( NewRegion );
	}
}