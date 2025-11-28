// (c) 2015 Beem Media

#pragma once

#include "EGConsole.h"
#include "EGTextNode.h"

extern EGConsole MainConsole;

eg_color32 DebugText_GetStringColorAndRemoveColorTag( eg_string* String );

class EGLogToScreen
{
private:

	struct egLogInfo
	{
		eg_uintptr_t  UniqueId = 0;
		eg_real       TimeLeft = 0.f;
		eg_string_big Message = CT_Clear;
	};

private:

	static EGLogToScreen s_Inst;

	EGArray<egLogInfo> m_Logs;
	EGMutex            m_LogLock;
	EGTextNode         m_TextNode = CT_Default;

public:

	static EGLogToScreen& Get() { return s_Inst; }

	void Init();
	void Update( eg_real DeltaTime );
	void Draw( eg_real AspectRatio );
	void Deinit();

	void Log( eg_uintptr_t UniqueId , eg_real Duration , eg_cpstr Message );
	void Log( const void* Owner , eg_uint Index , eg_real Duration , eg_cpstr Message );
};
