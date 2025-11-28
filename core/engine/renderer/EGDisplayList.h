// (c) 2017 Beem Media

#pragma once

#include "EGRendererTypes.h"
#include "EGList.h"
#include "EGStaticAlloc.h"

EG_DECLARE_FLAG( DL_F_PROC_MOTIONBLUR , 0 );
EG_DECLARE_FLAG( DL_F_PROC_FXAA       , 1 );
EG_DECLARE_FLAG( DL_F_PROC_TOSCREEN   , 2 );
EG_DECLARE_FLAG( DL_F_SCENE_DRAWTOZ   , 4 );
EG_DECLARE_FLAG( DL_F_PROC_STOREDEPTH , 5 );
EG_DECLARE_FLAG( DL_F_SCENE_REFLECT   , 6 );
EG_DECLARE_FLAG( DL_F_SCENE_SHADOWS   , 7 );

enum DISPLAY_LIST_CMD
{	
	DLFUNC_DrawRawTris ,
	DLFUNC_SetBoneMats ,
	
	#define DL_FUNCTION_0_PARM( _function_ ) DLFUNC_##_function_ ,
	#define DL_FUNCTION_1_PARM( _function_ , _parmtype0_ , _parmname0_ ) DLFUNC_##_function_ ,
	#define DL_FUNCTION_2_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ ) DLFUNC_##_function_ ,
	#define DL_FUNCTION_3_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ ) DLFUNC_##_function_ ,
	#define DL_FUNCTION_5_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ , _parmtype3_ , _parmname3_ , _parmtype4_ , _parmname4_ ) DLFUNC_##_function_ ,
	#include "EGDisplayListFunctions.items"
	#undef DL_FUNCTION_0_PARM
	#undef DL_FUNCTION_1_PARM
	#undef DL_FUNCTION_2_PARM
	#undef DL_FUNCTION_3_PARM
	#undef DL_FUNCTION_5_PARM
};

union egDisplayListCmdData
{
	struct { egv_vert_simple* pVerts; eg_uint nNumTris; } DrawRawTris;
	struct { eg_transform* pTrans; eg_uint nCount; } SetBoneMats;

	#define DL_FUNCTION_0_PARM( _function_ )
	#define DL_FUNCTION_1_PARM( _function_ , _parmtype0_ , _parmname0_ ) struct { _parmtype0_ _parmname0_; } _function_;
	#define DL_FUNCTION_2_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ ) struct { _parmtype0_ _parmname0_; _parmtype1_ _parmname1_; } _function_;
	#define DL_FUNCTION_3_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ ) struct { _parmtype0_ _parmname0_; _parmtype1_ _parmname1_; _parmtype2_ _parmname2_; } _function_;
	#define DL_FUNCTION_5_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ , _parmtype3_ , _parmname3_ , _parmtype4_ , _parmname4_ ) struct { _parmtype0_ _parmname0_; _parmtype1_ _parmname1_; _parmtype2_ _parmname2_; _parmtype3_ _parmname3_; _parmtype4_ _parmname4_; } _function_;
	#include "EGDisplayListFunctions.items"
	#undef DL_FUNCTION_0_PARM
	#undef DL_FUNCTION_1_PARM
	#undef DL_FUNCTION_2_PARM
	#undef DL_FUNCTION_3_PARM
	#undef DL_FUNCTION_5_PARM
};

class EGDisplayList
{
public:

	struct egCmd
	{
		DISPLAY_LIST_CMD     Cmd;
		egDisplayListCmdData Data;
	};

	struct egCmdListItem: public IListable
	{
		DISPLAY_LIST_CMD      Cmd:8;
		eg_uint               DataSize:24;
		egDisplayListCmdData* Data;
	};

public:

	EGList<egCmdListItem> List;
	eg_mat m_LastWorldTF = eg_mat::I;
	eg_mat m_LastViewTF = eg_mat::I;
	eg_mat m_LastProjTF = eg_mat::I;

public:

	EGDisplayList(): List(EGList<egCmdListItem>::DEFAULT_ID), m_AssetState(0), m_bCannotbeDropped(false){ }
	void InitDisplayList( void* Mem , eg_size_t MemSize , eg_uint AssetState );
	void DeinitDisplayList( void );
	eg_uint GetAssetState()const{ return m_AssetState; }
	eg_bool IsOutOfMem()const{ return m_OutOfMem; }
	void SetCannotBeDropped() { m_bCannotbeDropped = true; }
	eg_bool CannotBeDropped() const { return m_bCannotbeDropped; }

