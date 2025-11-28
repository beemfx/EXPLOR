// (c) 2017 Beem Media

#pragma once

#include "EGWeakPtr.h"

class EGFileData;
class EGEngineInst;

enum class eg_config_file_t
{
	UserSettings,
	SystemSettings,
};

class EGEngineCore
{
private:

	EGWeakPtr<EGEngineInst> m_EngineInst;

public:

	void Init( EGEngineInst* InEngineInstance );
	void Deinit();
	void Update( eg_real DeltaTime );

	void PrintDir();
	void ProcessSettings( eg_bool bLoad );
	void LoadKeyBindings();

	static eg_bool DoesFileExist( eg_cpstr Filename );

private:

	void ProcessSettings_LoadFromFileData( const EGFileData& InFileData );
};