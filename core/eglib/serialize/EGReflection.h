// (c) 2018 Beem Media

#pragma once

#include "EGReflectionDeclMacros.h"
#include "EGEditControlType.h"

struct egRflVar;
struct egRflEditor;
class EGFileData;

#define EG_FRIEND_RFL( _type_ ) friend struct __EGRfl_##_type_;

enum class eg_rfl_value_t
{
	Unknown,
	Primitive,
	Struct,
	Array,
};

enum class eg_rfl_serialize_fmt
{
	Unknown,
	XML,
};

struct egRflValueType
{
	virtual ~egRflValueType(){ }

	virtual eg_rfl_value_t GetType() const { return eg_rfl_value_t::Unknown; }
	virtual eg_size_t GetPrimitiveSize() const { return 0; }
	virtual void GetChildren( EGArray<egRflVar>& Out ) const { unused( Out ); }
	virtual void SetFromString( void* Data , eg_cpstr Str ) const = 0;
	virtual eg_d_string ToString( const void* Data ) const = 0;
	virtual const egRflValueType* GetArrayChildType() const { return nullptr; }
	virtual eg_edit_ctrl_t GetEditControlType() const { return eg_edit_ctrl_t::Unk; }
	virtual void GetComboChoices( const void* Data , EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const { unused( Data , Out ); bManualEditOut = true; }
	virtual void PostLoad( void* Data , eg_cpstr8 Filename , eg_bool bForEditor ) const { unused( Data , Filename , bForEditor ); }
};

struct egRflEditor final
{
private:

	// typedef eg_s_string_base<eg_char8,32> eg_rfl_ed_name;
	typedef eg_d_string eg_rfl_ed_name;

private:

	static const eg_mem_pool s_MemPool;

private:

	eg_rfl_ed_name         NameAsDynamicString = CT_Clear;
	eg_cpstr               NameAsStaticString = nullptr;
	eg_string_crc          NameAsCrc = CT_Clear;
	eg_byte*               Data = nullptr;
	const egRflValueType*  Type = nullptr;
	EGArray<egRflEditor>   Children;
	EGArray<egRflVar>      ChildVars = s_MemPool; // Cache to avoid allocations when deserializing
	mutable eg_rfl_ed_name DisplayName = CT_Clear;
	eg_bool                bIsExpanded = true;
	eg_bool                bIsEditable = true;
	eg_bool                bIsSerialized = true;
	eg_bool                bMarkedAsFound = true; // For deserialization, to wipe properties that no longer exist

public:

	egRflEditor();
	egRflEditor( eg_ctor_t Ct );
	egRflEditor( eg_cpstr InName , void* InData , const egRflValueType* InType );
	egRflEditor( void* InData , const egRflValueType* InType );
	egRflEditor( const egRflVar& Var , void* InData );
	egRflEditor( egRflEditor& rhs ) = default;
	egRflEditor( egRflEditor&& rhs ){ *this = std::move( rhs ); }

	egRflEditor& operator = ( const egRflEditor& rhs ) = default;

	const egRflEditor& operator = ( egRflEditor&& rhs )
	{
		NameAsDynamicString = std::move( rhs.NameAsDynamicString );
		NameAsStaticString = rhs.NameAsStaticString;
		NameAsCrc = rhs.NameAsCrc;
		DisplayName = std::move( rhs.DisplayName );
		Data = rhs.Data;
		Type = rhs.Type;
		Children = std::move( rhs.Children );
		ChildVars = std::move( rhs.ChildVars );
		bIsExpanded = rhs.bIsExpanded;
		bIsEditable = rhs.bIsEditable;
		bIsSerialized = rhs.bIsSerialized;
		bMarkedAsFound = rhs.bMarkedAsFound;
		return *this;
	}

	eg_bool operator == ( const egRflEditor& rhs ) const
	{
		return Data == rhs.Data && Type == rhs.Type && EGString_Equals( GetVarName() , rhs.GetVarName() );
	}

	void Rebuild();
	void HandleArrayChanged();
	void HandlePostLoad();

	void DebugPrint( eg_uint Level ) const;

	eg_bool IsValid() const { return Data && Type; }
	eg_rfl_value_t GetType() const { return Type ? Type->GetType() : eg_rfl_value_t::Unknown; }
	const egRflValueType* GetValueType() const { return Type; }
	eg_bool IsPrimitive() const { return Type ? Type->GetType() == eg_rfl_value_t::Primitive : false; }
	eg_bool IsExpanded() const { return bIsExpanded; }
	void SetExpanded( eg_bool bNewValue ) { bIsExpanded = bNewValue; }
	eg_bool IsEditable() const { return bIsEditable; }
	void SetEditable( eg_bool bNewValue ) { bIsEditable = bNewValue; }
	eg_bool IsSerialized() const { return bIsSerialized; }
	void SetSerialized( eg_bool bNewValue ) { bIsSerialized = bNewValue; }
	void SetDisplayName( eg_cpstr NewDisplayName ) { DisplayName = NewDisplayName; }
	eg_edit_ctrl_t GetEditControlType() const { return Type ? Type->GetEditControlType() : eg_edit_ctrl_t::Unk; }

