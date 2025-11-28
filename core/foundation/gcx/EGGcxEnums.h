// (c) 2016 Beem Media

// Grid Cartographer Enums

#pragma once

enum class gcx_direction
{
	UNKNOWN,
	UP,
	RIGHT,
	DOWN,
	LEFT,
	ALL,
	NONE,
};

enum class gcx_grid_t
{
	UNKNOWN,
	SQUARE,
	HEX,
};

enum class gcx_tile_origin
{
	UNKNOWN,
	BOTTOM_LEFT,
};

// LU = Left or Up , RD = Right or Down
enum class gcx_edge_t
{
	NONE                        = 0,
	WALL                        = 1,
	DOOR                        = 2,
	LOCKED_DOOR                 = 3,
	HIDDEN_DOOR                 = 4,
	ONE_WAY_DOOR_EXIT_LU        = 5,
	ONE_WAY_HIDDEN_DOOR_EXIT_LU = 6,
	ONE_WAY_WALL_EXIT_LU        = 7,
	ONE_WAY_DOOR_EXIT_RD        = 8,
	ONE_WAY_HIDDEN_DOOR_EXIT_RD = 9,
	ONE_WAY_WALL_EXIT_RD        = 10,
	EMPTY_DOOR_FRAME            = 12,
	SECRET_WALL                 = 13,
	TRAPPED_DOOR                = 14,
	HALF_DOOR_LEFT_SIDE         = 15,
	HALF_DOOR_RIGHT_SIDE        = 16,
	HALF_WALL_LEFT_SIDE         = 17,
	HALF_WALL_RIGHT_SIDE        = 18,
	BUTTON_FACING_LU            = 19,
	BUTTON_FACING_RD            = 20,
	TORCH_FACING_LU             = 21,
	TORCH_FACING_RD             = 22,
	LEVER_FACING_LU             = 23,
	LEVER_FACTING_RD            = 24,
	BARS                        = 25,
	TORCH_DOUBLE_SIDED          = 26,
	GATE                        = 27,
	MESSAGE                     = 28,
	SECRET_DOOR                 = 29,
	NICHE_FACING_LU             = 30,
	NICHE_FACING_RD             = 31,
	KEYHOLE_WALL                = 32,
	STANDARD_DOOR_BOX_STYLE     = 33,
};

enum class gcx_marker_t
{
	NONE                      = 0,
	STAIRS_UP                 = 1,
	STAIRS_DOWN               = 2,
	NPC                       = 3,
	TELEPORT_IN               = 4,
	TELEPORT_OUT              = 5,
	PIT_OPEN                  = 7,
	DEATH                     = 8,
	START                     = 9,
	EXIT                      = 10,
	TURNTABLE                 = 11,
	TREASURE_CHEST_OPEN       = 12,
	KEY                       = 13,
	MONSTER                   = 14,
	SWITCH                    = 15,
	FOUNTAIN                  = 16,
	SAVE_POINT                = 17,
	TARGET                    = 18,
	PRESSURE                  = 19,
	PENTAGRAM                 = 20,
	ELEVATOR                  = 21,
	ZAP                       = 22,
	UNKNOWN                   = 23,
	EVENT                     = 24,
	MESSAGE                   = 25,
	LADDER_UP                 = 26,
	LADDER_DOWN               = 27,
	BLOCK_EDGE_HORIZONTAL     = 28, // msub specifies edge style.
	BLOCK_EDGE_VERTICAL       = 29, // msub specifies edge style.
	SHAPE_CROSS               = 41,
	BLOCK_EDGE_DIAGONAL_LEFT  = 42, // '\' msub specifies edge style.
	BLOCK_EDGE_DIAGONAL_RIGHT = 43, // '/' msub specifies edge style.
	LADDER_TWO_WAYS           = 44,
	TREASURE_CHEST_CLOSED     = 46,
	TREASURE_CHEST_TRAPPED    = 47,
	TREASURE_CHEST_LOCKED     = 48,
	ORE                       = 49,
	PIT_COVERED               = 50,
	PIT_TRAPPED               = 51,
	WELL                      = 52,
	TRIANGLE                  = 53,
	SMALL_SQUARE              = 54,
	SQUARE                    = 55,
	SMALL_CIRCLE              = 56,
	CIRCLE                    = 57,
	DIAMOND                   = 58,
	EMERALD                   = 59,
	RUBY                      = 60,
	CRYSTAL                   = 61,
	ARROW_UP                  = 62,
	ARROW_RIGHT               = 63,
	ARROW_DOWN                = 64,
	ARROW_LEFT                = 65,
	SACK                      = 66,
	MAP                       = 67,
	PURSE                     = 68,
	BARREL                    = 69,
	RAMP_UP                   = 70,
	RAMP_DOWN                 = 71,
	BOULDER                   = 72,
	STONE                     = 73,
	PRESSURE_PLATE_WITH_STONE = 74,
	ARROW_LEFT_AND_RIGHT      = 78,
	ARROW_UP_AND_DOWN         = 79,
	ARROW_DIAGONALLY_UL       = 80,
	ARROW_DIAGONALLY_UR       = 81,
	ARROW_DIAGONALLY_DR       = 82,
	ARROW_DIAGONALLY_DL       = 83,
	TREE                      = 97,
	SHOP                      = 98,
	BED                       = 99,
	TAVERN                    = 100,
	HEALTH                    = 101,
	MOVEABLE_BLOCK            = 102,
	TRAINER                   = 103,
	SKULL                     = 104,
	BONES                     = 105,
	BOAT                      = 106,
	BRIDGE                    = 107,
	SIGNPOST                  = 108,
	PILLAR                    = 109,
	ARMOR                     = 110,
	GRAVE                     = 111,
	STATUE                    = 112,
	ARROW_U_THEN_L            = 117,
	ARROW_U_THEN_R            = 118,
	ARROW_D_THEN_L            = 119,
	ARROW_D_THEN_R            = 120,
	ARROW_L_THEN_U            = 121,
	ARROW_R_THEN_U            = 122,
	ARROW_L_THEN_D            = 123,
	ARROW_R_THEN_D            = 124,
	WEAPONS                   = 125,
	BOOTS                     = 126,
	ALTAR                     = 127,
	FOOD                      = 128,
	CORNER                    = 129, // msub specifies the corner style. 0 = North-west Quadrant, 1	= North East, 2 = South West,	3 = South East.
	SCROLL                    = 130,
	BOOK                      = 131,
	HARVEST_PLANT             = 132,
	TIMBER_PILE               = 133,
	DOORWAY                   = 134,
};

enum class gcx_terrain_t
{
#define DECL_GCT( _name_ , _value_ ) _name_ = _value_,
#define DECL_CUSTOM( _name_ , _file_ )
#include "EGGcxCustomTiles.items"

	CUSTOM_START    = 10000,
#define DECL_GCT( _name_ , _value_ )
#define DECL_CUSTOM( _name_ , _file_ ) CUSTOM_##_name_,
#include "EGGcxCustomTiles.items"
};

enum class gcx_dark_effect_t
{
	NONE = 0,
	DARK = 1,
};

enum class gcx_ceiling_t
{
	NONE = 0,
	HAS_CEILING = 1,
};