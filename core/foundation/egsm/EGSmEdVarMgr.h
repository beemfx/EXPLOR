// (c) 2016 Beem Media

#pragma once

#include "EGSmTypes.h"

void EGSmEdVarMgr_InitForFile( eg_cpstr16 Filename );
void EGSmEdVarMgr_GetVarsOfType( egsm_var_t VarType , EGArray<egsmVarDeclScr>& Out );
void EGSmEdVarMgr_GetFunctions( EGArray<egsmVarDeclScr>& Out );
egsmVarDeclScr EGSmEdVarMgr_GetFunctionInfo( eg_string_crc Name );
void EGSmEdVarMgr_Deinit();
