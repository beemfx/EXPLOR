// (c) 2014 Beem Media

//Additional classes needed:
#include "EGMake.h"
#include "EGXMLBase.h"
#include "EGToolsHelper.h"
#include "EGParse.h"
#include "EGFileData.h"
#include "EGWindowsAPI.h"
#include "EGWorldMapBase.h"
#include "EGExtProcess.h"
#include "EGLibExtern.h"
#include "EGCompress.h"

class EGMapCompiler
: private IXmlBase
{
private:
	struct egXmlGeoBlock
	{
		eg_uint nFirstTri;
		eg_uint nTriCount;
	};

private:
	eg_byte*  m_pMem;
	eg_size_t m_nMemSize;

	eg_byte*  m_PhysChunk;
	eg_uint   m_PhysChunkSize;

	eg_size_t m_NextStrOfs;
	eg_bool   m_bFoundTangent:1;
	eg_bool   m_bUseBase64Floats:1;

	EGWorldMapBase::egHeader m_XmlH; //For loading with XML.
	#define MAP_DATA_SECTION( _type_ , _var_ ) EGArray<_type_> m_##_var_##Array;
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION

	EGArray<egXmlGeoBlock> m_XmlGeoBlocksArray;
	EGArray<eg_vec4>       m_XmlGeoVertexArray;

public:
	EGMapCompiler()
	: m_pMem(nullptr)
	, m_nMemSize(0)
	, m_PhysChunk(nullptr)
	, m_PhysChunkSize(0)
	, m_NextStrOfs(0)
	, m_bFoundTangent(false)
	, m_bUseBase64Floats(false)
	{

	}

	eg_bool LoadFromXML(eg_cpstr szFilename);

	void Unload()
	{
		DeallocateMemory();
	}

private:
	void DeallocateMemory()
	{
		zero( &m_XmlH );
		#define MAP_DATA_SECTION( _type_ , _var_ ) m_##_var_##Array.Clear();
		#include "EGMapDataSections.inc"
		#undef MAP_DATA_SECTION	

		if( m_PhysChunk )
		{
			EG_SafeDeleteArray( m_PhysChunk );
			m_PhysChunk = nullptr;
			m_PhysChunkSize = 0;
		}

		if( m_pMem )
		{
			EG_SafeDeleteArray( m_pMem );
			m_pMem = nullptr;
			m_nMemSize = 0;
		}
	}

///////////////////
/// XML Loading ///
///////////////////
	eg_cpstr XMLObjName()const
	{
		return ("\"Emergence Map\"");
	}
	virtual void OnTag(const eg_string_base& Tag, const EGXmlAttrGetter& atts);
	void OnTag_geo_shape( const EGXmlAttrGetter& Getter );
	void OnTag_graph( const EGXmlAttrGetter& Getter );
	void OnTag_edge( const EGXmlAttrGetter& Getter );
	void OnTag_vertex_raster( const EGXmlAttrGetter& Getter );
	void OnTag_vertex_graph( const EGXmlAttrGetter& Getter );
	void OnTag_vertex_geometry( const EGXmlAttrGetter& Getter );
	void OnTag_light( const EGXmlAttrGetter& Getter );
	void OnTag_tag( const EGXmlAttrGetter& Getter );
	void OnTag_region( const EGXmlAttrGetter& Getter );

private:
	eg_uint AddString( const eg_string_base& s );
	void BuildData();

	eg_uint GetGeoBlockCount()const{ return static_cast<eg_uint>(m_XmlGeoBlocksArray.Len()); }
	const egXmlGeoBlock* GetGeoBlock(eg_uint nBlock)const{return &m_XmlGeoBlocksArray[nBlock-1];}
	const eg_vec4* GetGeoBlockTris(eg_uint nBlock)const{return &m_XmlGeoVertexArray[m_XmlGeoBlocksArray[nBlock-1].nFirstTri*3];}
public:
	const eg_byte* GetDataAndSize(eg_size_t* OutSize)const;
};

