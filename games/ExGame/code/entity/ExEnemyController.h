// (c) 2015 Beem Media

#pragma once

#include "ExMapNavBase.h"

class ExEnemyController : public ExMapNavBase
{
	EG_CLASS_BODY( ExEnemyController , ExMapNavBase )

private:

	struct exRepData
	{
		eg_string_crc EncounterId = CT_Preserve;
	};

	exRepData        m_RepData;
	EGNavGraphVertex m_NextNavToPlayer = CT_Preserve;
	EGNavGraphVertex m_BossNavVertex = CT_Preserve; // In case a boss moves we actually want to remember what vertex it spawned on.
	eg_transform     m_StartMovePose;
	eg_transform     m_EndMovePose;
	eg_string_crc    m_CachedEncounterId = CT_Clear;
	eg_bool          m_bHasSeenPlayer:1;
	eg_bool          m_bCanMove:1;
	eg_bool          m_bIsBoss:1;

public:

	// BEGIN ExMapNavBase
	virtual void OnCreate( const egEntCreateParms& Parms ) override final;
	virtual void OnDestroy() override final;
	virtual void OnUpdate( eg_real DeltaTime ) override final;
	virtual eg_bool SendMsg( ex_notify_t Msg , eg_ent_id SrcEntId ) override final;
	// END ExMapNavBase

	void HandleCombatStarted();
	void HandleCombatEnded( ex_combat_result CombatResult );
	void CalmHostility();
	void SetEncounterId( const eg_string_crc& NewEncounterId );
	eg_string_crc GetEncounterId() const;
	void SetIsBoss( eg_bool bNewValue ) { m_bIsBoss = bNewValue; if( m_bIsBoss ){ m_BossNavVertex = GetMapVertex(); } }
	eg_bool IsBoss() const { return m_bIsBoss; }
	const EGNavGraphVertex& GetBossVertex() const { return m_BossNavVertex; }

private:

	virtual void OnEnterWorld() override;
	virtual void OnDataReplicated( const void* Offset , eg_size_t Size ) override;

	void OnTurnStart( ExGame* pGameData );
};
