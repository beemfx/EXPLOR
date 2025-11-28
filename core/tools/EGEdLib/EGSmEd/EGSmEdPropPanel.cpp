// (c) 2016 Beem Media

#include "EGSmEdPropPanel.h"
#include "EGSmEd.h"
#include "EGWndPropControl_Text.h"
#include "EGWndPropControl_Bool.h"
#include "EGWndPropControl_ComboBox.h"
#include "EGSmEdScriptPanel.h"

EGSmEdPropPanel::EGSmEdPropPanel( EGWndPanel* Parent , egsm_panel_t Type , ISmEdApp* App )
: Super( Parent )
, m_Type( Type )
, m_App( App )
, m_Focus( CT_Clear )
{
	RefreshLayout();
	FullRedraw();
}

void EGSmEdPropPanel::HandlePropChanged( const egRflEditor* Property )
{
	Super::HandlePropChanged( Property );

	if( m_Type == egsm_panel_t::SETTINGS )
	{
		m_PropSettings.ApplyToSettings( EGSmEd_GetFile().GetMachineProps() );
	}
	else if( m_Type == egsm_panel_t::STATE_PROPS )
	{
		m_PropNode.ApplyNodeData();
		m_App->GetScriptPanel()->OnStatesChanged( true );
	}

	m_App->SetDirty();
}

void EGSmEdPropPanel::RefreshLayout()
{
	SetEditObject( nullptr );

	if( m_Type == egsm_panel_t::SETTINGS )
	{
		m_PropSettings.SetFromSettings( EGSmEd_GetFile().GetMachineProps() );
		SetEditObject( &m_PropSettings.Editor );
	}
	else if( m_Type == egsm_panel_t::STATE_PROPS )
	{
		m_PropNode.SetNodeData( m_Focus );
		SetEditObject( m_PropNode.GetEditor() );
	}

	OnScrollChanged( 0 , 0 );
}

void EGSmEdPropPanel::OnFocusChanged( const ISmEdApp::egFocus& NewFocus )
{
	assert( m_Type == egsm_panel_t::STATE_PROPS );

	m_Focus = NewFocus;
	RefreshLayout();
}

///////////////////////////////////////////////////////////////////////////////

void EGSmEdPropSettings::SetFromSettings( const egsmMachinePropsScr& Settings )
{
	ScriptProcessId = Settings.Id;
	LocTextPrefix = Settings.NewLocKeyPrefix;
}

void EGSmEdPropSettings::ApplyToSettings( egsmMachinePropsScr& Settings )
{
	Settings.Id = *ScriptProcessId;
	Settings.NewLocKeyPrefix = *LocTextPrefix;
}

///////////////////////////////////////////////////////////////////////////////