eg_bool EGMapCompiler::LoadFromXML(eg_cpstr szFilename)
{
	Unload();

	{
		EGFileData fileScript( eg_file_data_init_t::HasOwnMemory );
		EGLibExtern_LoadNowTo( szFilename, fileScript );

		if( fileScript.GetSize() > 0 )
		{
			EGArray<eg_byte> FileData;
			FileData.Append( fileScript.GetDataAs<eg_byte>(), fileScript.GetSize() );
			if( FileData.Len() == fileScript.GetSize() )
			{
				eg_bool bCanRead = true;

				if( EGCompress_IsCompressedData( FileData ) )
				{
					bCanRead = EGCompress_DecompressData( FileData , FileData );
				}

				if( bCanRead )
				{
					XMLLoad( FileData.GetArray() , FileData.Len() , szFilename );
				}
				else
				{
					EGLogf( eg_log_t::Error , __FUNCTION__ ": Failed to decompress %s." , szFilename );
				}
			}
			else
			{
				EGLogf( eg_log_t::Error , __FUNCTION__ ": Out of memory cannot load %s." , szFilename );
			}
		}
		else
		{
			EGLogf( eg_log_t::Error, __FUNCTION__ ": No such file \"%s\".", szFilename );
		}
	}

	if(m_XmlH.nVersion != EGWorldMapBase::MAP_VERSION)return false;

	// EX-Release: Note originally at this point a PhysX chunk was formed, this
	// simplified version of the engine does not store any physics data for
	// maps.
	// FormPhysXChunk();

	//Set the nFrom component for all the portals
	for(eg_uint i=0; i<m_XmlH.nRegionsCount; i++)
	{
		for(eg_uint nPort=0; nPort < m_RegionsArray[i].nPortalCount; nPort++)
		{
			m_PortalsArray[nPort + m_RegionsArray[i].nFirstPortal-1].nFrom = i+1;
		}
	}

	//And if we didn't find tangents, compute them as well.
	if(!m_bFoundTangent && m_XmlH.nVertexesCount > 0 )
	{
		EGLogf( eg_log_t::Warning , __FUNCTION__ " Warning: No tangents found, generating them..." );
		//Create a temp array to make our fake triangles:
		eg_uint nNumTris = static_cast<eg_uint>(m_VertexesArray.Len()/3);

		EGArray<egv_index> aI;
		aI.Resize( nNumTris*3 );
		for(eg_uint i=0; i<m_VertexesArray.Len(); i++)
		{
			aI[i] = static_cast<egv_index>(i);
		}

		::EGVertex_ComputeTangents(&m_VertexesArray[0], aI.GetArray() , static_cast<eg_uint>(m_VertexesArray.Len()) , nNumTris);
	}

	//Edge is at to m_pGraphVerts[SGraph::nFirstVert + anEdge[Edge]];
	//We just want it to be m_pGraphVerst[anEdge[Edge]];
	//Fix the graphs so that edges are absolute.
	for(eg_uint g=0; g<m_GraphsArray.Len(); g++)
	{
		for(eg_uint v=0; v<m_GraphsArray[g].nNumVerts; v++)
		{
			eg_uint IdVertex = m_GraphsArray[g].nFirstVert+v+1;
			EGWorldMapBase::egGraphVertex* pVert = &m_GraphVertsArray[IdVertex-1];
			for(eg_uint e=0; e<pVert->nNumEdges; e++)
			{
				pVert->anEdge[e] += m_GraphsArray[g].nFirstVert;
			}
		}
	}

	BuildData();

	return true;
}

