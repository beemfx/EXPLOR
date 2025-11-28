// (c) 2018 Beem Media

#pragma once

#include "EGGcxTypes.h"

struct gcxFloor
{
public:

	eg_int   Index = 0;
	eg_int   x0 = 0;
	eg_int   y0 = 0;
	eg_int   Width = 0;
	eg_int   Height = 0;
	EGArray<gcxNote> Notes;

private:

	EGArray<gcxTile> Tiles;
	eg_bool  bHasData = false;

	mutable gcxTile EmptyTile;

public:

	gcxFloor() = default;

	eg_bool HasData() const { return bHasData; }
	void Clear();
	void SetIndex( eg_int NewIndex );
	void SetBounds( eg_int Newx0 , eg_int Newy0 , eg_int NewWidth , eg_int NewHeight );
	void SetTile( eg_int x , eg_int y , const gcxTile& NewTile );
	eg_bool DoesTileHaveRealData( eg_int x , eg_int y ) const;
	gcxTile& GetTile( eg_int x , eg_int y , eg_bool AllowOutOfBounds = false );
	const gcxTile& GetTile( eg_int x , eg_int y , eg_bool AllowOutOfBounds = false )const;
	gcxTile& GetTileByIndex( eg_size_t TileIndex ) { return Tiles[TileIndex]; }
	const gcxTile& GetTileByIndex( eg_size_t TileIndex ) const { return Tiles[TileIndex]; }
	gcx_edge_t GetRight( eg_int x , eg_int y )const;
	gcx_edge_t GetBottom( eg_int x , eg_int y )const;
	gcx_terrain_t GetTerrainType( eg_int x , eg_int y )const;
	eg_bool HasRight( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetRight( x , y ); }
	eg_bool HasBottom( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetBottom( x , y ); }
	eg_bool TouchesWall( eg_int x , eg_int y )const;
	eg_uint GetPillarType( eg_int x , eg_int y )const;
	eg_real GetPillarLowerOffset( eg_int x , eg_int y ) const;
	eg_real GetBottomWallLowerOffset( eg_int x , eg_int y )const;
	eg_real GetRightWallLowerOffset( eg_int x , eg_int y ) const;
	eg_aabb GetTileBounds( eg_int x , eg_int y ) const;
	gcx_edge_t GetN( eg_int x , eg_int y )const { return GetBottom( x , y+1 ); }
	gcx_edge_t GetE( eg_int x , eg_int y )const { return GetRight( x , y ); }
	gcx_edge_t GetS( eg_int x , eg_int y )const { return GetBottom( x , y ); }
	gcx_edge_t GetW( eg_int x , eg_int y )const { return GetRight( x-1 , y ); }
	eg_bool HasN( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetN( x , y ); }
	eg_bool HasE( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetE( x , y ); }
	eg_bool HasS( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetS( x , y ); }
	eg_bool HasW( eg_int x , eg_int y )const{ return gcx_edge_t::NONE != GetW( x , y ); }
	void GetEffectiveBounds( eg_int* xMinOut , eg_int* xMaxOut , eg_int* yMinOut , eg_int* yMaxOut )const;
};
