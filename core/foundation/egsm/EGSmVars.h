// (c) 2017 Beem Media

#pragma once

#include "EGSmTypes.h"
#include "EGCppLibAPI.h"

typedef std::function<void(const egsmVarDeclScr&)> egsmVarDeclCallback;

void EGSmVars_LoadVarDecl( const eg_char8* FileAsString , const eg_size_t FileAsStringLen , const egsmVarDeclCallback& Callback );