void EGMapCompiler::OnTag( const eg_string_base& Tag , const EGXmlAttrGetter& Getter )
{
	if(m_XmlH.nVersion!=EGWorldMapBase::MAP_VERSION && "emap" != Tag)
		return;

	if( "graph" == Tag )OnTag_graph(Getter);
	else if( "edge" == Tag)OnTag_edge(Getter);
	else if( "emap" == Tag)
	{
		zero( &m_XmlH );
		m_XmlH.nID = EGWorldMapBase::MAP_ID;
		m_bFoundTangent = false;
		m_NextStrOfs=0;

		m_XmlGeoBlocksArray.Clear();
		m_XmlGeoVertexArray.Clear();
		#define MAP_DATA_SECTION( _type_ , _var_ ) m_##_var_##Array.Clear();
		#include "EGMapDataSections.inc"
		#undef MAP_DATA_SECTION
		m_HArray.Resize(1);

		eg_string_big strMapName = Getter.GetString( "name" );
		eg_string_big strFormat  = Getter.GetString( "format" );
	
		m_XmlH.nVersion        = Getter.GetUInt( "version" );

		if(m_XmlH.nVersion != EGWorldMapBase::MAP_VERSION)
		{
			EGLogf( eg_log_t::Warning , ("EGMap::LoadFromXML Warning: It may have been a while since this map was exported, not all data may be present."));
			m_XmlH.nVersion = EGWorldMapBase::MAP_VERSION;
		}

		if(EGWorldMapBase::MAP_VERSION == m_XmlH.nVersion)
		{
			// It's good...
		}
	
		m_bUseBase64Floats = strFormat.EqualsI("base64");
		
		m_XmlH.nStrOfsName = AddString(strMapName);
	}
	else if ("portal" == Tag )
	{
		EGWorldMapBase::egPortal ptl;
		zero(&ptl);

		ptl.nID = Getter.GetUInt( "id" );
		ptl.nTo = Getter.GetUInt( "to" );
		ptl.sphPortal.Sphere.SetupSphere( 1.0f );
		eg_string_big sTerminal  = Getter.GetString( "terminal" );
		eg_string_big sAlwaysVis = Getter.GetString( "always_visible" );
		eg_vec3 SpherePos( CT_Clear );
		Getter.GetVec( "pos" , reinterpret_cast<eg_real*>(&SpherePos) , 3 , m_bUseBase64Floats );
		ptl.sphPortal.Transform = eg_transform::BuildTranslation( SpherePos );
		Getter.GetVec( "radius" , reinterpret_cast<eg_real*>(&ptl.sphPortal.Sphere.Radius) , 1 , m_bUseBase64Floats );
		

		if(!sTerminal.CompareI(("true")))
		{
			ptl.bTerminal = true;
		}

		if(!sAlwaysVis.CompareI(("true")))
		{
			ptl.bAlwaysVisible = true;
		}

		//Now insert the portal:
		if( ptl.nID > 0 )
		{
			m_PortalsArray.ExtendToAtLeast( ptl.nID );
			m_PortalsArray[ptl.nID-1] = ptl;
		}
	}
	else if( "bounds" == Tag )
	{
		Getter.GetVec( "min" , reinterpret_cast<eg_real*>(&m_XmlH.aabbMapBounds.Min) , 3 , m_bUseBase64Floats );
		m_XmlH.aabbMapBounds.Min.w = 1.0f;
		Getter.GetVec( "max" , reinterpret_cast<eg_real*>(&m_XmlH.aabbMapBounds.Max) , 3 , m_bUseBase64Floats );
		m_XmlH.aabbMapBounds.Max.w = 1.0f;
	}
	else if( "material" == Tag )
	{
		eg_uint nID=Getter.GetUInt( "id" );

		EGMaterialDef MtrlDef;
		MtrlDef.SetFromTag(Getter);
	
		if( nID > 0 )
		{
			m_MtrlsArray.ExtendToAtLeast( nID );
			m_MtrlsArray[nID-1].Def  = MtrlDef;
			m_MtrlsArray[nID-1].Mtrl = EGV_MATERIAL_NULL;
		}
	}
	else if( "segment" == Tag )
	{
		eg_uint nID=0;
		EGWorldMapBase::egRasterSegment seg;
	
		nID           = Getter.GetUInt( "id" );
		seg.nFirst    = Getter.GetUInt( "first_vertex" );
		seg.nTriCount = Getter.GetUInt( "triangles" );
		seg.nMtrlRef  = Getter.GetUInt( "material" );
	
		//Adjust all references:
		seg.nFirst--;
		//seg.nMtrlRef--;
		
		if( nID > 0 )
		{
			m_SegmentsArray.ExtendToAtLeast( nID );
			m_SegmentsArray[nID-1] = seg;
		}
	}
	else if( "region" == Tag )OnTag_region(Getter);
	else if( "geo_shape" == Tag )OnTag_geo_shape(Getter);
	else if( "geo_triangle" == Tag )
	{
		eg_uint nID=Getter.GetUInt( "id" );
		eg_vec4 v1(0, 0, 0, 1), v2(0, 0, 0, 1), v3(0, 0, 0, 1);
		Getter.GetVec( "v1" , reinterpret_cast<eg_real*>(&v1) , 3 , m_bUseBase64Floats );
		v1.w = 1.0f;
		Getter.GetVec( "v2" , reinterpret_cast<eg_real*>(&v2) , 3 , m_bUseBase64Floats );
		v2.w = 1.0f;
		Getter.GetVec( "v3" , reinterpret_cast<eg_real*>(&v3) , 3 , m_bUseBase64Floats );
		v3.w = 1.0f;
	
		if( nID > 0 )
		{
			m_XmlGeoVertexArray.ExtendToAtLeast( (nID-1)*3+2 + 1 );
			m_XmlGeoVertexArray[(nID-1)*3+0]=v1;
			m_XmlGeoVertexArray[(nID-1)*3+1]=v2;
			m_XmlGeoVertexArray[(nID-1)*3+2]=v3;
		}
	}
	else if ( "vertex" == Tag )
	{
		const eg_string_crc tagParent = eg_string_crc(GetXmlTagUp(1));
		//There is more than one type of vertex:

		if(eg_crc("graph") == tagParent)
			OnTag_vertex_graph(Getter);
		else if(eg_crc("geometry") == tagParent)
			OnTag_vertex_geometry(Getter);
		else
			OnTag_vertex_raster(Getter);
	}
	else if( "light" == Tag )OnTag_light(Getter);
	else if( "tag" == Tag )OnTag_tag(Getter);
	else if( "geo_brushes" == Tag)
	{
		// Don't need to do anything.
	}
	else if( "aabb" == Tag )
	{
		if( "geo_brushes" == this->GetXmlTagUp(1) )
		{
			eg_aabb NewAabb;
			Getter.GetVec( "min" , reinterpret_cast<eg_real*>(&NewAabb.Min) , 3 , m_bUseBase64Floats);
			NewAabb.Min.w = 1.f;
			Getter.GetVec( "max" , reinterpret_cast<eg_real*>(&NewAabb.Max) , 3 , m_bUseBase64Floats);
			NewAabb.Max.w = 1.f;

			EGWorldMapBase::egGeoAlignedBox NewBox;
			NewBox.Bounds = NewAabb;
			m_GeoAlignedBoxesArray.Append( NewBox );
		}
	}
}

