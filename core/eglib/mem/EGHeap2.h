#pragma once
#include "EGMemAlloc.h"
#include "EGList.h"

#define EGHEAP2_STORE_DEBUG_DATA 0

class EGHeap2: public IMemAlloc
{
public:
	EGHeap2()
	: m_AllocList(HEAD_LIST_ID)
	, m_FreeList(FREE_LIST_ID)
	, m_MemSize(0)
	, m_Mem(nullptr)
	, m_MemAddr(0)
	, m_MinAlloc(0)
	, m_NumAllocs(0)
	, m_BiggestAlloc(0)
	, m_Alignment(0)
	{
		//
	}

	EGHeap2( void* Mem , eg_size_t Size , eg_size_t MinAlloc = 0 , eg_uint Alignment = EG_ALIGNMENT )
	: m_AllocList(HEAD_LIST_ID)
	, m_FreeList(FREE_LIST_ID)
	, m_MemSize(0)
	, m_Mem(nullptr)
	, m_MemAddr(0)
	, m_MinAlloc(0)
	, m_NumAllocs(0)
	, m_BiggestAlloc(0)
	, m_Alignment(0)
	, m_MemUsedAtOnce(0)
	, m_MemAlloced(0)
	{ 
		Init( Mem , Size , MinAlloc , Alignment ); 
	}

	void Init( void* Mem , eg_size_t Size , eg_size_t MinAlloc = 0 , eg_uint Alignment = EG_ALIGNMENT );
	void Deinit();

	virtual void* Alloc( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line ); //O(n)
	virtual void* AllocHigh( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line ); //O(n)
	virtual void  Free( void* Mem ); //O(1)

	virtual eg_size_t GetInfo( enum INFO_T Type)const;

	eg_uintptr_t GetRelativeAddress( void* Alloc )const;
	void*        GetAbsoluteAddress( eg_uintptr_t Addr )const;

	void         FreeAll(); //Not recommended to use unless you absolutely know what you're doing. (Mostly this is to prevent asserts on heaps that we know we can dispose of.)

private:

	struct egMainList: public IListable{ egMainList():IListable(){}};
	struct egFreeList: public IListable{ egFreeList():IListable(){}};
	
	struct egBlock: public egMainList, public egFreeList
	{
		egBlock()
		: egMainList()
		, egFreeList()
		, ChunkBegin(0)
		, ChunkSize(0)
#if EGHEAP2_STORE_DEBUG_DATA
		, Line(0)
		, Type(nullptr)
		, File(nullptr)
#endif // EGHEAP2_STORE_DEBUG_DATA
		, bAllocated(false)
		{}

		eg_size_t        ChunkBegin; //Index in the chunk where the memory begins (this is where the header is located) (For alignment, there may be a small gap between this and the previous chunk).
		eg_size_t        ChunkSize;  //The actual number of bytes taken by this chunk including header and and alignment.
#if EGHEAP2_STORE_DEBUG_DATA
		eg_uint          Line;
		eg_cpstr8        Type;
		eg_cpstr8        File;
#endif // EGHEAP2_STORE_DEBUG_DATA
		eg_bool          bAllocated:1;
	};
private:
	static void GetInfo_Iterate_CountFragments( egMainList* Item , eg_uintptr_t Data );
	static void GetInfo_Iterate_CountAllocMem( egMainList* Item , eg_uintptr_t Data );
	static void GetInfo_Iterate_CountFreeMem( egMainList* Item , eg_uintptr_t Data );
private:
	static const eg_uint HEAD_LIST_ID = 0xFEA0;
	static const eg_uint FREE_LIST_ID = 0xEAF0;
private:
	eg_uint      m_NumAllocs;
	eg_size_t    m_BiggestAlloc;
	eg_size_t    m_MinAlloc;
	eg_uint      m_Alignment;
	eg_size_t    m_MemSize;
	eg_size_t    m_MemUsedAtOnce;
	eg_size_t    m_MemAlloced;
	void*        m_Mem;
	eg_uintptr_t m_MemAddr;
	EGList<egMainList> m_AllocList;
	EGList<egFreeList> m_FreeList;

private:
	void* AllocInternal( eg_size_t Size , eg_cpstr8 const Type , eg_cpstr8 const File , const eg_uint Line , eg_bool bHigh );
	egBlock* FindFreeBlock( eg_size_t SizeNeeded , eg_bool bHigh );
	void InsertIntoFreeList( egBlock* Block );
	eg_size_t GetBlockSize()const;
	void AssertIntegrity();
};