// Input Buttons for Explor
// (c) 2016 Beem Media
#pragma once

#include "EGInputTypes.h"

//
// Input
//
static const eg_cmd_t CMDA_QSAVE             = 1;
static const eg_cmd_t CMDA_QLOAD             = 2;

static const eg_cmd_t CMDA_FORWARD1          = 3;
static const eg_cmd_t CMDA_FORWARD2          = 4;
static const eg_cmd_t CMDA_BACK1             = 5;
static const eg_cmd_t CMDA_BACK2             = 6;
static const eg_cmd_t CMDA_TURNLEFT1         = 7;
static const eg_cmd_t CMDA_TURNLEFT2         = 8;
static const eg_cmd_t CMDA_TURNRIGHT1        = 9;
static const eg_cmd_t CMDA_TURNRIGHT2        = 10;
static const eg_cmd_t CMDA_STRAFELEFT1       = 11;
static const eg_cmd_t CMDA_STRAFELEFT2       = 12;
static const eg_cmd_t CMDA_STRAFERIGHT1      = 13;
static const eg_cmd_t CMDA_STRAFERIGHT2      = 14;

static const eg_cmd_t CMDA_GAME_NEXTPMEMBER  = 15;
static const eg_cmd_t CMDA_GAME_NEXTPMEMBER2 = 16;
static const eg_cmd_t CMDA_GAME_PREVPMEMBER  = 17;
static const eg_cmd_t CMDA_WAITTURN          = 18;

static const eg_cmd_t CMDA_GAMEMENU          = 19;

static const eg_cmd_t CMDA_CONTEXTMENU       = 20;

static const eg_cmd_t CMDA_QUESTLOG          = 21;
static const eg_cmd_t CMDA_QUICK_INVENTORY   = 22;
static const eg_cmd_t CMDA_MAPMENU           = 23;
static const eg_cmd_t CMDA_VIEWCHARMENU      = 24;

static const eg_cmd_t CMDA_FREEMOVE_FORWARD  = 25;
static const eg_cmd_t CMDA_FREEMOVE_BACK     = 26;
static const eg_cmd_t CMDA_FREEMOVE_LEFT     = 27;
static const eg_cmd_t CMDA_FREEMOVE_RIGHT    = 28;
static const eg_cmd_t CMDA_FREEMOVE_ASCEND   = 29;
static const eg_cmd_t CMDA_FREEMOVE_DESCEND  = 30;

static const eg_cmd_t CMDA_DEBUG_NEXT        = 31;
static const eg_cmd_t CMDA_DEBUG_PREV        = 32;
static const eg_cmd_t CMDA_DEBUG_SPEED       = 33;

static const eg_cmd_t CMDA_MENU_DELETE       = 34;
static const eg_cmd_t CMDA_KBMENU_BKSP       = 35;
static const eg_cmd_t CMDA_MENU_COMPARE      = 36;
static const eg_cmd_t CMDA_GAMEMENU_CLOSE    = 37;

static const eg_cmd_t CMDA_EX_MENU_LEFTRIGHT = 38; // Only exists for glyphs