void EGMapCompiler::OnTag_region( const EGXmlAttrGetter& Getter )
{
	eg_uint nID=0;
	EGWorldMapBase::egRegion region;
	zero(&region);
	region.clrAmb.a = 1.0f;		

	nID                    = Getter.GetUInt( "id" );
	region.nFirstRasterSeg = Getter.GetUInt( "first_segment" );
	region.nRasterSegCount = Getter.GetUInt( "segments" );
	region.nFirstLight     = Getter.GetUInt( "first_light" );
	region.nLightCount     = Getter.GetUInt( "lights" );
	region.nFirstPortal    = Getter.GetUInt( "first_portal" );
	region.nPortalCount    = Getter.GetUInt( "portals" );
	region.nFirstGeoBlock  = Getter.GetUInt( "first_geo_block" );
	region.nGeoBlockCount  = Getter.GetUInt( "geo_blocks" );
	region.nStrOfsName     = AddString( Getter.GetString( "name" ) );

	Getter.GetVec( "bounds_min" , reinterpret_cast<eg_real*>(&region.aabb.Min) , 3 , m_bUseBase64Floats );
	region.aabb.Min.w = 1.0f;
	Getter.GetVec( "bounds_max" , reinterpret_cast<eg_real*>(&region.aabb.Max) , 3 , m_bUseBase64Floats );
	region.aabb.Max.w = 1.0f;
	Getter.GetVec( "ambient" , reinterpret_cast<eg_real*>(&region.clrAmb) , 4 , m_bUseBase64Floats );
		
	//Make adjustments.
	region.nFirstRasterSeg--;
	#if 0
	region.nFirstVertex--;
	#endif
	region.nFirstGeoBlock--;
		
	if( nID > 0 )
	{
		m_RegionsArray.ExtendToAtLeast( nID );
		m_RegionsArray[nID-1] = region;
		/*
		EGLogf(("Region %s: %u %u."),
				m_pRegions[nID-1].strName.String(), 
				m_pRegions[nID-1].nFirstRasterSeg,
				m_pRegions[nID-1].nRasterSegCount);
		*/
	}
}

void EGMapCompiler::OnTag_tag( const EGXmlAttrGetter& Getter )
{
	eg_vec4 vPos;
	eg_uint nID=Getter.GetUInt( "id" );
	eg_string_big strType = Getter.GetString( "type" );
	Getter.GetVec( "pos" , reinterpret_cast<eg_real*>(&vPos) , 3 , m_bUseBase64Floats );
	vPos.w = 1.0f;

	eg_string_big strAtts;

	const eg_uint NumAtts = Getter.GetNumAttributes();

	for( eg_uint i=0 ; i< NumAtts ; i++ )
	{
		eg_string_big att; 
		eg_string_big val;
		Getter.GetAttributePair( i , att , val );

		strAtts.Append(EGString_Format( ("%s=\"%s\" "), att.String(), val.String() ) );
	}

	//For now we just see if this is the spawn location:
	if(strType.EqualsI(("spawn")))
	{
		m_XmlH.vSpawn = vPos;
	}

	if( nID > 0 )
	{
		EGWorldMapBase::egTag Tag;
		Tag.nStrOfsType = AddString(strType);
		Tag.nStrOfsAtts = AddString(strAtts);
		Tag.Pos = vPos;

		m_TagsArray.ExtendToAtLeast( nID );
		m_TagsArray[nID-1] = Tag;
	}
}

