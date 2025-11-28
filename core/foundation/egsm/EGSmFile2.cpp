// (c) 2017 Beem Media

#include "EGSmFile2.h"
#include "EGFileData.h"
#include "EGToolsHelper.h"
#include "EGCrcDb.h"
#include "EGLocCompiler.h"
#include "EGEdUtility.h"
#include "EGLibFile.h"

EGSmFile2::EGSmFile2() 
: IXmlBase()
, m_MachineProps()
, m_Nodes()
{
	m_GarbageNode.Type = egsm_node_t::ENTRY_POINT;
	m_GarbageNode.Id = "GARBAGE";
	m_GarbageNode.bUsed = true;
}

EGSmFile2::~EGSmFile2()
{

}

void EGSmFile2::Clear()
{
	m_MachineProps = egsmMachinePropsScr();
	m_Nodes.Clear();
}

void EGSmFile2::InitDefault()
{
	egsmNodeScr& NewNode = CreateNode( egsm_node_t::ENTRY_POINT );
	NewNode.Parms[0] = "Main";
	NewNode.EditorPos = eg_vec2( 10.f, 150.f );
	SetDefaultEntryPoint( NewNode );
}

void EGSmFile2::Load( eg_cpstr Filename )
{
	Clear();
	XMLLoad( Filename );
}

void EGSmFile2::LoadForTool( eg_cpstr16 Filename )
{
	Clear();
	
	EGFileData File( eg_file_data_init_t::HasOwnMemory );
	EGLibFile_OpenFile( Filename , eg_lib_file_t::OS , File );
	eg_string_big FilenameSm = Filename;
	if( File.GetSize() > 0 ) // If it's an empty file we'll just treat it as freshly created.
	{
		XMLLoad( File.GetData() , File.GetSize() , FilenameSm );
	}
}

void EGSmFile2::LoadFromMemFile( const EGFileData& MemFile )
{
	Clear();

	XMLLoad( MemFile.GetData() , MemFile.GetSize() , "EGSM File" );
}

