// (C) 2014 Beem Media

#pragma once

#include "EGD11R_Mtrl.h"
#include "EGEngineConfig.h"
#include "EGList.h"
#include "EGMutex.h"

class EGD11R_MtrlMgr2
{
private:

	enum class eg_item_s
	{
		Unused,
		Used,
		BeingDestroyed,
		NullItem,
	};

	struct egItem
	{
		eg_uint      Index = 0;
		EGD11R_Mtrl* Material = nullptr;
		eg_item_s    State = eg_item_s::Unused;
		eg_size_t    RefCount = 0;
		eg_d_string8 UniqueId = "";
		// eg_d_string8 DebugRef;
	};

	EGMutex                 m_Lock;
	EGArray<egItem>         m_MasterList;
	EGArray<eg_size_t>      m_DestroyQue;
	EGD11R_Texture          m_TxDef;
	class EGD11R_ShaderMgr* m_ShaderMgr;

public:

	EGD11R_MtrlMgr2( class EGD11R_ShaderMgr* ShaderMgr );
	~EGD11R_MtrlMgr2();

	void Update_RenderThread( void );
	void PurgeDestroyQueue( void );

	egv_material       CreateMaterial( const EGMaterialDef* Def , eg_cpstr SharedId );
	void               DestroyMaterial( egv_material Material );
	const EGD11R_Mtrl* GetMaterial( egv_material Material );	
};