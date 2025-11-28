/******************************************************************************
File: EntDict.h

This is designed to contain the definitions of all the entities in the game. 
This is used on both the server and the client. The dictionary is loaded at
the game bootup. All definitions found on the disk are loaded.

Technically this is not thread safe because any thread can grab a def with
EntDict_GetDef. But it is const so the threads aren't modifying the defs so
it should be okay. However, it should be noted that the main thread does load
render and sound assets related to the item. (The m_Assets item in the edef is
mutable). But they are only modified on one thread, which is why I've chosen
to allow this.

For true thread safety, an option would be to create two copies of the
dictionary class, and use one for the client and one for the server threads.
Until an actual crash or bug occurs, I'm content with this.

(c) 2014 Blaine Myers.
******************************************************************************/
#pragma once

void EntDict_Init( void );
void EntDict_Deinit( void );
void EntDict_Update( void );
const class EGEntDef* EntDict_GetDef( eg_string_crc TemplateCrcId );
void EntDict_ShowInfo( void );
void EntDict_GetDefStrings( EGArray<eg_string>& ArrayOut , eg_bool bIncludeGameClasses , eg_bool bIncludeUiClasses );