void EGMapCompiler::OnTag_light( const EGXmlAttrGetter& Getter )
{
	EGWorldMapBase::egLight Light;
	eg_uint nID=Getter.GetUInt( "id" );
	Getter.GetVec( "pos" , reinterpret_cast<eg_real*>(&Light.Pos) , 3 , m_bUseBase64Floats );
	Light.Pos.w = 1.0f;
	Getter.GetVec( "color" , reinterpret_cast<eg_real*>(&Light.Color) , 4 , m_bUseBase64Floats );
	Getter.GetVec( "range" , reinterpret_cast<eg_real*>(&Light.fRangeSq) , 1 , m_bUseBase64Floats );
	//We actually use the square value of lights to speed up computations.
	Light.fRangeSq = Light.fRangeSq*Light.fRangeSq;

	if( nID > 0 )
	{
		m_LightsArray.ExtendToAtLeast( nID );
		m_LightsArray[nID-1] = Light;
	}

}


void EGMapCompiler::OnTag_vertex_geometry( const EGXmlAttrGetter& Getter )
{
	unused( Getter );
}

void EGMapCompiler::OnTag_graph( const EGXmlAttrGetter& Getter )
{
	eg_string_big strName      = Getter.GetString( "name" );
	eg_uint   nID          = Getter.GetUInt( "id" );
	eg_string_big strType      = Getter.GetString( "type" );
	eg_uint   nVertexCount = Getter.GetUInt( "vertexes" );

	EGWorldMapBase::egGraph GraphNew;
	GraphNew.nFirstVert = static_cast<eg_uint>(m_GraphVertsArray.Len());
	GraphNew.nNumVerts = 0;
	GraphNew.nStrOfsName = this->AddString( strName );
	GraphNew.Type = eg_string_crc(strType);

	m_GraphsArray.Append( GraphNew );
}

void EGMapCompiler::OnTag_vertex_graph( const EGXmlAttrGetter& Getter )
{
	assert(m_GraphsArray.Len() > 0);//How can we have a vertex without a graph?
	if(m_GraphsArray.Len() <= 0 || GetXmlTagUp(1) != "graph")
	{
		EGLogf(eg_log_t::Error , __FUNCTION__": Error: Can't have a vertex without a graph, badly formed XML file.");
		return;
	}
	//This is just a matter of adding the vertex to the current graph.
	eg_uint nID = Getter.GetUInt( "id" );
	eg_vec4 vPos(0,0,0,1);
	Getter.GetVec( "pos" , reinterpret_cast<eg_real*>(&vPos) , 3 , m_bUseBase64Floats );
	vPos.w = 1.0f;

	if(nID <= 0)
	{
		EGLogf(eg_log_t::Error, __FUNCTION__": Error: Graph vertex must have positive id (>=1).");
		return;
	}

	//NEW:
	//Current Graph:
	EGWorldMapBase::egGraph& G = m_GraphsArray[m_GraphsArray.Len()-1];
	G.nNumVerts++;

	EGWorldMapBase::egGraphVertex V;
	zero(&V);
	V.nNumEdges=0;
	V.Pos = vPos;
	//Set offset relative to first vertex in graph:
	m_GraphVertsArray.Resize(G.nFirstVert+(nID));
	m_GraphVertsArray[G.nFirstVert+(nID-1)] = V;
}

void EGMapCompiler::OnTag_edge( const EGXmlAttrGetter& Getter )
{
	assert(m_GraphVertsArray.Len() > 0);//How can we have a vertex without a graph?
	if(m_GraphVertsArray.Len() <= 0 || GetXmlTagUp(1) != "vertex")
	{
		EGLogf(eg_log_t::Error, __FUNCTION__": Error: Can't have an edge without a vertex, badly formed XML file.");
		return;
	}

	eg_uint nTo=Getter.GetUInt( "to" );

	if(nTo <= 0)
	{
		EGLogf(eg_log_t::Error, __FUNCTION__": Error: Graph edge must point to valid vertex (>=1).");
		return;
	}

	//NEW:
	EGWorldMapBase::egGraphVertex& V = m_GraphVertsArray[m_GraphVertsArray.Len()-1];
	V.anEdge[V.nNumEdges] = nTo;
	V.nNumEdges++;
}

