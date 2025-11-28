// (c) 2018 Beem Media

#pragma once

#include "EGCppLibAPI.h"

typedef std::function<void(eg_cpstr)> EGPerforceLogFn;

eg_bool __declspec(dllexport) EGPerforce_CheckoutFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn );
eg_bool __declspec(dllexport) EGPerforce_RevertFile( eg_cpstr16 Filename , eg_bool bOnlyIfUnchanged , EGPerforceLogFn& LogFn );
eg_bool __declspec(dllexport) EGPerforce_AddFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn );
eg_bool __declspec(dllexport) EGPerforce_DeleteFile( eg_cpstr16 Filename , EGPerforceLogFn& LogFn );
