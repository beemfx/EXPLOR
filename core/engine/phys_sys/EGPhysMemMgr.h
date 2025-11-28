// (c) 2017 Beem Media

#pragma once

#include "EGEngineConfig.h"

class EGPhysMemMgr
{
public:

	struct egChunk
	{
		void*     Data = nullptr;
		eg_size_t DataSize = 0;
		eg_bool   bUsed = false;
	};

private:

	static EGPhysMemMgr s_Inst;
	EGFixedArray<egChunk,MAX_LOCAL_CLIENTS+1> m_Chunks;

public:

	static EGPhysMemMgr& Get() { return s_Inst; }

	void Init();
	void Deinit();

	egChunk GetChunk();
	void ReleaseChunk( void* Data );
};
