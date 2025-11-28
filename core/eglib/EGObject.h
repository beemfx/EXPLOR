/*******************************************************************************
EGObject

The EGObject is a shared base class for classes that are managed in the engine
but implemented in part in the game layer (such as Menus and the game Client)
is is a robust way of using a EGClass to create an object of a particular type
without having to know what the particular type is. An EGObject should be
created using EGNewObject and deleted using EGDeleteObject. Declaring an
EGObject requires a little work:

// Must have EG_CLASS_BODY in the class body with the class name and the parent
// class
class EGSomeClass : public EGObject
{
	EG_CLASS_BODY( EGSomeClass , EGObject )
}

// And in the cpp file:
EG_CLASS_DECL( EGSomeClass )

Note EG_CLASS_BODY must be declared correctly or the class won't behave
correctly the 2nd parameter must be the parent class whether it be EGObject or 
some other object.

(c) 2016 Beem Media
*******************************************************************************/

#pragma once

#include "EGList.h"
#include "EGWeakPtrBase.h"

class EGObject;

struct egNewObjectParms
{
	const void* SerializedMem;
	eg_size_t SerializedMemSize;

	egNewObjectParms( eg_ctor_t Ct )
	: SerializedMem( nullptr )
	, SerializedMemSize( 0 )
	{
		unused( Ct );
	}
};

class EGClass : public IListable
{
private:

	EGClass*const   m_ParentClass;
	const eg_size_t m_ObjectSize;
	eg_cpstr        m_ClassName;
	eg_string_crc   m_ClassNameCrc;

public:

	EGClass( EGClass* ParentClass , eg_cpstr ClassName , eg_size_t ObjectSize )
	: m_ParentClass(ParentClass) 
	, m_ClassName(ClassName) 
	, m_ClassNameCrc(ClassName) 
	, m_ObjectSize( ObjectSize )
	{ 
		Register( this ); 
	}

	~EGClass(){ Unregister( this ); }

	virtual EGObject* CreateInstance( eg_mem_pool MemPool , const egNewObjectParms& Parms ) = 0;

	const EGClass* GetParentClass()const{ return m_ParentClass; }

	eg_bool IsA( EGClass* Class ) const;
	eg_size_t GetObjectSize() const{ return m_ObjectSize; }
	eg_cpstr GetName() const{ return m_ClassName; }

	static EGClass* FindClass( eg_cpstr ClassName );
	static EGClass* FindClassSafe( eg_cpstr ClassName, EGClass& DefaultClass );
	static EGClass* FindClassOfType( eg_cpstr ClassName, EGClass& DefaultClass );
	static EGClass* FindClassWithBackup( eg_cpstr ClassName, EGClass& BackupClass );
	static void FindAllClassesOfType( EGClass* BaseClass , EGArray<EGClass*>& Out );

private:

	class EGRegList : public EGList<EGClass>
	{
	public:
		EGRegList() : EGList( EGList::DEFAULT_ID ) {}
	};

	static eg_byte RegListMem[];
	static EGRegList* RegList;

	static void Register( EGClass* Class );
	static void Unregister( EGClass* Class );
};

class EGObject
{
public:

	virtual ~EGObject() { }

	eg_bool IsA( EGClass* Class ) const;
	eg_size_t GetObjectSize() const{ return m_Class ? m_Class->GetObjectSize() : 0; }
	EGClass* GetObjectClass() const { return m_Class; }
	void AddRef() { m_RefCount++; }
	eg_int Release();

	static void* operator new( eg_size_t Size , eg_mem_pool MemPool , const egNewObjectParms& Parms )
	{
		void* Out = EGMem2_Alloc( Size , MemPool );
		EGMem_Set( Out , 0 , Size );
		
		if( Parms.SerializedMemSize > 0 )
		{
			eg_size_t SizeToCopy = EG_Min( Size , Parms.SerializedMemSize );
			EGMem_Copy( Out , Parms.SerializedMem , SizeToCopy );
		}
		return Out;
	}

	static void operator delete( void* pData , eg_mem_pool MemPool , const egNewObjectParms& Parms )
	{
		unused( MemPool , Parms );
		return EGMem2_Free( pData );
	}

	static void operator delete( void* pData )
	{
		return EGMem2_Free( pData );
	}

