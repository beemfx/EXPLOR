// (c) 2016 Beem Media

#include "EGObject.h"

EG_CLASS_DECL( EGObject )

eg_bool EGObject::IsA( EGClass* Class ) const
{
	return m_Class->IsA( Class );
}

eg_int EGObject::Release()
{
	assert( m_RefCount > 0 ); 
	m_RefCount--; 
	eg_int RefCountOut = m_RefCount; 
	if( m_RefCount <= 0 ) 
	{ 
		CleanWeakPtrs(); 
		OnDestruct(); 
		delete this; 
	}
	return RefCountOut;
}

void EGObject::AddWeakPtr( EGWeakPtrBase* Ptr )
{
	m_WeakPtrs.Insert( Ptr );
}

void EGObject::RemoveWeakPtr( EGWeakPtrBase* Ptr )
{
	m_WeakPtrs.Remove( Ptr );
}

void EGObject::CleanWeakPtrs()
{
	while( m_WeakPtrs.HasItems() )
	{
		m_WeakPtrs.GetOne()->Unregister();
	}
}

eg_bool EGClass::IsA( EGClass* Class ) const
{
	for( const EGClass* ClassToCheck = this; nullptr != ClassToCheck; ClassToCheck = ClassToCheck->GetParentClass() )
	{
		if( Class == ClassToCheck )
		{
			return true;
		}
	}

	return false;
}

eg_byte EGClass::RegListMem[sizeof( EGClass::EGRegList )];
EGClass::EGRegList* EGClass::RegList = nullptr;

EGClass* EGClass::FindClass( eg_cpstr ClassName )
{
	eg_string_crc ClassNameCrc = eg_string_crc(ClassName);

	if( ClassNameCrc.IsNotNull() )
	{
		for( EGClass* Item : *RegList )
		{
			if( Item->m_ClassNameCrc == ClassNameCrc )
			{
				return Item;
			}
		}
	}

	return nullptr;
}

EGClass* EGClass::FindClassSafe( eg_cpstr ClassName, EGClass& DefaultClass )
{
	EGClass* ClassOut = EGClass::FindClass( ClassName );
	if( ClassOut == nullptr || !ClassOut->IsA( &DefaultClass ) )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ "%s was not a valid %s class. Using default.", ClassName, DefaultClass.GetName() );
		ClassOut = &DefaultClass;
	}

	return ClassOut;
}

EGClass* EGClass::FindClassOfType( eg_cpstr ClassName, EGClass& DefaultClass )
{
	EGClass* ClassOut = EGClass::FindClass( ClassName );
	if( ClassOut == nullptr || !ClassOut->IsA( &DefaultClass ) )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ "%s was not a valid %s class." , ClassName, DefaultClass.GetName() );
		ClassOut = nullptr;
	}

	return ClassOut;
}

EGClass* EGClass::FindClassWithBackup( eg_cpstr ClassName, EGClass& BackupClass )
{
	EGClass* ClassOut = EGClass::FindClass( ClassName );
	if( ClassOut == nullptr )
	{
		EGLogf( eg_log_t::Warning, __FUNCTION__ "%s was not a valid class. Using backup class %s.", ClassName, BackupClass.GetName() );
		ClassOut = &BackupClass;
	}

	return ClassOut;
}

void EGClass::FindAllClassesOfType( EGClass* BaseClass, EGArray<EGClass*>& Out )
{
	if( BaseClass )
	{
		for( EGClass* Item : *RegList )
		{
			if( Item->IsA( BaseClass ) )
			{
				Out.Append( Item );
			}
		}
	}
}

void EGClass::Register( EGClass* Class )
{
	if( nullptr == RegList )
	{
		RegList = new( RegListMem ) EGRegList;
	}
	if( FindClass( Class->m_ClassName ) )
	{
		assert( false ); // A class with this id was already registered, or name collision.
		return;
	}

	RegList->Insert( Class );
}

void EGClass::Unregister( EGClass* Class )
{
	RegList->Remove( Class );
}

