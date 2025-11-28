// (c) 2018 Beem Media

#pragma once

#include "EGThreadProc.h"

enum class eg_edapproc_reimport_t
{
	None,
	Standard,
	SingleFile,
};

struct egEdAppProcReimportInfo
{
	eg_edapproc_reimport_t Type = eg_edapproc_reimport_t::None;
	eg_d_string8 Parm = CT_Clear;
};

class EGEdAppProc : public IThreadProc
{
private:

public:

	mutable EGMutex      m_Mutex;
	EGArray<eg_d_string> m_MessageQueue;
	eg_bool              m_bIsExecuting = false;
	eg_bool              m_bWantsCancel = false;

private:

	egEdAppProcReimportInfo m_LastReimportInfo;

public:

	virtual void Update( eg_real DeltaTime ) override;
	virtual void OnThreadMsg(eg_cpstr strParm) override;
	eg_bool HasCommands() const;
	eg_bool IsExecuting() const;
	void CancelCommands();

	const egEdAppProcReimportInfo& GetLastReimportInfo() const { return m_LastReimportInfo; }
	void SetLastReimportInfo( eg_edapproc_reimport_t NewType , eg_cpstr8 NewParm ) { m_LastReimportInfo.Type = NewType; m_LastReimportInfo.Parm = NewParm; }

private:

	void SetIsExecuting( eg_bool bNewValue );
};
