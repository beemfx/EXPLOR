// (c) 2018 Beem Media

#include "EGGcxFloor.h"
#include "EGGcxBuildTypes.h"
#include "EGGcxBuildData.h"

void gcxFloor::Clear()
{
	Tiles.Clear();
	Width = 0;
	Height = 0;
	bHasData = false;
	EmptyTile.Clear();
}

void gcxFloor::SetIndex( eg_int NewIndex )
{
	Index = NewIndex;
}

void gcxFloor::SetBounds( eg_int Newx0, eg_int Newy0, eg_int NewWidth, eg_int NewHeight )
{
	assert( NewWidth >= 0 && NewHeight >= 0 );
	Clear();

	x0 = Newx0;
	y0 = Newy0;
	Width = NewWidth;
	Height = NewHeight;

	if( Width > 0 && Height > 0 )
	{
		eg_size_t SizeNeeded = sizeof( gcxTile )*Width*Height;
		Tiles.Resize( SizeNeeded );
		EGMem_Set( Tiles.GetArray(), 0, Tiles.Len() );
	}

	for( eg_int i = 0; i < ( Width*Height ); i++ )
	{
		Tiles[i].Clear();
	}

	bHasData = true;
}

void gcxFloor::SetTile( eg_int x, eg_int y, const gcxTile& NewTile )
{
	GetTile( x, y ) = NewTile;
}

eg_bool gcxFloor::DoesTileHaveRealData( eg_int x, eg_int y ) const
{
	eg_bool bOut = false;

	eg_int ColOffset = x - x0;
	eg_int RowOffset = y - y0;
	if(
		( 0 <= ColOffset && ColOffset < Width )
		&&
		( 0 <= RowOffset && RowOffset < Height )
		)
	{
		bOut = true;
	}

	return bOut;
}

gcxTile& gcxFloor::GetTile( eg_int x, eg_int y, eg_bool AllowOutOfBounds /*= false */ )
{
	if( DoesTileHaveRealData( x, y ) )
	{
		return Tiles[( y - y0 )*Width + ( x - x0 )];
	}
	assert( AllowOutOfBounds );
	unused( AllowOutOfBounds );
	EmptyTile.Clear(); //Just in case it got changed.
	return EmptyTile;
}

const gcxTile& gcxFloor::GetTile( eg_int x, eg_int y, eg_bool AllowOutOfBounds /*= false */ ) const
{
	if( DoesTileHaveRealData( x, y ) )
	{
		return Tiles[( y - y0 )*Width + ( x - x0 )];
	}
	assert( AllowOutOfBounds );
	unused( AllowOutOfBounds );
	EmptyTile.Clear(); //Just in case it got changed.
	return EmptyTile;
}

gcx_edge_t gcxFloor::GetRight( eg_int x, eg_int y ) const
{
	if( DoesTileHaveRealData( x, y ) )
	{
		const gcxTile& Tile = GetTile( x, y, false );
		return Tile.Right;
	}
	return gcx_edge_t::NONE;
}

gcx_edge_t gcxFloor::GetBottom( eg_int x, eg_int y ) const
{
	if( DoesTileHaveRealData( x, y ) )
	{
		const gcxTile& Tile = GetTile( x, y, false );
		return Tile.Bottom;
	}
	return gcx_edge_t::NONE;
}

gcx_terrain_t gcxFloor::GetTerrainType( eg_int x, eg_int y ) const
{
	if( DoesTileHaveRealData( x, y ) )
	{
		const gcxTile& Tile = GetTile( x, y, false );
		return Tile.Terrain;
	}
	return gcx_terrain_t::DEFAULT;
}

eg_bool gcxFloor::TouchesWall( eg_int x, eg_int y ) const
{
	return HasBottom( x, y ) || HasBottom( x - 1, y ) || HasRight( x - 1, y ) || HasRight( x - 1, y - 1 );
}

eg_uint gcxFloor::GetPillarType( eg_int x, eg_int y ) const
{
	if( HasBottom( x, y ) )
	{
		return GetTile( x, y ).BottomColor;
	}
	else if( HasBottom( x - 1, y ) )
	{
		return GetTile( x - 1, y ).BottomColor;
	}
	else if( HasRight( x - 1, y ) )
	{
		return GetTile( x - 1, y ).RightColor;
	}
	else if( HasRight( x - 1, y - 1 ) )
	{
		return GetTile( x - 1, y - 1 ).RightColor;
	}

	assert( false );
	return 0xFFFFFFFF;
}

eg_real gcxFloor::GetPillarLowerOffset( eg_int x, eg_int y ) const
{
	// Get the lowest floor value that possibly touches the wall in question
	eg_real Out = 0.f;
	eg_bool bGot = false;

	const eg_ivec2 TILES_OF_INTEREST[] =
	{
		eg_ivec2( x,y ),
		eg_ivec2( x,y - 1 ),
		eg_ivec2( x - 1,y ),
		eg_ivec2( x - 1,y - 1 ),
	};

	for( const eg_ivec2& TilePos : TILES_OF_INTEREST )
	{
		if( DoesTileHaveRealData( x, y ) )
		{
			const gcxBuildTileData& TileData = EGGcxBuildData::Get().GetInfoForTerrain( GetTerrainType( TilePos.x, TilePos.y ) );
			Out = !bGot ? TileData.TopOffset : EG_Min<eg_real>( Out, TileData.TopOffset );
			bGot = true;
		}
	}

	return Out;
}