eg_bool EGSmFile2::Save( eg_cpstr16 Filename ) const
{
	EGFileData Out( eg_file_data_init_t::HasOwnMemory );

	auto Write = [&Out]( auto... Args ) -> void
	{
		eg_char8 Buffer[1024];
		EGString_FormatToBuffer( Buffer , countof(Buffer) , Args... );
		Out.WriteStr8( Buffer );
	};

	auto AddAttributeS = [&Write]( eg_cpstr Name , const eg_string_base& Value ) -> void
	{
		eg_string_big XmlValue;
		eg_string::ToXmlFriendly( Value , XmlValue );
		Write( "%s=\"%s\" " , Name , XmlValue.String() );
	};

	auto AddAttributeB = [&Write]( eg_cpstr Name , const eg_bool Value ) -> void
	{
		Write( "%s=\"%s\" " , Name , Value ? "TRUE" : "FALSE" );
	};

	auto AddAttributeI = [&Write]( eg_cpstr Name , const eg_int Value ) -> void
	{
		Write( "%s=\"%i\" " , Name , Value );
	};

	auto AddAttributeV2 = [&Write]( eg_cpstr Name , const eg_vec2& Value ) -> void
	{
		Write( "%s=\"%g %g\" " , Name , Value.x , Value.y );
	};

	Write( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
	Write( "<!-- THIS FILE SCRIPT GENERATED: DO NOT EDIT BY HAND OR YOU RISK\r\nCORRUPTING FURTHER USE OF THIS SCRIPT IN GAME OR TOOL, AND MAY\r\nCAUSE SYSTEMS INTERPRETING IT TO CRASH. -->\r\n" );
	Write( "<egsm " );
	AddAttributeS( "id" , m_MachineProps.Id );
	AddAttributeS( "default_entry_point_node" , m_MachineProps.DefaultEntryPointNode );
	AddAttributeS( "new_node_id_prefix" , m_MachineProps.NewNodeIdPrefix );
	AddAttributeS( "new_loc_key_prefix" , m_MachineProps.NewLocKeyPrefix );
	AddAttributeI( "new_node_id_suffix" , m_MachineProps.NewNodeIdSuffix );
	AddAttributeI( "new_loc_key_suffix" , m_MachineProps.NewLocKeySuffix );
	AddAttributeV2( "view_offset" , m_MachineProps.ViewOffset );
	Write( ">\r\n" );

	for( const egsmNodeScr& Node : m_Nodes )
	{
		if( !Node.bUsed )
		{
			continue;
		}

		Write( "\t<node " );
		AddAttributeS( "id" , Node.Id );
		AddAttributeS( "type" , NodeTypeToString(Node.Type) );
		for( eg_size_t i=0; i<countof(Node.Parms); i++ )
		{
			if( Node.Parms[i].Len() > 0 )
			{
				AddAttributeS( EGString_Format("parm%u",i) , Node.Parms[i] );
			}
		}
		AddAttributeS( "en_ref" , Node.EnRef );
		AddAttributeV2( "editor_pos" , Node.EditorPos );
		Write( ">\r\n" );

		for( const egsmBranchScr& Branch : Node.Branches )
		{
			Write( "\t\t<branch " );
			AddAttributeS( "id" , Branch.Id );
			AddAttributeS( "to_node" , Branch.ToNode );
			AddAttributeB( "is_terminal" , Branch.bTerminal );
			AddAttributeS( "en_ref" , Branch.EnRef );
			Write( " />\r\n" );
		}

		Write( "\t</node>\r\n" );
	}

	Write( "</egsm>\r\n" );

	return EGEdUtility_SaveFile( Filename , Out );
}

void EGSmFile2::OnTag( const eg_string_base& Tag, const EGXmlAttrGetter& AttGet )
{
	auto OnTag_egsm = [this,&AttGet]() -> void
	{
		m_MachineProps.Id = AttGet.GetString( "id" );
		m_MachineProps.DefaultEntryPointNode = AttGet.GetString( "default_entry_point_node" );
		m_MachineProps.NewNodeIdPrefix = AttGet.GetString( "new_node_id_prefix" );
		m_MachineProps.NewLocKeyPrefix = AttGet.GetString( "new_loc_key_prefix" , "LOCID_" );
		m_MachineProps.NewNodeIdSuffix = AttGet.GetUInt( "new_node_id_suffix" );
		m_MachineProps.NewLocKeySuffix = AttGet.GetUInt( "new_loc_key_suffix" );
		AttGet.GetVec( "view_offset" , &m_MachineProps.ViewOffset , 2 , false );
	};

	auto OnTag_node = [this,&AttGet]() -> void
	{
		const eg_size_t NodeIndex = m_Nodes.Len();
		m_Nodes.ExtendToAtLeast( NodeIndex + 1 );
		egsmNodeScr& Node = m_Nodes[NodeIndex];

		Node.bUsed = true;
		Node.Id = AttGet.GetString( "id" );
		Node.Type  = NodeStringToType( AttGet.GetString( "type" ) );
		for( eg_size_t i=0; i<countof(Node.Parms); i++ )
		{
			Node.Parms[i] = AttGet.GetString( EGString_Format("parm%u",i) );
		}
		Node.EnRef = AttGet.GetString( "en_ref" );
		Node.EditorPos = eg_vec2( 0.f , 60.f*NodeIndex ); // In case no position, space it out.
		AttGet.GetVec( "editor_pos" , &Node.EditorPos , 2 , false );
	};

	auto OnTag_branch = [this,&AttGet]() -> void
	{
		// Choice will be for the last node added.
		if( m_Nodes.Len() > 0 )
		{
			const eg_size_t NodeIndex = m_Nodes.Len() - 1;
			egsmNodeScr& Node = m_Nodes[NodeIndex];
			const eg_size_t BranchIndex = Node.Branches.Len();
			Node.Branches.ExtendToAtLeast( BranchIndex + 1 );
			egsmBranchScr& Branch = Node.Branches[BranchIndex];

			Branch.Id = AttGet.GetString( "id" );
			Branch.ToNode = AttGet.GetString ( "to_node" );
			Branch.bTerminal = AttGet.GetBool( "is_terminal" );
			Branch.EnRef = AttGet.GetString( "en_ref" );
		}
		else
		{
			assert( false ); // Badly formed?
		}
	};

	eg_string_crc TagAsCrc( Tag );
	switch_crc( TagAsCrc )
	{
		case_crc( "egsm" ): OnTag_egsm(); break;
		case_crc( "node" ): OnTag_node(); break;
		case_crc( "branch" ): OnTag_branch(); break;
		default: assert( false ); break;
	}
}

eg_bool EGSmFile2::SaveEnLoc( eg_cpstr16 Filename ) const
{
	EGFileData Out( eg_file_data_init_t::HasOwnMemory );
	EGLocCompiler LocCompiler;
	EGFileData CurFile( eg_file_data_init_t::HasOwnMemory );
	EGToolsHelper_OpenFile( Filename , CurFile );

	if( CurFile.GetSize() == 0 )
	{
		LocCompiler.InitAsEnUS();
	}
	else
	{
		LocCompiler.LoadLocFile( CurFile , "EGSM Text" );
	}

	SaveEnLoc( LocCompiler );

	LocCompiler.SaveXml( Out );

	return EGEdUtility_SaveFile( Filename , Out );
}

void EGSmFile2::SaveEnLoc( class EGLocCompiler& LocCompiler ) const
{
	struct egPairing
	{
		eg_string_small Key;
		eg_string_big   Text;
	};

	EGArray<egPairing> ToExport;

	auto AddPair = [&ToExport]( eg_cpstr Key , eg_cpstr Text )
	{
		egPairing Pair;
		Pair.Key = Key;
		Pair.Text = Text;
		if( Pair.Key.Len() > 0 && Pair.Text.Len() > 0 )
		{
			eg_bool bHasDuplicate = false;
			for( eg_size_t i=0; i<ToExport.Len(); i++ )
			{
				if( ToExport[i].Key == Key )
				{
					bHasDuplicate = true;

					if( ToExport[i].Text != Text )
					{
						if( ToExport[i].Text.Len() > 0 && EGString_StrLen( Text ) > 0 )
						{
							EGLogf( eg_log_t::Warning , __FUNCTION__ ": \"%s\" and \"%s\" both have the same key." , ToExport[i].Text.String() , Text );
							// assert( false ); // Duplicate key pairing, we should show error not assert.
						}
					}
				}
			}

			if( !bHasDuplicate )
			{
				ToExport.Append( Pair );
			}
		}
	};

	for( const egsmNodeScr& Node : m_Nodes )
	{
		if( !Node.bUsed )
		{
			continue;
		}

		if( Node.Type == egsm_node_t::NATIVE_EVENT ) // We only care about text that is part of DIALOG states.
		{
			if( 0 <= Node.EnParmIndex && Node.EnParmIndex <  countof(Node.Parms) )
			{
				AddPair( Node.Parms[Node.EnParmIndex] , Node.EnRef ); // Parm[0] is the dialog
			}

			for( const egsmBranchScr& Branch : Node.Branches )
			{
				AddPair( Branch.Id , Branch.EnRef ); // Each branch is also dialog
			}
		}
	}

	for( const egPairing& Pair : ToExport )
	{
		LocCompiler.AddOrReplaceEntry( Pair.Key , EGString_ToWide(Pair.Text) );
	}
}

void EGSmFile2::FixLocIds()
{
	m_MachineProps.NewLocKeySuffix = 0;

	auto CreateNewLocId = [this]() -> eg_string_small
	{
		eg_string_big Out = EGString_Format( "%s%u" , m_MachineProps.NewLocKeyPrefix.String() , GenerateSuffix( egsm_suffix_t::LOC_TEXT ) );
		return Out;
	};

	auto Fix = [this,&CreateNewLocId]( eg_string_small& StringId , eg_string_big& EnText ) -> void
	{
		if( EnText.Len() > 0 )
		{
			StringId = CreateNewLocId();
		}
		else if( StringId.EqualsCount( m_MachineProps.NewLocKeyPrefix , m_MachineProps.NewLocKeyPrefix.Len() ) )
		{
			StringId = CreateNewLocId();
		}
	};
	
	for( egsmNodeScr& Node : m_Nodes )
	{
		if( !Node.bUsed )
		{
			continue;
		}

		if( Node.Type == egsm_node_t::NATIVE_EVENT )
		{
			if( 0 <= Node.EnParmIndex && Node.EnParmIndex <  countof(Node.Parms) )
			{
				Fix( Node.Parms[Node.EnParmIndex] , Node.EnRef ); // Parm[0] is the dialog
			}

			for( egsmBranchScr& Branch : Node.Branches )
			{
				Fix( Branch.Id , Branch.EnRef ); // Each branch is also dialog
			}
		}
	}
}

eg_bool EGSmFile2::CompileToByteCode( EGFileData& Out ) const
{
	EGArray<egsmNodeBc>   Nodes;
	EGArray<egsmBranchBc> Branches;
	EGArray<egsmLabelBc>  Labels;
	EGArray<eg_char>      DebugSymbols;

	eg_string_crc DefaultEntryPointNodeId = eg_string_crc(m_MachineProps.DefaultEntryPointNode);

	auto GetNodeIndexPlusOne = [&Nodes]( eg_string_crc CrcId ) -> eg_size_t
	{
		if( CrcId.IsNull() )
		{
			return 0; // Quick return if we know it's null.
		}

		for( eg_size_t i=0; i<Nodes.Len(); i++ )
		{
			if( Nodes[i].Id == CrcId )
			{
				return i+1;
			}
		}
		return 0;
	};

	auto AddDebugSymbol =[&DebugSymbols]( const eg_string_base& s ) -> eg_size_t
	{
		eg_size_t Offset = DebugSymbols.Len();
		DebugSymbols.Append( s.String() , s.Len()+1 );
		return Offset;
	};

	for( const egsmNodeScr& Node : m_Nodes )
	{
		if( !Node.bUsed )
		{
			continue;
		}

		egsmNodeBc BcNode( Node );
		BcNode.IdDebugSym = AddDebugSymbol( Node.Id );
		BcNode.FirstBranch = Branches.Len();
		Nodes.Append( BcNode );

		if( BcNode.Type == egsm_node_t::ENTRY_POINT || DefaultEntryPointNodeId == BcNode.Id )
		{
			egsmLabelBc Label;
			eg_string LabelName = BcNode.Type == egsm_node_t::ENTRY_POINT ? Node.Parms[0] : "DefaultEntryPoint";
			Label.LabelName = eg_string_crc(LabelName);
			Label.LabelNameDebugSym = AddDebugSymbol(LabelName);
			Label.NodeToIndexPlusOne = Nodes.Len();
			Label.bIsDefaultEntryPoint = ( DefaultEntryPointNodeId == BcNode.Id );
			Labels.Append( Label );
		}

		for( const egsmBranchScr& Branch : Node.Branches )
		{
			egsmBranchBc BcBranch( Branch );
			BcBranch.IdDebugSym = AddDebugSymbol( Branch.Id );
			Branches.Append( BcBranch );
		}
	}

	// Resolve Indexes:
	for( egsmBranchBc& Branch : Branches )
	{
		Branch.ToNodeAsIndexPlusOne = GetNodeIndexPlusOne( Branch.ToNodeCrc );
	}

	eg_size_t Offset = 0;
	auto GetOffset = [&Offset]( eg_size_t Num , eg_size_t ItemSize ) -> eg_size_t
	{
		eg_size_t ThisOffset = Offset;
		Offset += Num*ItemSize;
		return ThisOffset;
	};

	eg_size_t HeaderOffset = GetOffset( 1 , sizeof(egsmByteCodeHeader) );

	egsmByteCodeHeader ByteCodeHeader;
	ByteCodeHeader.FileType = EGSM_BYTE_CODE_HEADER;
	ByteCodeHeader.Props = egsmMachinePropsBc( m_MachineProps );
	ByteCodeHeader.Props.IdDebugSym = AddDebugSymbol( m_MachineProps.Id );
	ByteCodeHeader.NodeCount = Nodes.Len();
	ByteCodeHeader.NodeOffset = GetOffset( Nodes.Len() , sizeof(egsmNodeBc) );
	ByteCodeHeader.BranchCount = Branches.Len();
	ByteCodeHeader.BranchOffset = GetOffset( Branches.Len() , sizeof(egsmBranchBc) );
	ByteCodeHeader.LabelCount = Labels.Len();
	ByteCodeHeader.LabelOffset = GetOffset( Labels.Len() , sizeof(egsmLabelBc) );
	ByteCodeHeader.DebugSymbolsCharCount = DebugSymbols.Len();
	ByteCodeHeader.DebugSymbolsOffset = GetOffset( DebugSymbols.Len() , sizeof(eg_char) );
	ByteCodeHeader.Nodes = nullptr;
	ByteCodeHeader.Branches = nullptr;
	ByteCodeHeader.Labels = nullptr;
	ByteCodeHeader.DebugSymbols = nullptr;

	Out.Clear();
	Out.Write( &ByteCodeHeader , sizeof(ByteCodeHeader) );
	Out.Write( Nodes.GetArray() , Nodes.Len()*sizeof(egsmNodeBc) );
	Out.Write( Branches.GetArray() , Branches.Len()*sizeof(egsmBranchBc) );
	Out.Write( Labels.GetArray() , Labels.Len()*sizeof(egsmLabelBc) );
	Out.Write( DebugSymbols.GetArray() , DebugSymbols.Len()*sizeof(eg_char) );

	return true;
}

eg_bool EGSmFile2::VerifyIntegrity() const
{
	EGArray<eg_string_crc> AllIds;

	// Check to make sure all node id's are unique
	eg_bool bAllIdsUnique = true;

	for( const egsmNodeScr& Node : m_Nodes )
	{
		eg_string_crc IdAsCrc = eg_string_crc(Node.Id);
		EGCrcDb::AddAndSaveIfInTool( Node.Id );

		if( !Node.bUsed )
		{
			continue;
		}

		if( AllIds.Contains( IdAsCrc ) )
		{
			bAllIdsUnique = false;
			EGLogf( eg_log_t::Warning , __FUNCTION__ ": %s had a collision." , Node.Id.String() );
			break;
		}
		else
		{
			AllIds.Append( IdAsCrc );
		}
	}

	// Now make sure all branches point to valid ids
	eg_bool bAllGotosValid = true;

	for( const egsmNodeScr& Node : m_Nodes )
	{
		eg_string_crc IdAsCrc = eg_string_crc(Node.Id);
		if( !Node.bUsed )
		{
			continue;
		}

		for( const egsmBranchScr& Branch : Node.Branches )
		{
			if( Branch.ToNode == "" )
			{
				continue;
			}

			if( !AllIds.Contains( eg_string_crc(Branch.ToNode) ) )
			{
				bAllGotosValid = false;
				EGLogf( eg_log_t::Warning , __FUNCTION__ ": %s is not a valid goto node." , Branch.ToNode.String() );
				break;
			}
		}
	}

	return bAllIdsUnique && bAllGotosValid;
}

void EGSmFile2::FixStateNames()
{
	for( const egsmNodeScr& Node : m_Nodes )
	{
		eg_string_small NewId = EGString_Format( "__!!%s!!__" , Node.Id.String() );
		RenameNode( Node.Id , NewId );
	}

	m_MachineProps.NewNodeIdSuffix = 0;

	for( const egsmNodeScr& Node : m_Nodes )
	{
		eg_string_small NewId = EGString_Format( "%s%u" , m_MachineProps.NewNodeIdPrefix.String() , GenerateSuffix( egsm_suffix_t::NODE_ID ) );
		RenameNode( Node.Id , NewId );
	}
}

eg_size_t EGSmFile2::GetNodeIndexById( eg_cpstr Id ) const
{
	for( eg_size_t si=0; si<m_Nodes.Len(); si++ )
	{
		if( m_Nodes[si].Id == Id )
		{
			return si;
		}
	}
	return EGSM_INVALID_INDEX;
}

void EGSmFile2::SetDefaultEntryPoint( const egsmNodeScr& Node )
{
	m_MachineProps.DefaultEntryPointNode = Node.Id;
}

egsmNodeScr& EGSmFile2::CreateNode( egsm_node_t Type )
{
	auto FindInsertionPoint = [this]() -> eg_size_t
	{
		for( eg_size_t i=0; i<m_Nodes.Len(); i++ )
		{
			if( !m_Nodes[i].bUsed )
			{
				return i;
			}
		}
		return m_Nodes.Len();
	};
	
	eg_size_t NewNodeIndex = FindInsertionPoint();

	if( !m_Nodes.IsValidIndex( NewNodeIndex ) )
	{
		m_Nodes.Append( egsmNodeScr() );
		assert( m_Nodes.IsValidIndex( NewNodeIndex ) );
	}
	egsmNodeScr& Out = m_Nodes[NewNodeIndex];
	Out.Id = EGString_Format( "%s%u" , m_MachineProps.NewNodeIdPrefix.String() , GenerateSuffix( egsm_suffix_t::NODE_ID ) );
	Out.Type = Type;
	for( eg_string_small& Parm : Out.Parms )
	{
		Parm = "";
	}
	Out.Branches.Clear();
	Out.EnRef = "";
	Out.EnParmIndex = 0;
	Out.EditorPos = eg_vec2(0.f,0.f);
	Out.bUsed = true;

	switch( Out.Type )
	{
		case egsm_node_t::UNKNOWN:
		{
			assert( false ); // You cant' intentionally create this type.
		} break;
		case egsm_node_t::NATIVE_EVENT:
		{
			// The editor will take care of populating this.
		} break;
		case egsm_node_t::ENTRY_POINT:
		{
			if( m_MachineProps.DefaultEntryPointNode.Len() == 0 )
			{
				m_MachineProps.DefaultEntryPointNode = Out.Id;
			}
			InsertBranchInNode( Out , "NEXT" );
		} break;
		case egsm_node_t::CALL:
		{
			InsertBranchInNode( Out , "NEXT" );
		} break;
	}

	return Out;
}

void EGSmFile2::DeleteNode( eg_cpstr Id )
{
	const eg_size_t IndexToDelete = GetNodeIndexById( Id );
	if( EGSM_INVALID_INDEX != IndexToDelete )
	{
		m_Nodes[IndexToDelete].bUsed = false;
	}

	// Delete all references to the state.
	for( egsmNodeScr& Node : m_Nodes )
	{
		if( Node.Id == Id )
		{
			if( Node.Type == egsm_node_t::ENTRY_POINT && Node.Id == m_MachineProps.DefaultEntryPointNode )
			{
				m_MachineProps.DefaultEntryPointNode = "";
			}
			Node.Id = "DELETED";
			Node.bUsed = false;
		}
		for( egsmBranchScr& Branch : Node.Branches )
		{
			if( Branch.ToNode == Id )
			{
				Branch.ToNode = "";
			}
		}
	}
}

void EGSmFile2::InsertBranchInNode( egsmNodeScr& Node , eg_cpstr OptionalId , egsm_var_t VarType /*= egsm_var_t::UNK*/ )
{
	egsmBranchScr NewBranch;
	if( OptionalId )
	{
		NewBranch.Id = OptionalId;
	}
	else
	{
		eg_string Prefix( "Value" );
		eg_uint Suffix = static_cast<eg_uint>(Node.Branches.Len());


		if( VarType == egsm_var_t::LOC_TEXT )
		{
			Prefix = m_MachineProps.NewLocKeyPrefix.String();
			Suffix = GenerateSuffix( egsm_suffix_t::LOC_TEXT );
		}

		NewBranch.Id = EGString_Format( "%s%u" , Prefix.String() , Suffix  );
	}

	NewBranch.ToNode = "";
	NewBranch.EnRef = "";
	NewBranch.bTerminal = false;

	Node.Branches.Append( NewBranch );
}

eg_string EGSmFile2::GetNextLocTextId()
{
	eg_string Prefix = m_MachineProps.NewLocKeyPrefix.String();
	eg_uint Suffix = GenerateSuffix( egsm_suffix_t::LOC_TEXT );
	return EGString_Format( "%s%u" , Prefix.String() , Suffix );
}

eg_string EGSmFile2::NodeTypeToString( egsm_node_t Type )
{
	switch( Type )
	{
#define DECL_NODE( _name_ , _color_ ) case egsm_node_t::_name_: return #_name_;
#include "EGSmNodes.items"
	}
	return "(unknown)";
}

eg_string EGSmFile2::NodeTypeToDisplayString( egsm_node_t Type )
{
	switch( Type )
	{
		case egsm_node_t::NATIVE_EVENT: return "Native Event";
		case egsm_node_t::ENTRY_POINT: return "Function";
		case egsm_node_t::CALL: return "Function Call";
		default: break;
	}
	return "(Unknown)";
}

egsm_node_t EGSmFile2::NodeStringToType( eg_cpstr Str )
{
	egsm_node_t Out = egsm_node_t::UNKNOWN;

	eg_string_small MyStr( Str );
	MyStr.ConvertToUpper();
	eg_string_crc CrcType( MyStr );

	switch_crc( CrcType )
	{
#define DECL_NODE( _name_ , _color_ ) case_crc(#_name_): Out = egsm_node_t::_name_; break;
#include "EGSmNodes.items"
	default: assert( false ); break;
	}

	return Out;
}

void EGSmFile2::RenameNode( eg_cpstr InOldName, eg_cpstr InNewName )
{
	// Need copies since the pointers passed in are probably pointers in this very thing.
	eg_string_small OldName = InOldName;
	eg_string_small NewName = InNewName;

	if( EGSM_INVALID_INDEX != GetNodeIndexById( NewName ) )
	{
		assert( false ); // Cant rename a node to something that is already used.
		return;
	}

	if( EGString_Equals( OldName , m_MachineProps.DefaultEntryPointNode ) )
	{
		m_MachineProps.DefaultEntryPointNode = InNewName;
	}

	for( egsmNodeScr& Node : m_Nodes )
	{
		if( Node.Id == OldName )
		{
			Node.Id = NewName;
		}

		for( egsmBranchScr& Branch : Node.Branches )
		{
			if( Branch.ToNode == OldName )
			{
				Branch.ToNode = NewName;
			}
		}
	}
}

eg_uint EGSmFile2::GenerateSuffix( egsm_suffix_t Type )
{
	eg_uint Out = 0;
	switch( Type )
	{
	case egsm_suffix_t::NODE_ID:
		Out = m_MachineProps.NewNodeIdSuffix;
		m_MachineProps.NewNodeIdSuffix++;
		break;
	case egsm_suffix_t::LOC_TEXT:
		// Dialog keys and choices share suffixes so that the prefix can be the same and they won't collide.
		Out = m_MachineProps.NewLocKeySuffix;
		m_MachineProps.NewLocKeySuffix++;
		break;
	}
	return Out;
}