void EGMapCompiler::OnTag_geo_shape( const EGXmlAttrGetter& Getter )
{
	eg_uint nID=0;
	egXmlGeoBlock XmlBlock;
	EGWorldMapBase::egGeoBlock BinBlock;
	zero(&XmlBlock);
	zero(&BinBlock);
		
	nID                = Getter.GetUInt( "id" );
	XmlBlock.nFirstTri = Getter.GetUInt( "first_triangle" );
	XmlBlock.nTriCount = Getter.GetUInt( "triangles" );

	Getter.GetVec( "bounds_min" , reinterpret_cast<eg_real*>(&BinBlock.aabb.Min) , 3 , m_bUseBase64Floats );
	BinBlock.aabb.Min.w = 1.0f;
	Getter.GetVec( "bounds_max" , reinterpret_cast<eg_real*>(&BinBlock.aabb.Max) , 3 , m_bUseBase64Floats );
	BinBlock.aabb.Max.w = 1.0f;

	XmlBlock.nFirstTri--;
		
	if( nID > 0 )
	{
		m_XmlGeoBlocksArray.ExtendToAtLeast( nID );
		m_GeoBlocksArray.ExtendToAtLeast( nID );
		m_XmlGeoBlocksArray[nID-1] = XmlBlock;
		m_GeoBlocksArray[nID-1] = BinBlock;
	}
}


void EGMapCompiler::OnTag_vertex_raster( const EGXmlAttrGetter& Getter )
{
	egv_vert_mesh v;
	zero(&v);

	eg_bool FoundTangent = false;
	eg_uint nID = ::EGVertex_FromXmlTag(&v, m_bUseBase64Floats, Getter , &FoundTangent);
	m_bFoundTangent = FoundTangent;

	if( nID > 0 )
	{
		m_VertexesArray.ExtendToAtLeast( nID );
		m_VertexesArray[nID-1] = v;
	}
}

eg_uint EGMapCompiler::AddString( const eg_string_base& s )
{
	for( eg_size_t i=0; i<s.Len(); i++ )
	{
		m_StringsArray.Append( s[i] );
	}
	m_StringsArray.Append( '\0' );

	eg_size_t nOfs = m_NextStrOfs;
	m_NextStrOfs = m_StringsArray.Len();
	return static_cast<eg_uint>(nOfs);
}

void EGMapCompiler::BuildData()
{
	m_HArray.ExtendToAtLeast( 1 );
	m_HArray[0] = m_XmlH; // Make sure the header exists.

	// Set counts...
	#define MAP_DATA_SECTION( _type_ , _var_ ) m_XmlH.n##_var_##Count = static_cast<eg_uint>(m_##_var_##Array.Len());
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION

	// Set offsets...
	eg_size_t DataSize = 0;
	#define MAP_DATA_SECTION( _type_ , _var_ ) m_XmlH.Ofs##_var_## = static_cast<eg_uint>(DataSize); DataSize += EG_AlignUp( m_XmlH.n##_var_##Count * sizeof( _type_ ) );
	#include "EGMapDataSections.inc"
	#undef MAP_DATA_SECTION

	// Copy data to a chunk...
	m_pMem = new eg_byte[DataSize];
	if( nullptr != m_pMem )
	{
		m_HArray[0] = m_XmlH; // Set the header again with the correct information.
		#define MAP_DATA_SECTION( _type_ , _var_ ) if( m_XmlH.n##_var_##Count > 0 ){ EGMem_Copy( &m_pMem[m_XmlH.Ofs##_var_##] , &m_##_var_##Array[0] , m_XmlH.n##_var_##Count*sizeof(_type_)); }
		#include "EGMapDataSections.inc"
		#undef MAP_DATA_SECTION
		m_nMemSize = DataSize;
	}
}

const eg_byte* EGMapCompiler::GetDataAndSize(eg_size_t* OutSize)const
{
	*OutSize = m_nMemSize;
	return m_pMem;
}

static eg_bool EGMakeRes_emap( eg_cpstr strIn , eg_cpstr strOutEmap , eg_cpstr strOutPhysx )
{
	EGMapCompiler Map;

	if(!Map.LoadFromXML(strIn))
	{
		return false;
	}

	eg_size_t nSize = 0;
	const eg_byte* pData = Map.GetDataAndSize(&nSize);
	eg_uint nPhysXSize = static_cast<eg_uint>(EGString_StrLen("NODATA")) + 1;
	const eg_byte pPhysXData[] = { 'N' , 'O' , 'D' , 'A', 'T', 'A',  '\n' };

	eg_bool bSuc = ::EGMake_WriteOutputFile( strOutEmap , pData , nSize);
	bSuc = bSuc && ::EGMake_WriteOutputFile( strOutPhysx , pPhysXData , nPhysXSize );
	Map.Unload();

	return bSuc;
}