	void SetWorldTF( const eg_mat& Mat ) { m_LastWorldTF = Mat; SetWorldTFInternal( Mat ); }
	const eg_mat& GetLastWorldTF() const { return m_LastWorldTF; }
	void SetViewTF( const eg_mat& Mat ) { m_LastViewTF = Mat; SetViewTFInternal( Mat ); }
	const eg_mat& GetLastViewTF() const { return m_LastViewTF; }
	void SetProjTF( const eg_mat& Mat ) { m_LastProjTF = Mat; SetProjTFInternal( Mat ); }
	const eg_mat& GetLastProjTF() const { return m_LastProjTF; }

	void DrawRawTris(const egv_vert_simple* pVerts, eg_uint nNumTris);
	void SetBoneMats(const eg_transform* pTrans, const eg_uint nCount);

	#define DL_FUNCTION_0_PARM( _function_ ) void _function_(){ egCmd Cmd; Cmd.Cmd = DLFUNC_##_function_; InsertSimpleCommand( &Cmd , 0 ); }
	#define DL_FUNCTION_1_PARM( _function_ , _parmtype0_ , _parmname0_ ) void _function_( _parmtype0_ _parmname0_ ){ egCmd Cmd; Cmd.Cmd = DLFUNC_##_function_; Cmd.Data._function_._parmname0_ = _parmname0_; InsertSimpleCommand( &Cmd , sizeof(Cmd.Data._function_) ); }
	#define DL_FUNCTION_2_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ ) void _function_( _parmtype0_ _parmname0_ , _parmtype1_ _parmname1_ ){ egCmd Cmd; Cmd.Cmd = DLFUNC_##_function_; Cmd.Data._function_._parmname0_ = _parmname0_; Cmd.Data._function_._parmname1_ = _parmname1_; InsertSimpleCommand( &Cmd , sizeof(Cmd.Data._function_) ); }
	#define DL_FUNCTION_3_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ ) void _function_( _parmtype0_ _parmname0_ , _parmtype1_ _parmname1_ , _parmtype2_ _parmname2_ ){ egCmd Cmd; Cmd.Cmd = DLFUNC_##_function_; Cmd.Data._function_._parmname0_ = _parmname0_; Cmd.Data._function_._parmname1_ = _parmname1_; Cmd.Data._function_._parmname2_ = _parmname2_; InsertSimpleCommand( &Cmd , sizeof(Cmd.Data._function_) ); }
	#define DL_FUNCTION_5_PARM( _function_ , _parmtype0_ , _parmname0_ , _parmtype1_ , _parmname1_ , _parmtype2_ , _parmname2_ , _parmtype3_ , _parmname3_ , _parmtype4_ , _parmname4_ ) void _function_( _parmtype0_ _parmname0_ , _parmtype1_ _parmname1_ , _parmtype2_ _parmname2_ , _parmtype3_ _parmname3_ , _parmtype4_ _parmname4_ ){ egCmd Cmd; Cmd.Cmd = DLFUNC_##_function_; Cmd.Data._function_._parmname0_ = _parmname0_; Cmd.Data._function_._parmname1_ = _parmname1_; Cmd.Data._function_._parmname2_ = _parmname2_; Cmd.Data._function_._parmname3_ = _parmname3_; Cmd.Data._function_._parmname4_ = _parmname4_; InsertSimpleCommand( &Cmd , sizeof(Cmd.Data._function_) ); }
	#include "EGDisplayListFunctions.items"
	#undef DL_FUNCTION_0_PARM
	#undef DL_FUNCTION_1_PARM
	#undef DL_FUNCTION_2_PARM
	#undef DL_FUNCTION_3_PARM
	#undef DL_FUNCTION_5_PARM

private:
	void InsertSimpleCommand( const egCmd* Cmd , eg_uint DataSize );

	template<class A> A* Alloc( eg_size_t Size ){ A* Out = static_cast<A*>(m_Heap.Alloc( Size , __FUNCTION__ , __FILE__ , __LINE__)); m_OutOfMem = m_OutOfMem || nullptr == Out; return Out; }
	template<class A> void Free( A*& Obj ){ m_Heap.Free( Obj ); Obj = nullptr; }
private:
	eg_uint       m_AssetState;
	EGStaticAlloc m_Heap;
	eg_bool       m_bCannotbeDropped:1;
	eg_bool       m_OutOfMem:1; //If we run out of memory make not of it so that we can just skip the draw.
	//Some metrics.
	static eg_size_t MaxBytesUsed;
};
