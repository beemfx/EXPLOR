// (c) 2016 Beem Media

#pragma once

int EGLytEd_Run( eg_cpstr16 InitialFilename , const struct egSdkEngineInitParms& EngineParms );
void EGLytEd_DrawPreview();
void EGLytEd_SetDirty();
eg_string_big EGLytEd_GetCurrentGameFilename();