void EGSmEdPropNode::SetNodeData( const ISmEdApp::egFocus& Data )
{
	ClearChildren();

	m_Focus = Data;
	m_FunctionCall.DeclType = egsm_var_decl_t::UNK;

	const eg_size_t NumStates = EGSmEd_GetFile().GetNodeCount();

	if( EG_IsBetween<eg_size_t>( m_Focus.StateIndex , 0 , NumStates-1 ) )
	{
		const egsmNodeScr& State = EGSmEd_GetFile().GetNode( m_Focus.StateIndex );
		eg_bool bEditingState = m_Focus.ChoiceIndex == ISmEdApp::NO_FOCUS_CHOICE;
		if( bEditingState )
		{
			m_FunctionCall = EGSmEdVarMgr_GetFunctionInfo( eg_string_crc(State.Parms[0]) );

			auto PopulateNativeEventParms =[this,&State]() -> void
			{
				m_Properties.Append( new EGSmEdLabelProp( "Event" , State.Parms[0] ) );

				if( m_FunctionCall.DeclType == egsm_var_decl_t::UNK )
				{
					m_Properties.Append( new EGSmEdLabelProp( "Parms" , "Class Error: Missing function" ) );
				}
				else
				{
					eg_bool bHasEnBox = false;
					for( eg_uint i=0; i<countof(m_FunctionCall.ParmInfo); i++ )
					{
						const egsmVarDeclParmScr& Info = m_FunctionCall.ParmInfo[i];
						if( Info.Type != egsm_var_t::UNK )
						{
							m_Properties.Append( new EGSmEdParmProp( Info , i , State.Parms[i+1] ) );

							if( Info.Type == egsm_var_t::LOC_TEXT )
							{
								bHasEnBox = true;
							}
						}
					}

					if( bHasEnBox )
					{
						m_Properties.Append( new EGSmEdPropBase( "English Text" , eg_edit_ctrl_t::BigText , egsmed_prop_edit_thing::StateEnUs , State.EnRef ) );
					}
				}
			};
			
			switch( State.Type )
			{
				case egsm_node_t::NATIVE_EVENT:
				{
					PopulateNativeEventParms();
				} break;
				case egsm_node_t::ENTRY_POINT:
				{
					m_Properties.Append( new EGSmEdPropBase( "Label" , eg_edit_ctrl_t::Text , egsmed_prop_edit_thing::StateParm0 , State.Parms[0] ) );
				} break;
				case egsm_node_t::CALL:
				{
					m_Properties.Append( new EGSmEdPropBase( "Module ID" , eg_edit_ctrl_t::Text , egsmed_prop_edit_thing::StateParm0 , State.Parms[0] ) );
					m_Properties.Append( new EGSmEdPropBase( "Entry Point Label" , eg_edit_ctrl_t::Text , egsmed_prop_edit_thing::StateParm1 , State.Parms[1] ) );
				} break;
				default:
				{
				}break;
			}
		}
		else
		{
			auto PopulateNativeEventBranch = [this, &State]() -> void
			{
				egsmVarDeclScr VarDecl = EGSmEdVarMgr_GetFunctionInfo( eg_string_crc( State.Parms[0] ) );

				if( State.Branches.IsValidIndex( m_Focus.ChoiceIndex ) )
				{
					eg_bool bHasEnBox = VarDecl.ReturnType == egsm_var_t::LOC_TEXT;
					if( VarDecl.ReturnType != egsm_var_t::BOOL && VarDecl.ReturnType != egsm_var_t::RETURN_VOID && VarDecl.ReturnType != egsm_var_t::UNK )
					{
						m_Properties.Append( new EGSmEdPropBase( "Result Label" , eg_edit_ctrl_t::Text , egsmed_prop_edit_thing::BranchLabel , State.Branches[m_Focus.ChoiceIndex].Id ) );
					}

					if( bHasEnBox )
					{
						m_Properties.Append( new EGSmEdPropBase( "English Text" , eg_edit_ctrl_t::BigText , egsmed_prop_edit_thing::BranchEnUs , State.Branches[m_Focus.ChoiceIndex].EnRef ) );
					}
				}
			};

			switch( State.Type )
			{
				case egsm_node_t::NATIVE_EVENT:
				{
					PopulateNativeEventBranch();
				} break;
				default:
				{
				} break;
			}

			m_Properties.Append( new EGSmEdPropBase( "Is Terminal" , eg_edit_ctrl_t::Bool , egsmed_prop_edit_thing::BranchTerminal , State.Branches[m_Focus.ChoiceIndex].bTerminal ? "TRUE" : "FALSE" ) );
		}
	}

	m_Editor = egRflEditor( "Node" , this , &Rfl_CustomEditor );
	for( EGSmEdPropBase* Prop : m_Properties )
	{
		m_Editor.AddExplicitChild( egRflEditor( *Prop->GetVarName() , Prop , Prop ) );
	}
}

void EGSmEdPropNode::ApplyNodeData()
{
	auto SetParm = [this]( egsmNodeScr& State , eg_size_t ParmIndex , EGSmEdPropBase* Prop ) -> void
	{
		eg_d_string Refactored = Prop->ToString( nullptr );

		// Could check to see if type is allowed.
		if( m_FunctionCall.DeclType == egsm_var_decl_t::FUNCTION && EG_IsBetween<eg_size_t>( ParmIndex , 1 , countof(m_FunctionCall.ParmInfo) ) )
		{
			eg_bool bIsRef = m_FunctionCall.ParmInfo[ParmIndex-1].bIsRef;
			egsm_var_t VarType = m_FunctionCall.ParmInfo[ParmIndex-1].Type;

			eg_bool bAllowed = false;
			EGArray<egsmVarDeclScr> AllowedVars;
			EGSmEdVarMgr_GetVarsOfType( VarType , AllowedVars );

			if( VarType == egsm_var_t::BOOL && ( Refactored.EqualsI("true") || Refactored.EqualsI("false") ) )
			{
				bAllowed = true;
			}

			if( !bAllowed )
			{
				for( const egsmVarDeclScr& Decl : AllowedVars )
				{
					if( Refactored.Equals(Decl.Name) )
					{
						bAllowed = true;
						break;
					}
				}
			}

			if( !bAllowed && Refactored.Len() == 0 )
			{
				bAllowed = true;
			}

			if( !bAllowed && (VarType == egsm_var_t::CRC || VarType == egsm_var_t::LOC_TEXT) )
			{
				bAllowed = true;
			}

			if( !bAllowed && VarType == egsm_var_t::INT && EGString_IsInteger( *Refactored ) )
			{
				bAllowed = true;
			}

			if( !bAllowed && (VarType == egsm_var_t::REAL || VarType == egsm_var_t::NUMBER) && EGString_IsNumber( *Refactored ) )
			{
				bAllowed = true;
			}

			if( !bAllowed )
			{
				Refactored = "";
			}
		}

		Prop->SetFromString( nullptr , *Refactored );
		State.Parms[ParmIndex] = *Refactored;
	};

	const eg_size_t NumStates = EGSmEd_GetFile().GetNodeCount();

	if( EG_IsBetween<eg_size_t>( m_Focus.StateIndex , 0 , NumStates-1 ) )
	{
		egsmNodeScr& State = EGSmEd_GetFile().GetNode( m_Focus.StateIndex );
		eg_bool bEditingState = m_Focus.ChoiceIndex == ISmEdApp::NO_FOCUS_CHOICE;
		if( bEditingState )
		{
			for( EGSmEdPropBase* Prop : m_Properties )
			{
				if( Prop )
				{
					switch( Prop->GetEditThing() )
					{
						default:
							assert( false );
							break;
						case egsmed_prop_edit_thing::None:
							break;
						case egsmed_prop_edit_thing::StateParm0:
							SetParm( State , 0 , Prop );
							break;
						case egsmed_prop_edit_thing::StateParm1:
							SetParm( State , 1 , Prop );
							break;
						case egsmed_prop_edit_thing::StateParm2:
							SetParm( State , 2 , Prop );
							break;
						case egsmed_prop_edit_thing::StateParm3:
							SetParm( State , 3 , Prop );
							break;
						case egsmed_prop_edit_thing::StateEnUs:
							State.EnRef = *Prop->ToString( nullptr );
							break;
					}
				}
			}
		}
		else
		{
			if( State.Branches.IsValidIndex( m_Focus.ChoiceIndex ) )
			{
				egsmBranchScr& Branch = State.Branches[m_Focus.ChoiceIndex];

				for( EGSmEdPropBase* Prop : m_Properties )
				{
					if( Prop )
					{
						switch( Prop->GetEditThing() )
						{
							case egsmed_prop_edit_thing::None:
								break;
							case egsmed_prop_edit_thing::BranchLabel:
								Branch.Id = *Prop->ToString( nullptr );
								break;
							case egsmed_prop_edit_thing::BranchEnUs:
								Branch.EnRef = *Prop->ToString( nullptr );
								break;
							case egsmed_prop_edit_thing::BranchTerminal:
								Branch.bTerminal = EGString_ToBool( *Prop->ToString( nullptr) );
								break;
							default:
								assert( false );
								break;

						}
					}
				}
			}
		}
	}
	else
	{
		assert( false );
	}
}