	void SetRawData( const void* RawData , eg_size_t RawDataSize );
	void SetFromString( eg_cpstr Str );
	eg_d_string ToString() const;
	eg_cpstr GetVarName() const { return NameAsDynamicString.Len() > 0 ? *NameAsDynamicString : NameAsStaticString ? NameAsStaticString : ""; }
	eg_string_crc GetVarNameCrc() const { return NameAsCrc; }
	eg_cpstr GetDisplayName() const;
	eg_size_t GetNumChildren() const { return Children.Len(); }
	egRflEditor GetChild( eg_cpstr VarName );
	egRflEditor GetArrayChild( eg_size_t Index );
	egRflEditor InsertArrayChild();
	egRflEditor* GetChildPtr( eg_cpstr VarName );
	egRflEditor* GetArrayChildPtr( eg_size_t Index );
	egRflEditor* InsertArrayChildPtr();
	void DeleteArrayChildAt( eg_size_t Index );
	void DeleteArrayChild( egRflEditor* Child );
	void InsertArrayChildAt( eg_size_t Index );
	void InsertArrayChildBefore( egRflEditor* Child );
	void GetChildren( EGArray<egRflEditor>& Out );
	void GetChildrenPtrs( EGArray<egRflEditor*>& Out );
	void TrimArrayReserved();
	void ClearArray();
	void ReserveArraySpace( eg_size_t Size );
	void Serialize( eg_rfl_serialize_fmt Format , eg_int Depth , EGFileData& MemFile ) const;
	void SetExpandedInEditor( eg_bool bNewValue ) { bIsExpanded = bNewValue; }
	void SetChildValue( eg_cpstr VarName , eg_cpstr NewValue );
	const void* GetData() const { return Data; }
	void* GetData() { return Data; }
	eg_d_string GetPreviewValue() const;
	void GetComboChoices( EGArray<eg_d_string>& Out , eg_bool& bManualEditOut ) const;
	void ExecutePostLoad( eg_cpstr8 Filename , eg_bool bForEditor );
	void AddExplicitChild( const egRflEditor& ChildEditor );
	void CopyFrom( const egRflEditor& SourceEditor );

	template<typename Type> void SetChildValue( eg_cpstr VarName , const Type& NewValue )
	{
		egRflEditor Child = GetChild( VarName );
		if( Child.IsValid() )
		{
			Child.SetRawData( &NewValue , sizeof(NewValue) );
		}
		else
		{
			assert( false ); // No such child.
		}
	}

	static eg_string_big BeautifyVarNameAsDisplayName( eg_cpstr InStr );
};

struct egRflVar final
{
private:

	eg_cpstr Name = "";
	eg_size_t Offset = 0;
	const egRflValueType* ValueType = nullptr;

public:

	egRflVar() = default;

	const egRflVar& operator = ( const egRflVar& rhs )
	{
		Name = rhs.Name;
		Offset = rhs.Offset;
		ValueType = rhs.ValueType;
		return *this;
	}

	egRflVar( eg_cpstr InName , eg_size_t InOffset , const egRflValueType& InValueType )
	: Name( InName ) , Offset( InOffset ) , ValueType( &InValueType )
	{
	}

	eg_cpstr GetName() const { return Name; }
	eg_size_t GetOffset() const { return Offset; }
	const egRflValueType* GetType() const { return ValueType; }

	eg_bool IsValid() const { return nullptr != ValueType && Name && Name[0] != '\0'; }
};

struct egRflStructType : public egRflValueType , public IListable
{
private:

	typedef EGList<egRflStructType> EGRflStructList;

	static EGRflStructList* s_AllTypes;
	static eg_byte          s_AllTypesMem[sizeof(EGRflStructList)];

public:

	static const egRflStructType* FindStructTypeByName( eg_cpstr InName );

private:

	eg_cpstr const m_ClassName = nullptr;

public:

	egRflStructType( eg_cpstr InClassName );
	~egRflStructType();
	egRflStructType( const egRflStructType& rhs ) = delete;
	const egRflStructType& operator = ( const egRflStructType& rhs ) = delete;
	virtual eg_rfl_value_t GetType() const override { return eg_rfl_value_t::Struct; }
	virtual void SetFromString( void* Data , eg_cpstr Str ) const override { unused( Data , Str ); assert( false ); }
	virtual eg_d_string ToString( const void* Data ) const override { unused( Data ); assert( false ); return ""; }
	virtual eg_edit_ctrl_t GetEditControlType() const { return eg_edit_ctrl_t::Struct; }
};

struct egRflArrayType : public egRflValueType
{
	const egRflValueType& ValueType;

	egRflArrayType() = delete;
	egRflArrayType( const egRflArrayType& rhs ) = delete;
	const egRflArrayType& operator = ( const egRflArrayType& rhs ) = delete;

	egRflArrayType( const egRflValueType& InValueType )
	: ValueType( InValueType )
	{
	}

	virtual eg_rfl_value_t GetType() const override { return eg_rfl_value_t::Array; }
	virtual void SetFromString( void* Data , eg_cpstr Str ) const override { unused( Data , Str ); assert( false ); }
	virtual eg_d_string ToString( const void* Data ) const override { unused( Data ); assert( false ); return ""; }
	virtual const egRflValueType* GetArrayChildType() const override { return &ValueType; }
	virtual eg_edit_ctrl_t GetEditControlType() const { return eg_edit_ctrl_t::Array; }
};

struct egRflEnumType : public egRflValueType
{
	egRflEnumType(){ }
	egRflEnumType( const egRflEnumType& rhs ) = delete;
	const egRflEnumType& operator = ( const egRflEnumType& rhs ) = delete;

	virtual eg_rfl_value_t GetType() const override final { return eg_rfl_value_t::Primitive; }
	virtual eg_size_t GetPrimitiveSize() const override { assert( false ); return 0; }
};

egRflEditor EGReflection_GetEditorForClass( eg_cpstr ClassName , void* Data , eg_cpstr Name );
extern egRflStructType Rfl_CustomEditor;
