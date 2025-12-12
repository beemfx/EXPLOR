// (c) 2017 Beem Media

#pragma once

#include "ExGameTypes.h"
#include "ExMapInfos.h"
#include "EGRendererTypes.h"

class ExGame;
class EGFileData;

struct exGridPose
{
	eg_ivec2 GridPos;
	ex_face  GridFace;

	exGridPose() = default;

	exGridPose( eg_ctor_t Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			GridPos = eg_ivec2(0,0);
			GridFace = ex_face::NORTH;
		}
	}
};

struct exMappingRevealed
{
private:

	eg_string_crc    MapId = CT_Clear;
	eg_ivec2         MapSize = eg_ivec2(0,0);
	EGArray<eg_bool> RevealedArray;

public:

	void InitForMapping( const eg_string_crc& InMapId , const eg_ivec2& InMapSize );
	const eg_string_crc& GetMapId() const { return MapId; }
	eg_bool IsRevealed( const eg_ivec2& Pos ) const;
	void SetRevealed( const eg_ivec2& Pos , eg_bool bRevealed );
	eg_int GetNumSquaresRevealed() const;

	void SaveTo( EGFileData& FileOut );
	void LoadFrom( const EGFileData& FileIn );

	eg_bool operator == ( const exMappingRevealed& rhs ) const 
	{
		return MapId == rhs.MapId;
	}

protected:

	eg_int PosToIndex( const eg_ivec2& Pos ) const;
};

class ExMapping : public EGObject
{
	EG_CLASS_BODY( ExMapping , EGObject )

public:

	EGMCDelegate<egv_material /*NewMaterial*/> MapMaterialChangedDelegate;

private:

	static const eg_int AUTOMAP_TILE_WIDTH = 16; // Should match the gcx tool

private:

	ExGame*                    m_OwningGame = nullptr;
	eg_string_crc              m_CurrentMapId = CT_Clear;
	exMapInfoData              m_MapInfo;
	egv_material               m_MapImage = EGV_MATERIAL_NULL;
	egv_material               m_BlockSquare = EGV_MATERIAL_NULL;
	egv_material               m_MapRenderMaterial = EGV_MATERIAL_NULL;
	egv_rtarget                m_MapRender = egv_rtarget::Null;
	eg_bool                    m_bNeedsDraw = false;
	exMappingRevealed          m_ClientMapReveal;
	exMappingRevealed*         m_CurrentMapReveal = nullptr;
	mutable eg_int             m_CachedSquaresRevealed = -1;
	EGArray<exMappingRevealed> m_MapRevealDatas;

public:

	virtual void OnDestruct() override;

	void SetOwner( ExGame* OwningGame );
	void SetMap( const eg_string_crc& MapId );
	egv_material GetMapTexture() const { return m_MapImage; }
	egv_material GetMapRenderMaterial() const { return  m_MapRenderMaterial; }
	void SetGlobalSamplerToMap();
	eg_string_crc GetMapId() const { return m_CurrentMapId; }
	eg_ivec2 GetMapSize() const;
	eg_ivec2 GetMapLowerLeftOrigin() const;
	ex_map_reveal_result RevealPose( const eg_transform& WorldPose );
	void RevealReplicatedSquare( const eg_event_parms& Parms );
	exGridPose WorldPoseToGridPos( const eg_transform& WorldPose ) const;
	eg_bool IsPoseRevealed( const exGridPose& GridPose ) const;
	eg_int GetNumSquaresRevealed() const;
	void ReplicateCurrentMap();
	void ClientBeginFullReplication( eg_string_crc m_CurrentMapId );
	void RefreshMap() { m_bNeedsDraw = true; }

	void Draw();

	void SaveTo( EGFileData& FileOut );
	void LoadFrom( const EGFileData& FileIn );

private:

	void SetupMapGraphics();
	void SetupCurrentRevealData();
};