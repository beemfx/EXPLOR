// (c) 2017 Beem Media

#pragma once

#include "EGSmTypes.h"

static const eg_size_t EGSM_CALLSTACK_SIZE = 10;
static const eg_size_t EGSM_INPUT_LIMIT = 10;

struct egsmStackInfo
{
	eg_string_crc ScriptProc;
	egsm_node_id CurrentNode;

	egsmStackInfo() = default;
	egsmStackInfo( eg_string_crc InScriptProc , egsm_node_id InCurrentNode )
	: ScriptProc(InScriptProc) 
	, CurrentNode(InCurrentNode)
	{ 
	}
};

typedef EGFixedArray<egsmStackInfo,EGSM_CALLSTACK_SIZE> EGSmCallstack;
typedef EGFixedArray<eg_string_crc,EGSM_INPUT_LIMIT> EGSmBranchOptions;

enum class egsm_proc_state
{
	READY,          // Initial state.
	RUNNING,        // Used internally when the machine state is executing.
	TERMINAL,       // Calling has ended.
	YIELD,          // Calling wants input from one of the InputOptions
	TERMINAL_YIELD, // Calling was suspended and should be resumed (next encounter with thing that's running the script, frame tick, etc, context dependent).
	TERMINAL_ERROR, // Calling was ended due to a code error.
};

struct egsmState
{
	egsm_proc_state   ProcState;
	eg_string_crc     EntryProc;
	eg_size_t         ExecCount;
	EGSmCallstack     Callstack;

	egsmState(): egsmState( CT_Preserve ){ }

	egsmState( eg_ctor_t Ct )
	: Callstack( Ct )
	{
		if( Ct == CT_Clear || Ct == CT_Default )
		{
			ProcState = egsm_proc_state::READY;
			ExecCount = 0;
		}
	}

	void ClearToTerminalError()
	{
		ProcState = egsm_proc_state::TERMINAL_ERROR;
		Callstack.Clear();
	}
};

class ISmRuntimeHandler
{
public:

	struct egsmYield
	{
		egsmYield(){ }
	};

	struct egsmTerminal
	{
		egsmTerminal(){ }
	};

	struct egsmBranch
	{
		egsmBranch( eg_string_crc InBranch ): Branch(InBranch){ }
		eg_string_crc Branch;
	};

	struct egsmNotHandled
	{
		egsmNotHandled(){ }
	};

	struct egsmDefault
	{
		egsmDefault(){ }
	};

	struct egsmNativeRes
	{
		
		egsmNativeRes( const egsmYield& rhs ): bNotHandled(false),bYield(true),bTerminal(false),Branch(CT_Clear){ unused(rhs); }
		egsmNativeRes( const egsmBranch& rhs ): bNotHandled(false),bYield(false),bTerminal(false),Branch(rhs.Branch){ }
		egsmNativeRes( const egsmTerminal& rhs): bNotHandled(false),bYield(false),bTerminal(true),Branch(CT_Clear){ unused(rhs); }
		egsmNativeRes( const egsmNotHandled& rhs): bNotHandled(true),bYield(false),bTerminal(false),Branch(CT_Clear){ unused(rhs); }
		egsmNativeRes( const egsmDefault& rhs ): bNotHandled(false),bYield(false),bTerminal(false),Branch(eg_crc("NEXT")){ unused(rhs); }

		eg_bool IsYield() const { return bYield; }
		eg_bool IsTerminal() const { return bTerminal; }
		eg_bool IsNotHandled() const { return bNotHandled; }
		eg_string_crc GetBranch() const { return Branch; }

	private:
		eg_bool       bYield;
		eg_bool       bTerminal;
		eg_bool       bNotHandled;
		eg_string_crc Branch;

	};

	
	virtual egsm_var SmGetVar( eg_string_crc VarName ) const { unused( VarName ); return egsm_var( CT_Clear ); }
	virtual eg_bool SmDoesVarExist( eg_string_crc VarName ) const { unused( VarName ); return true; }
	virtual void SmSetVar( eg_string_crc VarName , const egsm_var& Value ){ unused( VarName , Value ); }
	virtual void SmOnError( eg_cpstr ErrorString ){ unused( ErrorString ); }
	virtual egsmNativeRes SmOnNativeEvent( eg_string_crc EventName , const EGArray<egsm_var>& Parms , const EGArray<eg_string_crc>& Branches ){ unused( EventName , Parms ); return egsmNativeRes( Branches.IsValidIndex(0) ? Branches[0] : eg_crc("DEFAULT") ); }
	
	egsm_var SmResolveParm( const egsm_var& Parm );
};

void EGSmRuntime_Run( egsmState& State , eg_string_crc ProcId , eg_string_crc EntryPoint , ISmRuntimeHandler* Handler );
void EGSmRuntime_Resume( egsmState& State , eg_string_crc BranchChoiceMade , ISmRuntimeHandler* Handler );
