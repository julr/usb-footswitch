﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Configurator.Model
{
    internal enum ScanCode : byte
    {
        KEY_NONE = 0x00,
        
        KEY_A = 0x04,
        KEY_B = 0x05,
        KEY_C = 0x06,
        KEY_D = 0x07,
        KEY_E = 0x08,
        KEY_F = 0x09,
        KEY_G = 0x0A,
        KEY_H = 0x0B,
        KEY_I = 0x0C,
        KEY_K = 0x0E,
        KEY_L = 0x0F,
        KEY_M = 0x10,
        KEY_N = 0x11,
        KEY_O = 0x12,
        KEY_P = 0x13,
        KEY_Q = 0x14,
        KEY_R = 0x15,
        KEY_S = 0x16,
        KEY_T = 0x17,
        KEY_U = 0x18,
        KEY_V = 0x19,
        KEY_W = 0x1A,
        KEY_X = 0x1B,
        KEY_Y = 0x1C,
        KEY_Z = 0x1D,

        KEY_1 = 0x1E,
        KEY_2 = 0x1F,
        KEY_3 = 0x20,
        KEY_4 = 0x21,
        KEY_5 = 0x22,
        KEY_6 = 0x23,
        KEY_7 = 0x24,
        KEY_8 = 0x25,
        KEY_9 = 0x26,
        KEY_0 = 0x27,

        KEY_ENTER = 0x28,
        KEY_ESC = 0x29,
        KEY_DEL = 0x2A,
        KEY_TAB = 0x2B,
        KEY_SPACE = 0x2C,
        KEY_MINUS = 0x2D,
        KEY_EQUAL = 0x2E,
        KEY_LEFTBRACE = 0x2F,
        KEY_RIGHTBRACE = 0x30,
        KEY_BACKSLASH = 0x31,
        KEY_TILDE = 0x32,
        KEY_SEMICOLON = 0x33,
        KEY_APOSTROPHE = 0x34,
        KEY_GRAVE = 0x35,
        KEY_COMMA = 0x36,
        KEY_DOT = 0x37,
        KEY_SLASH = 0x38,
        KEY_CAPS_LOCK = 0x39,

        KEY_F1 = 0x3A,
        KEY_F2 = 0x3B,
        KEY_F3 = 0x3C,
        KEY_F4 = 0x3D,
        KEY_F5 = 0x3E,
        KEY_F6 = 0x3F,
        KEY_F7 = 0x40,
        KEY_F8 = 0x41,
        KEY_F9 = 0x42,
        KEY_F10 = 0x43,
        KEY_F11 = 0x44,
        KEY_F12 = 0x45,

        KEY_PRINT = 0x46,
        KEY_SCROLL_LOCK = 0x47,
        KEY_PAUSE = 0x48,
        KEY_INSERT = 0x49,
        KEY_HOME = 0x4A,
        KEY_PAGE_UP = 0x4B,
        KEY_DEL_FORWARD = 0x4C,
        KEY_END = 0x4D,
        KEY_PAGE_DOWN = 0x4E,
        KEY_RIGHT_ARROW = 0x4F,
        KEY_LEFT_ARROW = 0x50,
        KEY_DOWN_ARROW = 0x51,
        KEY_UP_ARROW = 0x52,

        KEY_NUM_LOCK = 0x53,
        KEY_NUM_SLASH = 0x54,
        KEY_NUM_ASTERISK = 0x55,
        KEY_NUM_MINUS = 0x56,
        KEY_NUM_PLUS = 0x57,
        KEY_NUM_ENTER = 0x58,
        KEY_NUM_1 = 0x59,
        KEY_NUM_2 = 0x5A,
        KEY_NUM_3 = 0x5B,
        KEY_NUM_4 = 0x5C,
        KEY_NUM_5 = 0x5D,
        KEY_NUM_6 = 0x5E,
        KEY_NUM_7 = 0x5F,
        KEY_NUM_8 = 0x60,
        KEY_NUM_9 = 0x61,
        KEY_NUM_0 = 0x62,
        KEY_NUM_DOT = 0x63,
        KEY_PIPE = 0x64,
        KEY_APPLICATION = 0x65,
        KEY_POWER = 0x66,
        KEY_NUM_EQUAL = 0x67,

        KEY_F13 = 0x68,
        KEY_F14 = 0x69,
        KEY_F15 = 0x6A,
        KEY_F16 = 0x6B,
        KEY_F17 = 0x6C,
        KEY_F18 = 0x6D,
        KEY_F19 = 0x6E,
        KEY_F20 = 0x6F,
        KEY_F21 = 0x70,
        KEY_F22 = 0x71,
        KEY_F23 = 0x72,
        KEY_F24 = 0x73,

        KEY_OPEN = 0x74,
        KEY_HELP = 0x75,
        KEY_PROPS = 0x76,
        KEY_FRONT = 0x77,
        KEY_STOP = 0x78,
        KEY_AGAIN = 0x79,
        KEY_UNDO = 0x7A,
        KEY_CUT = 0x7B,
        KEY_COPY = 0x7C,
        KEY_PASTE = 0x7D,
        KEY_FIND = 0x7E,
        KEY_MUTE = 0x7F,
        KEY_VOLUMEUP = 0x80,
        KEY_VOLUMEDOWN = 0x81,

        KEY_LEFT_CONTROL = 0xE0,
        KEY_LEFT_SHIFT = 0xE1,
        KEY_LEFT_ALT = 0xE2,
        KEY_LEFT_GUI = 0xE3,
        KEY_RIGHT_CONTROL = 0xE4,
        KEY_RIGHT_SHIFT = 0xE5,
        KEY_RIGHT_ALT = 0xE6,
        KEY_RIGHT_GUI = 0xE7,

        KEY_MEDIA_PLAYPAUSE = 0xE8,
        KEY_MEDIA_STOPCD = 0xE9,
        KEY_MEDIA_PREVIOUSSONG = 0xEA,
        KEY_MEDIA_NEXTSONG = 0xEB,
        KEY_MEDIA_EJECTCD = 0xEC,
        KEY_MEDIA_VOLUMEUP = 0xED,
        KEY_MEDIA_VOLUMEDOWN = 0xEE,
        KEY_MEDIA_MUTE = 0xEF,
        KEY_MEDIA_WWW = 0xF0,
        KEY_MEDIA_BACK = 0xF1,
        KEY_MEDIA_FORWARD = 0xF2,
        KEY_MEDIA_STOP = 0xF3,
        KEY_MEDIA_FIND = 0xF4,
        KEY_MEDIA_SCROLLUP = 0xF5,
        KEY_MEDIA_SCROLLDOWN = 0xF6,
        KEY_MEDIA_EDIT = 0xF7,
        KEY_MEDIA_SLEEP = 0xF8,
        KEY_MEDIA_COFFEE = 0xF9,
        KEY_MEDIA_REFRESH = 0xFA,
        KEY_MEDIA_CALC = 0xFB
    }
}