void EGSmEdPropNode::ClearChildren()
{
	m_Editor = CT_Clear;
	for( EGSmEdPropBase* Prop : m_Properties )
	{
		delete Prop;
	}
	m_Properties.Clear();
}

///////////////////////////////////////////////////////////////////////////////

EGSmEdParmProp::EGSmEdParmProp( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex , eg_cpstr InitialValue )
: m_ParmIndex( ParmIndex )
, m_VarInfo( VarInfo )
, EGSmEdPropBase( *ComputeControlLabel( VarInfo , ParmIndex ) , ComputeControlType( VarInfo , ParmIndex ) , ComputeEditThing( VarInfo , ParmIndex ) , InitialValue )
{

}

void EGSmEdParmProp::GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const
{
	unused( Data );

	bManualEditOut = true;

	egsm_var_t VarType = m_VarInfo.Type;
	eg_bool bIsRef = m_VarInfo.bIsRef;

	if( VarType == egsm_var_t::BOOL && !bIsRef )
	{
		bManualEditOut = false;
		Out.Append( "true" );
		Out.Append( "false" );
	}
	else
	{
		Out.Append( "" );
	}

	EGArray<egsmVarDeclScr> Decls;
	EGSmEdVarMgr_GetVarsOfType( VarType , Decls );
	for( const egsmVarDeclScr& Decl : Decls )
	{
		Out.Append( Decl.Name.String() );
	}

	if( m_VarInfo.bIsRef )
	{
		bManualEditOut = false;
	}
}

eg_edit_ctrl_t EGSmEdParmProp::ComputeControlType( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex )
{
	unused( ParmIndex );

	if( VarInfo.bIsRef )
	{
		return eg_edit_ctrl_t::Enum;
	}

	if( VarInfo.Type == egsm_var_t::LOC_TEXT )
	{
		return eg_edit_ctrl_t::Text;
	}

	return eg_edit_ctrl_t::Combo;
}

eg_d_string EGSmEdParmProp::ComputeControlLabel( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex )
{
	unused( ParmIndex );

	eg_string_big VarStr = VarInfo.Name;
	if( VarInfo.bIsRef && !VarInfo.bIsConst )
	{
		VarStr.Append( " (OUTPUT)" );
	}

	return *VarStr;
}

egsmed_prop_edit_thing EGSmEdParmProp::ComputeEditThing( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex )
{
	unused( VarInfo );
	
	switch( ParmIndex )
	{
		case 0: return egsmed_prop_edit_thing::StateParm1;
		case 1: return egsmed_prop_edit_thing::StateParm2;
		case 2: return egsmed_prop_edit_thing::StateParm3;
	}

	return egsmed_prop_edit_thing::None;
}