	static EGClass& GetStaticClass() { return StaticClassInst; }

private:
	friend class EGWeakPtrBase;

	void AddWeakPtr( EGWeakPtrBase* Ptr );
	void RemoveWeakPtr( EGWeakPtrBase* Ptr );
	void CleanWeakPtrs();

private:
	class EGInnerClass : public EGClass
	{
	public:
		EGInnerClass(): EGClass( nullptr , "EGObject" , sizeof(EGObject) ){ }
		virtual EGObject* CreateInstance( eg_mem_pool MemPool , const egNewObjectParms& Parms ){ assert( false ); unused(MemPool,Parms); return nullptr; }
	};
private:
	EGClass* m_Class = nullptr;
	EGList<EGWeakPtrBase> m_WeakPtrs = EGList<EGWeakPtrBase>::DEFAULT_ID;
	eg_int m_RefCount = 0;
protected:
	void InitClassInfo( EGClass* Class ){ m_Class = Class; }
	virtual void OnConstruct(){ }
	virtual void OnDestruct(){ }
private:
	static EGInnerClass StaticClassInst;
};

#define EG_CLASS_BODY( _class_ , _parentclass_ ) \
private:\
	typedef _parentclass_ Super;\
	typedef _class_ ThisClass;\
	class EGInnerClass : public EGClass\
	{ \
	public:\
		EGInnerClass(): EGClass( &Super::GetStaticClass() , #_class_ , sizeof(_class_) ){ }\
		virtual EGObject* CreateInstance( eg_mem_pool MemPool , const egNewObjectParms& Parms) override final{ _class_* NewObject = new ( MemPool , Parms ) _class_; NewObject->InitClassInfo( &StaticClassInst ); NewObject->AddRef(); NewObject->OnConstruct(); return NewObject; }\
	}; \
	static EGInnerClass StaticClassInst; \
public:\
	static EGClass& GetStaticClass(){ return StaticClassInst; }\
private:

#define EG_ABSTRACT_CLASS_BODY( _class_ , _parentclass_ ) \
private:\
	typedef _parentclass_ Super;\
	typedef _class_ ThisClass;\
	class EGInnerClass : public EGClass\
	{ \
	public:\
		EGInnerClass(): EGClass( &Super::GetStaticClass() , #_class_ , sizeof(_class_) ){ }\
		virtual EGObject* CreateInstance( eg_mem_pool MemPool , const egNewObjectParms& Parms) override final { unused( MemPool , Parms ); assert( false ); return nullptr; }\
	}; \
	static EGInnerClass StaticClassInst; \
public:\
	static EGClass& GetStaticClass(){ return StaticClassInst; }\
private:

#define EG_CLASS_DECL( _class_ ) \
_class_::EGInnerClass _class_::StaticClassInst; \
EGClass& _class_##_GetClass() { return _class_::GetStaticClass(); }

template<typename T> T* EGCast( EGObject* Object )
{
	if( Object && Object->IsA( &T::GetStaticClass() ) )
	{
		return static_cast<T*>(Object);
	}

	return nullptr;
}

template<typename T> const T* EGCast( const EGObject* Object )
{
	if( Object && Object->IsA( &T::GetStaticClass() ) )
	{
		return static_cast<const T*>( Object );
	}

	return nullptr;
}

static inline EGObject* EGNewObject( EGClass* Class , eg_mem_pool MemPool = eg_mem_pool::DefaultObject , egNewObjectParms Parms = CT_Default )
{
	return Class ? Class->CreateInstance( MemPool , Parms ) : nullptr;
}

template<typename T> static inline  T* EGNewObject( EGClass* Class , eg_mem_pool MemPool = eg_mem_pool::DefaultObject , egNewObjectParms Parms = CT_Default )
{
	return Class && Class->IsA( &T::GetStaticClass() ) ? EGCast<T>( EGNewObject( Class , MemPool , Parms ) ) : nullptr;
}

template<typename T> static inline T* EGNewObject( eg_mem_pool MemPool = eg_mem_pool::DefaultObject , egNewObjectParms Parms = CT_Default )
{ 
	return EGCast<T>( EGNewObject( &T::GetStaticClass() , MemPool , Parms ) );
}

static inline void EGDeleteObject( EGObject* Object )
{
	eg_int RefCount = Object->Release();
	unused( RefCount );
	assert( RefCount == 0 );
}