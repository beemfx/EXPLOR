// (c) 2018 Beem Media

#pragma once

#include "EGDataAsset.h"
#include "EGAssetPath.h"
#include "ExCreditsData.reflection.h"

egreflect enum class ex_credits_line_t
{
	Normal,
	FuturePop,
	HeaderPop,
	FutureHeaderPop,
	Header,
	Center,
	Blob,
	Delay,
};

egreflect struct exCreditsName
{
	egprop eg_string_crc Name;
	egprop eg_d_string   Name_enus;
};

egreflect struct exCreditsLine
{
	egprop eg_string_crc     Comment; // This is just so when collapsed you can see what it is in the editor.
	egprop ex_credits_line_t Type;
	egprop exCreditsName     Title;
	egprop exCreditsName     Name;
	egprop eg_real           DelayTime;
};

egreflect class ExCreditsData : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExCreditsData , EGDataAsset )

public:

	egprop EGArray<exCreditsLine> m_Lines = eg_mem_pool::DefaultHi;

public:

	virtual void OnPropChanged( const egRflEditor& ChangedProperty , egRflEditor& RootEditor , eg_bool& bNeedsRebuildOut ) override;
	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;

private:

	void RefreshVisibleProperties( egRflEditor& RootEditor );
};

egreflect struct exCreditsDataInfo
{
	egprop eg_asset_path CreditsDataPath = "egasset";
	ExCreditsData* LoadedCreditsData = nullptr;

	~exCreditsDataInfo()
	{
		if( LoadedCreditsData )
		{
			EGDeleteObject( LoadedCreditsData );
			LoadedCreditsData = nullptr;
		}
	}
};

egreflect class ExCreditsCollection : public egprop EGDataAsset
{
	EG_CLASS_BODY( ExCreditsCollection , EGDataAsset )

public:
	
	egprop eg_real m_DesiredCreditsDurationSeconds = 30.f;
	egprop EGArray<exCreditsDataInfo> m_CreditsDataInfo;

public:

	virtual void PostLoad( eg_cpstr16 Filename , eg_bool bForEditor , egRflEditor& RflEditor ) override;
};