eg_bool EGMakeRes_gcx()
{
#if 1
	EGLogf( eg_log_t::Warning , "GCX Files are no longer supported for the build process, they must be imported as raw assets." );
	return false;
#else
	eg_string_big TmpDir = true ? EGMake_GetEGOUT() : EGMake_GetInputPath( FINFO_DIR );
	TmpDir.Append( "_tmp" );
	CreateDirectoryA( TmpDir, nullptr );
	TmpDir.Append( "\\" );
	TmpDir.Append( EGMake_GetInputPath( FINFO_SHORT_NOEXT ) );
	CreateDirectoryA( TmpDir , nullptr );

	EGArray<eg_string_big> EmapsToCompile;
	EGArray<eg_string_big> TgasToCompile;
	
	auto WriteLine  = []( EGFileData& File , auto ... Format ) -> void
	{
		File.WriteStr8( EGString_Format( Format... ) );
	};

	auto DoGcxToEmaps = [&TmpDir,&WriteLine]() -> eg_bool
	{
		eg_bool bSucc = true;

		eg_string_big FilenameEBuild = TmpDir;
		FilenameEBuild.Append( "\\build_maps.ebuild" );

		EGFileData File;

		WriteLine( File , "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
		WriteLine( File , "<ebuild>\r\n" );
		WriteLine( File , "<command>egmake2_x64.exe MAP -gcx -in \"%s\" -emap \"$(IN_DIR)_unused_.emap\"</command>\r\n" , EGMake_GetInputPath(FINFO_FULL_FILE) );
		WriteLine( File , "</ebuild>\r\n" );

		bSucc = bSucc && EGMake_WriteOutputFile( FilenameEBuild, File.GetMem(), File.GetSize() );

		eg_string_big BuildCmd = EGString_Format( "egmake2_x64.exe RESOURCE -in \"%s\" -out \"%s\"" , FilenameEBuild.String() , FilenameEBuild.String() );
		bSucc = bSucc && EGExtProcess_Run( BuildCmd , nullptr );

		//::DeleteFile( FilenameEBuild );

		return bSucc;
	};

	auto DoCompileMaps = [&TmpDir,&WriteLine,&EmapsToCompile,&TgasToCompile]() -> eg_bool
	{
		eg_bool bSucc = true;

		eg_string_big FilenameEBuild = TmpDir;
		FilenameEBuild.Append( "\\compile_maps.ebuild" );

		EGFileData File;

		// Find all the emaps and tgas that were made
		WIN32_FIND_DATAA FindData;
		zero( &FindData );
		eg_string_big FindStr = TmpDir;
		FindStr.Append( "\\*");
		HANDLE hFind = FindFirstFileA( FindStr , &FindData );
		if( INVALID_HANDLE_VALUE != hFind )
		{
			do 
			{

				if( EGString_Contains( FindData.cFileName , ".tga" ) )
				{
					TgasToCompile.Append( FindData.cFileName );
				}

				if( EGString_Contains( FindData.cFileName, ".emap" ) )
				{
					EmapsToCompile.Append( FindData.cFileName );
				}

			} while ( FindNextFileA( hFind , &FindData ) );

			FindClose( hFind );
		}
		
		WriteLine( File , "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
		WriteLine( File , "<ebuild>\r\n" );

		for( const eg_string_base& Filename : EmapsToCompile )
		{
			WriteLine( File , "<command>egmake2_x64.exe RESOURCE -in \"$(IN_DIR)%s\" -out \"$(OUT_DIR)%s\"</command>\r\n" , Filename.String() , Filename.String() );
		}

		for( const eg_string_base& Filename : TgasToCompile )
		{
			WriteLine( File, "<command>egmake2_x64.exe RESOURCE -in \"$(IN_DIR)%s\" -out \"$(OUT_DIR)%s\"</command>\r\n", Filename.String() , Filename.String() );
		}

		WriteLine( File , "</ebuild>\r\n" );
		
		bSucc = bSucc && EGMake_WriteOutputFile( FilenameEBuild , File.GetMem() , File.GetSize() );

		eg_string_big BuildCmd = EGString_Format( "egmake2_x64.exe RESOURCE -in \"%s\" -out \"%s\"" , FilenameEBuild.String() , EGMake_GetOutputPath() );
		bSucc = bSucc && EGExtProcess_Run( BuildCmd , nullptr );
		
		//::DeleteFile( FilenameEBuild );

		return bSucc;
	};

	auto WriteEsave = [&WriteLine,&EmapsToCompile,&TmpDir]() -> eg_bool
	{
		eg_bool bSucc = true;

		for( const eg_string_base& Filename : EmapsToCompile )
		{	
			// Create the file
			EGFileData File;

			eg_string_big FilenameNoExt = Filename;
			FilenameNoExt.ClampEnd( 5 );
		
			WriteLine( File , "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n");
			WriteLine( File , "<esave time=\"0\" flags=\"SPAWNMAPENTS\">\r\n" );
			WriteLine( File , "<map>%s</map>\r\n" , FilenameNoExt.String() );
			WriteLine( File , "</esave>\r\n" ); 

			eg_string_big FilenameOutEsave = EGMake_GetOutputPath(FINFO_DIR);
			FilenameOutEsave.Append( FilenameNoExt );
			FilenameOutEsave.Append( ".esave" );
			bSucc = bSucc && EGMake_WriteOutputFile(FilenameOutEsave, File.GetMem(), File.GetSize());
		}

		return bSucc;
	};

	eg_bool bSucc = true;

	bSucc = bSucc && DoGcxToEmaps();
	bSucc = bSucc && DoCompileMaps();
	bSucc = bSucc && WriteEsave();

	return bSucc;
#endif
}

int EGMap_main( int argc , char* argv[] )
{
	eg_cpstr strIn  = nullptr;
	eg_cpstr strEmapOut = nullptr;
	eg_cpstr strPhysxOut = nullptr;
	eg_bool  IsGcx = false;
	eg_bool  bShouldCompress = false;

	enum NEXT_T
	{
		NEXT_PARM,
		NEXT_IN,
		NEXT_EMAPOUT,
		NEXT_PHYSXOUT,
	};

	NEXT_T Next = NEXT_PARM;

	for(int i = 0; i < argc; i++)
	{
		if(NEXT_IN == Next)
		{
			strIn = argv[i];
			Next = NEXT_PARM;
		}
		else if(NEXT_EMAPOUT == Next)
		{
			strEmapOut = argv[i];
			Next = NEXT_PARM;
		}
		else if(NEXT_PHYSXOUT == Next)
		{
			strPhysxOut = argv[i];
			Next = NEXT_PARM;
		}
		else
		{
			if(eg_string_big(argv[i]).EqualsI(("-in")))
			{
				Next = NEXT_IN;
			}
			else if(eg_string_big(argv[i]).EqualsI( ("-emap") ) )
			{
				Next = NEXT_EMAPOUT;
			}
			else if(eg_string_big(argv[i]).EqualsI( ("-physx") ) )
			{
				Next = NEXT_PHYSXOUT;
			}
			else if(eg_string_big(argv[i]).EqualsI( "-gcx" ) )
			{
				IsGcx = true;
			}
			else if(eg_string_big(argv[i]).EqualsI( "-compress") )
			{
				bShouldCompress = true;
			}
		}
	}

	EGRMake_InitPaths( strIn , strEmapOut );

	if((nullptr == strIn) || (nullptr == strEmapOut) || (nullptr == strPhysxOut && !IsGcx) )
	{
		EGLogf(eg_log_t::Error, ("egmap Error: Invalid input: -in \"%s\" -emap \"%s\" -physx \"%s\"") , strIn , strEmapOut , strPhysxOut );
		return 0;
	}

	EGLogf(eg_log_t::General, ("egmap: \"%s\" -> \"%s\" , \"%s\" ") , strIn , strEmapOut , strPhysxOut );

	eg_bool bSuccess = false;
	
	if( IsGcx )
	{
		bSuccess = EGMake_gcx( strIn , strEmapOut , bShouldCompress );
	}
	else
	{
		bSuccess = EGMakeRes_emap( strIn , strEmapOut , strPhysxOut );
		#if 0//defined(__DEBUG__)
		eg_s_string_big8 SecondPhysX = strPhysxOut;
		SecondPhysX.Append( ".2" );
		EGMakeRes_emap( strIn , strEmapOut , *SecondPhysX );
		#endif
	}
	if(!bSuccess)
	{
		EGLogf(eg_log_t::Error , ("egmap Error: Failed to compile \"%s\"."), strIn);
	}
	return bSuccess ? 0 : -1;
}
