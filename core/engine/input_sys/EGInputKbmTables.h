///////////////////////////////////////////////////////////////////////////////
// InputKbmTables - Tables for mapping keyboard an mouse system data to a
// game compatible format.
//
// (c) 2015 Beem Software
///////////////////////////////////////////////////////////////////////////////
static const struct egInputKbSysToKeyMap
{
	BYTE             SysKey;
	eg_kbm_btn GameKey; 
} 
InputKbmDevice_KbSysToKeyMap[] =
{
	{ VK_ESCAPE , KBM_BTN_ESCAPE          },
	{ '1' , KBM_BTN_1               },
	{ '2' , KBM_BTN_2               },
	{ '3' , KBM_BTN_3               },
	{ '4' , KBM_BTN_4               },
	{ '5' , KBM_BTN_5               },
	{ '6' , KBM_BTN_6               },
	{ '7' , KBM_BTN_7               },
	{ '8' , KBM_BTN_8               },
	{ '9' , KBM_BTN_9               },
	{ '0' , KBM_BTN_0               },
	{ VK_OEM_MINUS , KBM_BTN_MINUS           },    /* - on main keyboard */
	{ VK_OEM_PLUS , KBM_BTN_EQUALS          },
	{ VK_BACK , KBM_BTN_BACK            },    /* backspace */
	{ VK_TAB , KBM_BTN_TAB             },
	{ 'Q' , KBM_BTN_Q               },
	{ 'W' , KBM_BTN_W               },
	{ 'E' , KBM_BTN_E               },
	{ 'R' , KBM_BTN_R               },
	{ 'T' , KBM_BTN_T               },
	{ 'Y' , KBM_BTN_Y               },
	{ 'U' , KBM_BTN_U               },
	{ 'I' , KBM_BTN_I               },
	{ 'O' , KBM_BTN_O               },
	{ 'P' , KBM_BTN_P               },
	{ VK_OEM_4 , KBM_BTN_LBRACKET        },
	{ VK_OEM_6 , KBM_BTN_RBRACKET        },
	{ VK_RETURN , KBM_BTN_RETURN          },    /* Enter on main keyboard */
	{ VK_CONTROL , KBM_BTN_LCONTROL        },
	{ 'A' , KBM_BTN_A               },
	{ 'S' , KBM_BTN_S               },
	{ 'D' , KBM_BTN_D               },
	{ 'F' , KBM_BTN_F               },
	{ 'G' , KBM_BTN_G               },
	{ 'H' , KBM_BTN_H               },
	{ 'J' , KBM_BTN_J               },
	{ 'K' , KBM_BTN_K               },
	{ 'L' , KBM_BTN_L               },
	{ VK_OEM_1 , KBM_BTN_SEMICOLON       },
	{ VK_OEM_7 , KBM_BTN_APOSTROPHE      },
	{ VK_OEM_3 , KBM_BTN_GRAVE           },    /* accent grave */
	{ VK_SHIFT , KBM_BTN_LSHIFT          },
	{ VK_OEM_5 , KBM_BTN_BACKSLASH       },
	{ 'Z' , KBM_BTN_Z               },
	{ 'X' , KBM_BTN_X               },
	{ 'C' , KBM_BTN_C               },
	{ 'V' , KBM_BTN_V               },
	{ 'B' , KBM_BTN_B               },
	{ 'N' , KBM_BTN_N               },
	{ 'M' , KBM_BTN_M               },
	{ VK_OEM_COMMA , KBM_BTN_COMMA           },
	{ VK_OEM_PERIOD , KBM_BTN_PERIOD          },    /* . on main keyboard */
	{ VK_OEM_2 , KBM_BTN_SLASH           },    /* / on main keyboard */
	{ VK_MULTIPLY , KBM_BTN_MULTIPLY        },    /* * on numeric keypad */
	{ VK_MENU , KBM_BTN_LMENU           },    /* left Alt */
	{ VK_SPACE , KBM_BTN_SPACE           },
	{ VK_CAPITAL , KBM_BTN_CAPITAL         },
	{ VK_F1  , KBM_BTN_F1              },
	{ VK_F2  , KBM_BTN_F2              },
	{ VK_F3  , KBM_BTN_F3              },
	{ VK_F4  , KBM_BTN_F4              },
	{ VK_F5  , KBM_BTN_F5              },
	{ VK_F6  , KBM_BTN_F6              },
	{ VK_F7  , KBM_BTN_F7              },
	{ VK_F8  , KBM_BTN_F8              },
	{ VK_F9  , KBM_BTN_F9              },
	{ VK_F10 , KBM_BTN_F10             },
	{ VK_NUMLOCK , KBM_BTN_NUMLOCK         },
	{ VK_SCROLL , KBM_BTN_SCROLL          },    /* Scroll Lock */
	{ VK_NUMPAD7 , KBM_BTN_NUMPAD7         },
	{ VK_NUMPAD8 , KBM_BTN_NUMPAD8         },
	{ VK_NUMPAD9 , KBM_BTN_NUMPAD9         },
	{ VK_SUBTRACT , KBM_BTN_SUBTRACT        },    /* - on numeric keypad */
	{ VK_NUMPAD4 , KBM_BTN_NUMPAD4         },
	{ VK_NUMPAD5 , KBM_BTN_NUMPAD5         },
	{ VK_NUMPAD6 , KBM_BTN_NUMPAD6         },
	{ VK_ADD , KBM_BTN_ADD             },    /* + on numeric keypad */
	{ VK_NUMPAD1 , KBM_BTN_NUMPAD1         },
	{ VK_NUMPAD2 , KBM_BTN_NUMPAD2         },
	{ VK_NUMPAD3 , KBM_BTN_NUMPAD3         },
	{ VK_NUMPAD0 , KBM_BTN_NUMPAD0         },
	{ VK_DECIMAL , KBM_BTN_DECIMAL         },    /* . on numeric keypad */
	{ VK_OEM_102 , KBM_BTN_OEM_102         },    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
	{ VK_F11 , KBM_BTN_F11             },
	{ VK_F12 , KBM_BTN_F12             },
	{ VK_F13 , KBM_BTN_F13             },    /*                     (NEC PC98) */
	{ VK_F14 , KBM_BTN_F14             },    /*                     (NEC PC98) */
	{ VK_F15 , KBM_BTN_F15             },    /*                     (NEC PC98) */
	{ VK_KANA , KBM_BTN_KANA            },    /* (Japanese keyboard)            */
	{ 0 , KBM_BTN_ABNT_C1         },    /* /? on Brazilian keyboard */
	{ VK_CONVERT , KBM_BTN_CONVERT         },    /* (Japanese keyboard)            */
	{ VK_NONCONVERT , KBM_BTN_NOCONVERT       },    /* (Japanese keyboard)            */
	{ 0 , KBM_BTN_YEN             },    /* (Japanese keyboard)            */
	{ 0 , KBM_BTN_ABNT_C2         },    /* Numpad . on Brazilian keyboard */
	{ VK_OEM_NEC_EQUAL , KBM_BTN_NUMPADEQUALS    },    /* = on numeric keypad (NEC PC98) */
	{ 0 , KBM_BTN_PREVTRACK       },    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
	{ 0 , KBM_BTN_AT              },    /*                     (NEC PC98) */
	{ 0 , KBM_BTN_COLON           },    /*                     (NEC PC98) */
	{ 0 , KBM_BTN_UNDERLINE       },    /*                     (NEC PC98) */
	{ VK_KANJI , KBM_BTN_KANJI           },    /* (Japanese keyboard)            */
	{ 0 , KBM_BTN_STOP            },    /*                     (NEC PC98) */
	{ VK_OEM_AX , KBM_BTN_AX              },    /*                     (Japan AX) */
	{ 0 , KBM_BTN_UNLABELED       },    /*                        (J3100) */
	{ 0 , KBM_BTN_NEXTTRACK       },    /* Next Track */
	{ 0 , KBM_BTN_NUMPADENTER     },    /* Enter on numeric keypad */
	{ 0 , KBM_BTN_MUTE            },    /* Mute */
	{ 0 , KBM_BTN_CALCULATOR      },    /* Calculator */
	{ 0 , KBM_BTN_PLAYPAUSE       },    /* Play / Pause */
	{ 0 , KBM_BTN_MEDIASTOP       },    /* Media Stop */
	{ 0 , KBM_BTN_VOLUMEDOWN      },    /* Volume - */
	{ 0 , KBM_BTN_VOLUMEUP        },    /* Volume + */
	{ 0 , KBM_BTN_WEBHOME         },    /* Web home */
	{ 0 , KBM_BTN_NUMPADCOMMA     },    /* , on numeric keypad (NEC PC98) */
	{ VK_DIVIDE , KBM_BTN_DIVIDE          },    /* / on numeric keypad */
	{ VK_SNAPSHOT , KBM_BTN_SYSRQ           },
	{ VK_MENU , KBM_BTN_RMENU           },    /* right Alt */
	{ VK_PAUSE , KBM_BTN_PAUSE           },    /* Pause */
	{ VK_HOME , KBM_BTN_HOME            },    /* Home on arrow keypad */
	{ VK_UP , KBM_BTN_UP              },    /* UpArrow on arrow keypad */
	{ VK_PRIOR , KBM_BTN_PRIOR           },    /* PgUp on arrow keypad */
	{ VK_LEFT , KBM_BTN_LEFT            },    /* LeftArrow on arrow keypad */
	{ VK_RIGHT , KBM_BTN_RIGHT           },    /* RightArrow on arrow keypad */
	{ VK_END , KBM_BTN_END             },    /* End on arrow keypad */
	{ VK_DOWN , KBM_BTN_DOWN            },    /* DownArrow on arrow keypad */
	{ VK_NEXT , KBM_BTN_NEXT            },    /* PgDn on arrow keypad */
	{ VK_INSERT , KBM_BTN_INSERT          },    /* Insert on arrow keypad */
	{ VK_DELETE , KBM_BTN_DELETE          },    /* Delete on arrow keypad */
	{ VK_LWIN , KBM_BTN_LWIN            },    /* Left Windows key */
	{ VK_RWIN , KBM_BTN_RWIN            },    /* Right Windows key */
	{ VK_APPS , KBM_BTN_APPS            },    /* AppMenu key */
	{ 0 , KBM_BTN_POWER           },    /* System Power */
	{ 0 , KBM_BTN_SLEEP           },    /* System Sleep */
	{ 0 , KBM_BTN_WAKE            },    /* System Wake */
	{ 0 , KBM_BTN_WEBSEARCH       },    /* Web Search */
	{ 0 , KBM_BTN_WEBFAVORITES    },    /* Web Favorites */
	{ 0 , KBM_BTN_WEBREFRESH      },    /* Web Refresh */
	{ 0 , KBM_BTN_WEBSTOP         },    /* Web Stop */
	{ 0 , KBM_BTN_WEBFORWARD      },    /* Web Forward */
	{ 0 , KBM_BTN_WEBBACK         },    /* Web Back */
	{ 0 , KBM_BTN_MYCOMPUTER      },    /* My Computer */
	{ 0 , KBM_BTN_MAIL            },    /* Mail */
	{ 0 , KBM_BTN_MEDIASELECT     },    /* Media Select */
	{ 0 , KBM_BTN_COUNT           },    /* Only to count how many keys there are */
};