static const struct exInputMapping
{
	eg_cmd_t Cmd;
	eg_cpstr Name;
	eg_cpstr GpButton;
	eg_cpstr KbButton;
}
EX_INPUT_MAPPING[] =
{
	{ CMDA_FORWARD1         , "EX_FORWARD1"         , "GP_LUP"    , "KBM_W"        },
	{ CMDA_BACK1            , "EX_BACK1"            , "GP_LDOWN"  , "KBM_S"        },
	{ CMDA_TURNRIGHT1       , "EX_TURNRIGHT1"       , "GP_RRIGHT" , "KBM_E"        },
	{ CMDA_TURNLEFT1        , "EX_TURNLEFT1"        , "GP_RLEFT"  , "KBM_Q"        },
	{ CMDA_STRAFERIGHT1     , "EX_STRAFERIGHT1"     , "GP_LRIGHT" , "KBM_D"        },
	{ CMDA_STRAFELEFT1      , "EX_STRAFELEFT1"      , "GP_LLEFT"  , "KBM_A"        },

	{ CMDA_FORWARD2         , "EX_FORWARD2"         , "GP_DUP"    , "KBM_UP"       },
	{ CMDA_BACK2            , "EX_BACK2"            , "GP_DDOWN"  , "KBM_DOWN"     },
	{ CMDA_TURNRIGHT2       , "EX_TURNRIGHT2"       , "GP_DRIGHT" , "KBM_RIGHT"    },
	{ CMDA_TURNLEFT2        , "EX_TURNLEFT2"        , "GP_DLEFT"  , "KBM_LEFT"     },
	{ CMDA_STRAFERIGHT2     , "EX_STRAFERIGHT2"     , ""          , ""             },
	{ CMDA_STRAFELEFT2      , "EX_STRAFELEFT2"      , ""          , ""             },

	{ CMDA_GAMEMENU         , "EX_GAMEMENU"         , "GP_START"  , "KBM_ESCAPE"   },
	{ CMDA_GAMEMENU_CLOSE   , "EX_GAMEMENUCLOSE"    , "GP_START"  , ""             },
	{ CMDA_QSAVE            , "EX_QSAVE"            , ""          , "KBM_F5"       },
	{ CMDA_QLOAD            , "EX_QLOAD"            , ""          , "KBM_F9"       },
	{ CMDA_WAITTURN         , "EX_WAITTURN"         , "GP_X"      , "KBM_SPACE"    },
	{ CMDA_QUESTLOG         , "EX_QUESTLOG"         , "GP_R2"     , "KBM_L"        },
	{ CMDA_MAPMENU          , "EX_MAPMENU"          , "GP_Y"      , "KBM_M"        },
	{ CMDA_VIEWCHARMENU     , "EX_CHARMENU"         , "GP_L2"     , "KBM_C"        },
	{ CMDA_CONTEXTMENU      , "EX_CONTEXTMENU"      , "GP_A"      , "KBM_RETURN"   },
	{ CMDA_GAME_NEXTPMEMBER2, "EX_CHANGEPARTY"      , ""          , "KBM_TAB"      },
	{ CMDA_GAME_NEXTPMEMBER , "EX_NEXTPARTYMEMBER"  , "GP_R1"     , "KBM_RBRACKET" },
	{ CMDA_GAME_PREVPMEMBER , "EX_PREVPARTYMEMBER"  , "GP_L1"     , "KBM_LBRACKET" },
	{ CMDA_QUICK_INVENTORY  , "EX_QUICK_INVENTORY"  , "GP_BACK"   , "KBM_I"        },
	{ CMDA_FREEMOVE_FORWARD , "EX_FREEMOVE_FORWARD" , ""          , "KBM_W"        },
	{ CMDA_FREEMOVE_BACK    , "EX_FREEMOVE_BACK"    , ""          , "KBM_S"        },
	{ CMDA_FREEMOVE_RIGHT   , "EX_FREEMOVE_RIGHT"   , ""          , "KBM_D"        },
	{ CMDA_FREEMOVE_LEFT    , "EX_FREEMOVE_LEFT"    , ""          , "KBM_A"        },
	{ CMDA_FREEMOVE_ASCEND  , "EX_FREEMOVE_ASCEND"  , ""          , "KBM_SPACE"    },
	{ CMDA_FREEMOVE_DESCEND , "EX_FREEMOVE_DESCEND" , ""          , "KBM_LCONTROL" },
	{ CMDA_KBMENU_BKSP      , "EX_KBMENU_BKSP"      , "GP_X"      , "KBM_BACK"     },
	{ CMDA_DEBUG_NEXT       , "EX_DEBUG_NEXT"       , ""          , "KBM_EQUALS"   },
	{ CMDA_DEBUG_PREV       , "EX_DEBUG_PREV"       , ""          , "KBM_MINUS"    },
	{ CMDA_DEBUG_SPEED      , "EX_DEBUG_SPEED"      , ""          , "KBM_LSHIFT"   },
	{ CMDA_MENU_DELETE      , "EX_MENU_DELETE"      , "GP_X"      , "KBM_DELETE"   },
	{ CMDA_MENU_COMPARE     , "EX_MENU_COMPARE"     , "GP_R3"     , "KBM_LCONTROL" },
};
