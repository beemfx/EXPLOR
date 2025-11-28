// (c) 2017 Beem Media

#pragma once

#include "EGSmTypes.h"
#include "EGXMLBase.h"

class EGFileData;

class EGSmFile2 : public IXmlBase
{
private:

	egsmMachinePropsScr  m_MachineProps;
	EGArray<egsmNodeScr> m_Nodes;
	egsmNodeScr          m_GarbageNode;

public:

	EGSmFile2( const EGSmFile2& rhs ) = delete;

	EGSmFile2();
	~EGSmFile2();

	void Clear();
	void InitDefault();

	void Load( eg_cpstr Filename );
	void LoadForTool( eg_cpstr16 Filename );
	void LoadFromMemFile( const class EGFileData& MemFile );
	eg_bool Save( eg_cpstr16 Filename ) const;
	eg_bool SaveEnLoc( eg_cpstr16 Filename ) const;
	void SaveEnLoc( class EGLocCompiler& LocCompiler ) const;
	void FixLocIds();

	eg_bool CompileToByteCode( EGFileData& Out ) const;

	eg_bool VerifyIntegrity()const;
	void FixStateNames();

	egsmMachinePropsScr& GetMachineProps(){ return m_MachineProps; }
	const egsmMachinePropsScr& GetMachineProps() const { return m_MachineProps; }

	eg_size_t GetNodeCount() const { return m_Nodes.Len(); }
	const egsmNodeScr& GetNode( eg_size_t Index ) const { return m_Nodes.IsValidIndex( Index ) ? m_Nodes[Index] : m_GarbageNode; }
	egsmNodeScr& GetNode( eg_size_t Index ) { return m_Nodes.IsValidIndex( Index ) ? m_Nodes[Index] : m_GarbageNode; }
	eg_size_t GetNodeIndexById( eg_cpstr Id ) const;

	void SetDefaultEntryPoint( const egsmNodeScr& Node );
	egsmNodeScr& CreateNode( egsm_node_t Type ); // After creating a node, all references and pointers to the file are invalid.
	void DeleteNode( eg_cpstr Id );
	void InsertBranchInNode( egsmNodeScr& Node , eg_cpstr OptionalId , egsm_var_t VarType = egsm_var_t::UNK );
	eg_string GetNextLocTextId();

	static eg_string NodeTypeToString( egsm_node_t Type );
	static eg_string NodeTypeToDisplayString( egsm_node_t Type );
	static egsm_node_t NodeStringToType( eg_cpstr Str );


private:

	enum class egsm_suffix_t
	{
		NODE_ID,
		LOC_TEXT,
	};

private:

	void RenameNode( eg_cpstr InOldName , eg_cpstr InNewName );
	eg_uint GenerateSuffix( egsm_suffix_t Type );

	virtual void OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet ) override;
	virtual eg_cpstr XMLObjName() const override { return "EGSmFile2"; }

};