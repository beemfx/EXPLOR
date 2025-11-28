// (c) 2011 Beem Media

//DRAM
MAP_DATA_SECTION( EGWorldMapBase::egHeader        , H )
MAP_DATA_SECTION( EGWorldMapBase::egRasterSegment , Segments )
MAP_DATA_SECTION( EGWorldMapBase::egRegion        , Regions )
MAP_DATA_SECTION( EGWorldMapBase::egGeoBlock      , GeoBlocks )
MAP_DATA_SECTION( EGWorldMapBase::egGeoAlignedBox , GeoAlignedBoxes )
MAP_DATA_SECTION( EGWorldMapBase::egLight         , Lights )
MAP_DATA_SECTION( EGWorldMapBase::egPortal        , Portals )
MAP_DATA_SECTION( EGWorldMapBase::egMtrl          , Mtrls )
MAP_DATA_SECTION( EGWorldMapBase::egTag           , Tags )
MAP_DATA_SECTION( EGWorldMapBase::egGraph         , Graphs )
MAP_DATA_SECTION( EGWorldMapBase::egGraphVertex   , GraphVerts )
MAP_DATA_SECTION( eg_char                         , Strings )

//VRAM:
MAP_DATA_SECTION( egv_vert_mesh                          , Vertexes )
