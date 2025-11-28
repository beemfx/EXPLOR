// (c) 2020 Beem Media. All Rights Reserved.

#pragma once

#include "EGDataAsset.h"

egreflect struct exAchievementInfo
{
	egprop eg_d_string Id;
};

egreflect class ExAchievements : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExAchievements , EGDataAsset )
	EG_FRIEND_RFL( ExAchievements )

private:

	egprop EGArray<exAchievementInfo> m_Data;

	EGArray<eg_string_crc> m_UnlockedAchievments;
	EGArray<eg_string_crc> m_UnlockQueue;
	mutable EGMutex m_DataLock;

	static ExAchievements* s_Inst;

public:

	static ExAchievements& Get(){ return *s_Inst; }

	static void Init( eg_cpstr Filename );
	static void Deinit();

	void Update( eg_real DeltaTime );
	void UnlockAchievement( eg_string_crc Id );

	void ResetAchievements();

	exAchievementInfo GetAchievmentFromCrc( eg_string_crc Id ) const;
};
