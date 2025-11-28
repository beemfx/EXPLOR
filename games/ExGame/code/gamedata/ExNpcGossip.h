// (c) 2021 Beem Media. All Rights Reserved.

#pragma once

#include "ExGameTypes.h"
#include "EGDataAsset.h"
#include "ExNpcGossip.reflection.h"

class EGRandom;

egreflect struct exNpcGossipDialog
{
	egprop exLocTextMultiline CharacterDialog;
	egprop exLocText PlayerResponse;
};

egreflect struct exNpcGossipForNpc
{
	egprop eg_string_crc Id = CT_Clear;
	egprop EGArray<exNpcGossipDialog> Dialogs;

	exNpcGossipDialog GetRandomDialog( EGRandom& Rng ) const;
};

egreflect class ExNpcGossip : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExNpcGossip , EGDataAsset )
	EG_FRIEND_RFL( ExNpcGossip )

protected:

	static ExNpcGossip* s_Inst;

	egprop EGArray<exNpcGossipForNpc> m_NpcGossips;

public:

	static ExNpcGossip& Get(){ return *s_Inst; }

	static void Init( eg_cpstr Filename );
	static void Deinit();

	exNpcGossipDialog GetRandomDialogForNpc( eg_string_crc NpcId , EGRandom& Rng ) const;

protected:

	void TestUniqueness();
};
