// (c) 2018 Beem Media

#pragma once

enum class eg_rfl_gen_t
{
	Unknown,
	Class,
	Struct,
	Enum,
};

struct egRflGenProp
{
	eg_d_string VarType;
	eg_d_string VarName;
};

struct egRflGenStruct
{
	eg_rfl_gen_t          Type = eg_rfl_gen_t::Unknown;
	eg_d_string           Name;
	EGArray<eg_d_string>  ParentClasses;
	EGArray<egRflGenProp> Properties;
};

struct egRflGenInfo
{
	EGArray<egRflGenStruct> Structs;

	void PrintToLog( eg_log_channel LogChannel );
	eg_bool HasInfo() const { return Structs.HasItems(); }
};

eg_bool EGRflGen_ProcessFile( eg_cpstr16 Filename );
int EGRflGen_main( int argc , char* argv[] );