eg_real gcxFloor::GetBottomWallLowerOffset( eg_int x, eg_int y ) const
{
	// Get the lowest floor value that possibly touches the wall in question
	eg_real Out = 0.f;
	eg_bool bGot = false;

	const eg_ivec2 TILES_OF_INTEREST[] =
	{
		eg_ivec2( x,y ),
		eg_ivec2( x,y - 1 ),
		eg_ivec2( x + 1,y ),
		eg_ivec2( x + 1,y - 1 ),
		eg_ivec2( x - 1,y ),
		eg_ivec2( x - 1,y - 1 ),
	};

	for( const eg_ivec2& TilePos : TILES_OF_INTEREST )
	{
		if( DoesTileHaveRealData( x, y ) )
		{
			const gcxBuildTileData& TileData = EGGcxBuildData::Get().GetInfoForTerrain( GetTerrainType( TilePos.x, TilePos.y ) );
			Out = !bGot ? TileData.TopOffset : EG_Min<eg_real>( Out, TileData.TopOffset );
			bGot = true;
		}
	}

	return Out;
}

eg_real gcxFloor::GetRightWallLowerOffset( eg_int x, eg_int y ) const
{
	// Get the lowest floor value that possibly touches the wall in question
	eg_real Out = 0.f;
	eg_bool bGot = false;

	const eg_ivec2 TILES_OF_INTEREST[] =
	{
		eg_ivec2( x,y ),
		eg_ivec2( x,y - 1 ),
		eg_ivec2( x,y + 1 ),
		eg_ivec2( x + 1,y ),
		eg_ivec2( x + 1,y - 1 ),
		eg_ivec2( x + 1,y + 1 ),
	};

	for( const eg_ivec2& TilePos : TILES_OF_INTEREST )
	{
		if( DoesTileHaveRealData( x, y ) )
		{
			const gcxBuildTileData& TileData = EGGcxBuildData::Get().GetInfoForTerrain( GetTerrainType( TilePos.x, TilePos.y ) );
			Out = !bGot ? TileData.TopOffset : EG_Min<eg_real>( Out, TileData.TopOffset );
			bGot = true;
		}
	}

	return Out;
}

eg_aabb gcxFloor::GetTileBounds( eg_int x, eg_int y ) const
{
	eg_aabb BoundsTile;
	const eg_real WALL_LENGTH = EGGcxBuildData::Get().GetWallLength();
	const eg_real WALL_HEIGHT = EGGcxBuildData::Get().GetInfoForWall(0).Height;
	BoundsTile.Min = eg_vec4( x*WALL_LENGTH, Index*WALL_HEIGHT, y*WALL_LENGTH, 1.f );
	BoundsTile.Max = BoundsTile.Min + eg_vec4( WALL_LENGTH, WALL_HEIGHT, WALL_LENGTH, 0.f );
	return BoundsTile;
}

void gcxFloor::GetEffectiveBounds( eg_int* xMinOut, eg_int* xMaxOut, eg_int* yMinOut, eg_int* yMaxOut ) const
{
	if( !HasData() )
	{
		*xMinOut = 0;
		*xMaxOut = 0;
		*yMinOut = 0;
		*yMaxOut = 0;
		return;
	}

	// xMin: (Is an actual bound only if there is a bottom somewhere along the way.
	eg_bool IsFullWidth = false;
	for( eg_int y = y0; !IsFullWidth && y < ( y0 + Height ); y++ )
	{
		eg_int LeftColumn = x0;
		if
			(
				HasN( LeftColumn, y )
				|| HasS( LeftColumn, y )
				|| GetTerrainType( LeftColumn, y ) != gcx_terrain_t::DEFAULT
				)
		{
			IsFullWidth = true;
		}
	}

	//yMax: (Is an actual bound only if there is a right somewhere along the way.
	eg_bool IsFullHeight = false;
	for( eg_int x = x0; !IsFullHeight && x < ( x0 + Width ); x++ )
	{
		eg_int TopRow = y0 + Height - 1;
		if
			(
				HasE( x, TopRow )
				|| HasW( x, TopRow )
				|| GetTerrainType( x, TopRow ) != gcx_terrain_t::DEFAULT
				)
		{
			IsFullHeight = true;
		}
	}

	*xMinOut = IsFullWidth ? x0 : x0 + 1;
	*xMaxOut = x0 + Width - 1;
	*yMinOut = y0;
	*yMaxOut = IsFullHeight ? y0 + Height - 1 : y0 + Height - 2;
}
