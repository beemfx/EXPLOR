// (c) 2017 Beem Media

#pragma once

#include "EGWndPanelPropEditor.h"
#include "EGWndPropControl.h"
#include "EGSmEd.h"
#include "EGSmEdPropPanel.reflection.h"

class EGWndPropControl;
class ISmEdApp;
class EGSmEdPropBase;


enum class egsmed_prop_edit_thing
{
	None,

	BranchLabel,
	BranchEnUs,
	BranchTerminal,

	StateParm0,
	StateParm1,
	StateParm2,
	StateParm3,
	StateEnUs,
};

class EGSmEdPropBase : public egRflValueType
{
private:

	eg_edit_ctrl_t         m_ControlType = eg_edit_ctrl_t::Unk;
	egsmed_prop_edit_thing m_EditThing = egsmed_prop_edit_thing::None;
	const eg_d_string      m_VarName;
	mutable eg_d_string    m_Value;

public:

	EGSmEdPropBase() = default;
	EGSmEdPropBase( eg_cpstr InVarName , eg_edit_ctrl_t ControlType , egsmed_prop_edit_thing EditThing , eg_cpstr InitialValue )
	: m_VarName( InVarName )
	, m_ControlType( ControlType )
	, m_EditThing( EditThing )
	, m_Value( InitialValue )
	{

	}

	virtual eg_edit_ctrl_t GetEditControlType() const override final { return m_ControlType; }
	virtual eg_d_string ToString( const void* Data ) const override final { unused( Data ); return m_Value; }
	virtual void SetFromString( void* Data , eg_cpstr Str ) const override final { unused( Data ); m_Value = Str; }
	egsmed_prop_edit_thing GetEditThing() const { return m_EditThing; }
	virtual eg_uint GetParmIndex() const { return 0; }
	const eg_d_string& GetVarName() const { return m_VarName; }
	const eg_d_string& GetValue() const { return m_Value; }
};

class EGSmEdLabelProp : public EGSmEdPropBase
{
public:

	EGSmEdLabelProp( eg_cpstr InVarName , eg_cpstr Label )
	: EGSmEdPropBase( InVarName , eg_edit_ctrl_t::StaticText , egsmed_prop_edit_thing::None , Label )
	{
	}
};

class EGSmEdParmProp : public EGSmEdPropBase
{
private:

	eg_uint            m_ParmIndex = 0;
	egsmVarDeclParmScr m_VarInfo;

public:

	EGSmEdParmProp( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex , eg_cpstr InitialValue );

	virtual eg_uint GetParmIndex() const override { return m_ParmIndex; }
	virtual void GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const override;

private:

	static eg_edit_ctrl_t ComputeControlType( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex );
	static eg_d_string ComputeControlLabel( const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex );
	static egsmed_prop_edit_thing ComputeEditThing(  const egsmVarDeclParmScr& VarInfo , eg_uint ParmIndex );

};

egreflect struct EGSmEdPropSettings
{
	egprop eg_d_string ScriptProcessId = "";
	egprop eg_d_string LocTextPrefix   = "";

	egRflEditor Editor;

	EGSmEdPropSettings()
	{
		Editor = EGReflection_GetEditor( *this , "Settings" );
	}

	void SetFromSettings( const egsmMachinePropsScr& Settings );
	void ApplyToSettings( egsmMachinePropsScr& Settings );
};

class EGSmEdPropNode
{
private:

	EGArray<EGSmEdPropBase*> m_Properties;
	egsmVarDeclScr           m_FunctionCall;
	ISmEdApp::egFocus        m_Focus;
	egRflEditor              m_Editor;

public:

	EGSmEdPropNode()
	: m_Focus( CT_Clear ) 
	{
		m_FunctionCall.DeclType = egsm_var_decl_t::UNK;
	}
	~EGSmEdPropNode(){ ClearChildren(); }

	void SetNodeData( const ISmEdApp::egFocus& Data );
	void ApplyNodeData();

	const egsmVarDeclScr& GetFnCall() const { return m_FunctionCall; }

	egRflEditor* GetEditor() { return &m_Editor; }

private:

	void ClearChildren();
};

class EGSmEdPropPanel : public EGWndPanelPropEditor
{
	EG_DECL_SUPER( EGWndPanelPropEditor )

private:

	using EGWndPanelPropEditor::RefreshAll;

public:
	
	enum class egsm_panel_t
	{
		SETTINGS,
		STATE_PROPS,
	};


private:

	const egsm_panel_t m_Type;
	ISmEdApp*const     m_App;
	ISmEdApp::egFocus  m_Focus;
	EGSmEdPropSettings m_PropSettings;
	EGSmEdPropNode     m_PropNode;

public:

	EGSmEdPropPanel( EGWndPanel* Parent , egsm_panel_t Type , ISmEdApp* App );

	virtual void HandlePropChanged( const egRflEditor* Property ) override;

	void RefreshLayout();
	void OnFocusChanged( const ISmEdApp::egFocus& NewFocus );
};