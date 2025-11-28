// (c) 2017 Beem Media

#include "EGSmRuntime.h"
#include "EGSmMgr.h"
#include "EGSmProc.h"

static ISmRuntimeHandler::egsmNativeRes EGSmRuntime_HandleIntrinsicNativeEvent( ISmRuntimeHandler* Handler , eg_string_crc EventName , EGArray<egsm_var> Parms )
{
	auto GetParmValue = [&Handler,&Parms]( eg_size_t ParmIndex ) -> egsm_var
	{
		return Parms.IsValidIndex( ParmIndex ) ? Handler->SmResolveParm( Parms[ParmIndex] ) : egsm_var( static_cast<eg_int>(0) );
	};
	
	switch_crc( EventName )
	{
		case_crc("If"):
		{
			egsm_var Value = GetParmValue( 0 );
			return ISmRuntimeHandler::egsmBranch( Value.as_bool() ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc("Exit"):
		{
			return ISmRuntimeHandler::egsmTerminal();
		}
		case_crc("Add"):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A + B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Subtract" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A - B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Multiply" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A * B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Divide" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A / B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Max" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A > B ? A : B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Min" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			egsm_var Out = A < B ? A : B;

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "Clamp" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			egsm_var B = GetParmValue( 2 );

			if( A > B )
			{
				egsm_var T = A;
				A = B;
				B = T;
			}

			egsm_var Out = GetParmValue( 0 );
			if( Out < A )
			{
				Out = A;
			}
			if( Out > B )
			{
				Out = B;
			}

			Handler->SmSetVar( Parms[0].as_crc() , Out );

		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "SetNumber" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			Handler->SmSetVar( Parms[0].as_crc() , A );
		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "SetBool" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			Handler->SmSetVar( Parms[0].as_crc() , A.as_bool() );
		} return ISmRuntimeHandler::egsmDefault();
		case_crc( "IsGreaterEqualThan" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A >= B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "IsGreaterThan" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A > B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "IsEqual" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A == B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "IsNotEqual" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A != B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "IsLessEqualThan" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A <= B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "IsLessThan" ):
		{
			egsm_var A = GetParmValue( 0 );
			egsm_var B = GetParmValue( 1 );

			return ISmRuntimeHandler::egsmBranch( A < B ? eg_crc("TRUE") : eg_crc("FALSE") );
		}
		case_crc( "Not" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			Handler->SmSetVar( Parms[0].as_crc() , !A.as_bool() );
			return ISmRuntimeHandler::egsmDefault();
		}
		case_crc( "Negate" ):
		{
			assert( Parms[0].GetType() == egsm_var_t::CRC );

			egsm_var A = GetParmValue( 1 );
			Handler->SmSetVar( Parms[0].as_crc() , -A );
			return ISmRuntimeHandler::egsmDefault();
		}

		case_crc("SetCrc"):
		{
			egsm_var A = GetParmValue( 1 );
			Handler->SmSetVar( Parms[0].as_crc() , A.as_crc() );
			return ISmRuntimeHandler::egsmDefault();
		}

		case_crc("SwitchOnCrc"):
		{
			egsm_var Var = GetParmValue( 0 );
			return ISmRuntimeHandler::egsmBranch( Var.as_crc() );
		}
	}

	return ISmRuntimeHandler::egsmNotHandled();
}

static void EGSmRuntime_HandleNode( egsmState& State , ISmRuntimeHandler* Handler )
{
	if( State.ProcState != egsm_proc_state::RUNNING || State.Callstack.Len() == 0 )
	{
		Handler->SmOnError( "Handle Node Error: State was not running." );
		State.ClearToTerminalError();
		return;
	}

	const EGSmProc* CurProc = EGSmMgr_GetProc( State.Callstack.Top().ScriptProc );
	if( nullptr == CurProc )
	{
		Handler->SmOnError( "Handle Node Error: No proc." );
		State.ClearToTerminalError();
		return;
	}

	// Handle popping the callstack.

	if( State.Callstack.Top().CurrentNode.IsNull() )
	{
		State.Callstack.Pop();
		CurProc = nullptr;

		if( State.Callstack.IsEmpty() )
		{
			State.ProcState = egsm_proc_state::TERMINAL;
		}
		else
		{
			const EGSmProc* CallingProc = EGSmMgr_GetProc( State.Callstack.Top().ScriptProc );
			if( nullptr == CallingProc )
			{
				Handler->SmOnError( "Handle Node Error: Callstack popped and the calling proc was missing." );
				State.ClearToTerminalError();
				return;
			}

			const egsmNodeBc* CallingNode = CallingProc->GetNode( State.Callstack.Top().CurrentNode );
			if( nullptr == CallingNode )
			{
				Handler->SmOnError( "Handle Node Error: A call came from an unknown source." );
				State.ClearToTerminalError();
				return;
			}
			if( CallingNode->Type != egsm_node_t::CALL )
			{
				Handler->SmOnError( "Handle Node Error: A call came from an a source that was not a CALL." );
				State.ClearToTerminalError();
				return;
			}
			if( CallingNode->NumBranches != 1 )
			{
				Handler->SmOnError( "Handle Node Error: A call came from a source that had more than one branch. Possible compilation error." );
				State.ClearToTerminalError();
				return;
			}
			const egsmBranchBc* Branch = CallingProc->GetBranch( CallingNode , 0 );
			if( nullptr == Branch )
			{	
				Handler->SmOnError( "Handle Node Error: No branch existed for CALL." );
				State.ClearToTerminalError();
				return;
			}
			State.Callstack.Top().CurrentNode = egsm_node_id(Branch->ToNodeAsIndexPlusOne);
			if( Branch->bTerminal )
			{
				State.ProcState = egsm_proc_state::TERMINAL_YIELD;
				// We'll also check to see if we are popping to a terminal
				// node, because if we are we'll set the state to TERMINAL instead.
				if( State.Callstack.Top().CurrentNode.IsNull() )
				{
					State.ProcState = egsm_proc_state::TERMINAL;
				}
			}
		}

		return;
	}

	const egsmNodeBc* CurNode = CurProc->GetNode( State.Callstack.Top().CurrentNode );
	if( nullptr == CurNode )
	{
		Handler->SmOnError( "Resume Error: No current node." );
		State.ClearToTerminalError();
		return;
	}

	auto GotoBranch = [&State,&Handler,&CurProc,&CurNode]( eg_size_t BranchIndex ) -> void
	{
		const egsmBranchBc* Branch = CurProc->GetBranch( CurNode , BranchIndex );
		if( Branch )
		{
			State.Callstack.Top().CurrentNode = egsm_node_id(Branch->ToNodeAsIndexPlusOne);
			if( Branch->bTerminal )
			{
				State.ProcState = egsm_proc_state::TERMINAL_YIELD;
			}
		}
		else
		{
			Handler->SmOnError( "Handle Node Error: The proc tried to go to a branch that didn't exist." );
			State.ClearToTerminalError();
		}
	};

	switch( CurNode->Type )
	{
		case egsm_node_t::UNKNOWN:
		{
			Handler->SmOnError( "Handle Node Error: Invalid node, possibly and old script." );
			State.ClearToTerminalError();
		} break;
		case egsm_node_t::ENTRY_POINT:
		{
			GotoBranch( 0 );
		} break;
		case egsm_node_t::CALL:
		{
			eg_string_crc CallProcId = CurNode->Parms[0].as_crc();
			eg_string_crc CallLabel = CurNode->Parms[1].as_crc();
			const EGSmProc* CallProc = CallProcId.IsNull() ? CurProc : EGSmMgr_GetProc( CallProcId );
			egsm_node_id EntryNode = CallProc ? CallProc->FindEntryNode( CallLabel , CallLabel.IsNull() ) : CT_Clear;
			if( EntryNode.IsNull() )
			{
				Handler->SmOnError( "Handle Node Error: Called a bad function." );
				GotoBranch( 0 );
			}
			else if( State.Callstack.IsFull() )
			{
				Handler->SmOnError( "Handle Node Error: Stack overflow." );
				GotoBranch( 0 );
			}
			else
			{
				State.Callstack.Push( egsmStackInfo( CallProc->GetId() , EntryNode ) );
			}
		} break;
		case egsm_node_t::NATIVE_EVENT:
		{
			EGArray<eg_string_crc> BranchChoices;
			for( eg_size_t i = 0; i < CurNode->NumBranches; i++ )
			{
				const egsmBranchBc* Branch = CurProc->GetBranch( CurNode, i );
				BranchChoices.Append( Branch ? Branch->Id : eg_string_crc( CT_Clear ) );
			}
			eg_string_crc EventName( CT_Clear );
			EGArray<egsm_var> Parms;
			Parms.Resize( 3 );
			EventName = CurNode->Parms[0].as_crc();
			Parms[0] = CurNode->Parms[1];
			Parms[1] = CurNode->Parms[2];
			Parms[2] = CurNode->Parms[3];

			ISmRuntimeHandler::egsmNativeRes Res = EGSmRuntime_HandleIntrinsicNativeEvent( Handler , EventName , Parms );
			if( Res.IsNotHandled() )
			{
				Res = Handler->SmOnNativeEvent( EventName , Parms , BranchChoices );
			}

			if( Res.IsTerminal() )
			{
				State.ProcState = egsm_proc_state::TERMINAL;
				State.Callstack.Clear();
			}
			else if( Res.IsYield() )
			{
				State.ProcState = egsm_proc_state::YIELD;
			}
			else if( Res.IsNotHandled() )
			{
				GotoBranch( 0 );
			}
			else
			{
				eg_size_t ChoiceMade = EGSM_INVALID_INDEX;
				for( eg_size_t i=0; i<CurNode->NumBranches; i++ )
				{
					const egsmBranchBc* Branch = CurProc->GetBranch( CurNode , i );
					if( Branch->Id == Res.GetBranch() )
					{
						ChoiceMade = i;
						break;
					}
				}

				if( ChoiceMade == EGSM_INVALID_INDEX )
				{
					Handler->SmOnError( "Handle Node Error: NATIVE_EVENT wanted to go to an invalid branch." );
					ChoiceMade = 0;
				}

				GotoBranch( ChoiceMade );
			}
		} break;
	}
}

static void EGSmRuntime_Exec( egsmState& State , ISmRuntimeHandler* Handler )
{
	const eg_size_t EGSM_CALL_LIMIT = 1000;	

	State.ExecCount = 0;

	while( State.ProcState == egsm_proc_state::RUNNING && State.Callstack.Len() > 0 )
	{
		EGSmRuntime_HandleNode( State , Handler );

		State.ExecCount++;
		if( State.ExecCount >= EGSM_CALL_LIMIT )
		{
			Handler->SmOnError( "Exec Warning: Calls went for a long time possible infinite loop." );
			State.ProcState = egsm_proc_state::TERMINAL_YIELD;
		}
	}
}

void EGSmRuntime_Run( egsmState& State, eg_string_crc ProcId, eg_string_crc EntryPoint, ISmRuntimeHandler* Handler )
{
	State = egsmState( CT_Clear ); // Whatever was in the state will get blasted.
	State.EntryProc = ProcId;

	if( nullptr == Handler )
	{
		assert( false ); // This is the only error that this machine will assert on since the handler can do what it wants.
		State.ClearToTerminalError();
		return;
	}

	// Insert the first call
	
	const EGSmProc* EntryProc = EGSmMgr_GetProc( ProcId );
	if( nullptr == EntryProc )
	{
		Handler->SmOnError( "Invalid proc." );
		State.ClearToTerminalError();
		return;
	}

	egsm_node_id EntryNode = EntryProc->FindEntryNode( EntryPoint , EntryPoint.IsNull() );
	if( EntryNode.IsNull() )
	{
		Handler->SmOnError( "No entry point." );
		State.ClearToTerminalError();
		return;
	}

	State.Callstack.Push( egsmStackInfo( ProcId , EntryNode ) );
	State.ProcState = egsm_proc_state::RUNNING;

	EGSmRuntime_Exec( State , Handler );
}

void EGSmRuntime_Resume( egsmState& State , eg_string_crc BranchChoiceMade , ISmRuntimeHandler* Handler )
{
	if( nullptr == Handler )
	{
		assert( false ); // This is the only error that this machine will assert on since the handler can do what it wants.
		State.ClearToTerminalError();
		return;
	}

	eg_bool bGoodState = State.ProcState == egsm_proc_state::YIELD || State.ProcState == egsm_proc_state::TERMINAL_YIELD;
	bGoodState = bGoodState && State.Callstack.Len() > 0;
	if( !bGoodState )
	{
		Handler->SmOnError( "Resume Error: Bad state." );
		State.ClearToTerminalError();
		return;
	}

	if( State.ProcState == egsm_proc_state::YIELD )
	{
		const EGSmProc* CurProc = EGSmMgr_GetProc( State.Callstack.Top().ScriptProc );
		if( nullptr == CurProc )
		{
			Handler->SmOnError( "Resume Error: No proc." );
			State.ClearToTerminalError();
			return;
		}

		const egsmNodeBc* CurNode = CurProc->GetNode( State.Callstack.Top().CurrentNode );
		if( nullptr == CurNode )
		{
			Handler->SmOnError( "Resume Error: No current node." );
			State.ClearToTerminalError();
			return;
		}

		eg_size_t BranchChoiceIndex = EGSM_INVALID_INDEX;
		if( BranchChoiceMade.IsNull() )
		{
			BranchChoiceIndex = 0;
		}
		else
		{
			for( eg_size_t i=0; i < CurNode->NumBranches; i++ )
			{
				const egsmBranchBc* Branch = CurProc->GetBranch( CurNode , i );
				if( Branch && Branch->Id == BranchChoiceMade )
				{
					BranchChoiceIndex = i;
					break;
				}
			}
		}

		if( EGSM_INVALID_INDEX == BranchChoiceIndex )
		{
			BranchChoiceIndex = 0;
			Handler->SmOnError( "Resume Error: Branch chosen was invalid, using DEFAULT." );
		}

		const egsmBranchBc* Branch = CurProc->GetBranch( CurNode , BranchChoiceIndex );
		if( nullptr == Branch )
		{
			Handler->SmOnError( "Resume Error: Branch was not found." );
			State.ClearToTerminalError();
			return;
		}

		State.Callstack.Top().CurrentNode = egsm_node_id(Branch->ToNodeAsIndexPlusOne);
		if( Branch->bTerminal )
		{
			State.ProcState = egsm_proc_state::TERMINAL_YIELD;
			return;
		}
	}

	State.ProcState = egsm_proc_state::RUNNING;

	EGSmRuntime_Exec( State , Handler );
}

egsm_var ISmRuntimeHandler::SmResolveParm( const egsm_var& Parm )
{
	if( Parm.GetType() == egsm_var_t::CRC )
	{
		// If the var exists return the resolved var
		// otherwise return the crc as the parm.
		if( SmDoesVarExist( Parm.as_crc() ) )
		{
			return SmGetVar( Parm.as_crc() );
		}
		else
		{
			return Parm;
		}
	}
	else if( Parm.GetType() == egsm_var_t::INT || Parm.GetType() == egsm_var_t::REAL || Parm.GetType() == egsm_var_t::BOOL )
	{
		return Parm;
	}

	return egsm_var( static_cast<eg_int>(0) );
}
