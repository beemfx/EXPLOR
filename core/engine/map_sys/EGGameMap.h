// (c) 2011 Beem Software

#pragma once

#include "EGWorldMapBase.h"
#include "EGMap_PhysLoader.h"
#include "EGDelegate.h"

class EGGameMap : public EGWorldMapBase , private ILoadable
{
public:

	EG_DECLARE_FLAG( MAPDEBUG_HULLAABB  , 1 );
	EG_DECLARE_FLAG( MAPDEBUG_HULLTRI   , 2 );
	EG_DECLARE_FLAG( MAPDEBUG_WORLDAABB , 3 );

public:

	EGGameMap( eg_bool Create3DAssets );
	~EGGameMap();
	
	//Replace the load function (still call base
	//function, but the new one loads the 3D info).
	void LoadOnLoadingThread( eg_cpstr Filename , eg_bool bServer );
	void LoadOnThisThread( eg_cpstr Filename );
	//Same for Unload();
	void Unload();
	LOAD_S GetLoadState() const { return m_LoadState; }

	void Draw()const;
	void DrawRegion(eg_uint nRegion)const;
	void DrawAABBs(eg_flags flags)const;
	void DrawGraphs( class EGDebugSphere* VertexShape )const;

	eg_bool HasPhysXChunk()const { return m_PhysData.IsLoaded(); }
	eg_size_t GetPhysXChunkSize() const { return m_PhysData.GetSize(); }
	const void* GetPhysXChunk() const { return m_PhysData.GetChunk(); }

private:

	virtual void DoLoad(eg_cpstr strFile, const eg_byte*const  pMem, const eg_size_t Size) override;
	virtual void OnLoadComplete(eg_cpstr strFile) override;

	void DeallocateMemory();

	static eg_string BuildFinalFilename( eg_cpstr Filename );

public:

	EGMCDelegate<EGGameMap*> MapLoadedDelegate;

private:

	EGMap_PhysLoader m_PhysData;
	egv_vbuffer      m_VB;
	LOAD_S           m_LoadState:4;
	const eg_bool    m_Create3DAssets:1;
};



