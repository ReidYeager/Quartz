#pragma once

#include "quartz/defines.h"

namespace Quartz
{

enum ButtonCode
{
  Button_Unknown = 0,

  Key_A, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J, Key_K, Key_L, Key_M,
  Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T, Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,

  Key_0, Key_1, Key_2, Key_3, Key_4,
  Key_5, Key_6, Key_7, Key_8, Key_9,

  Key_Num_0, Key_Num_1, Key_Num_2, Key_Num_3, Key_Num_4,
  Key_Num_5, Key_Num_6, Key_Num_7, Key_Num_8, Key_Num_9,
  Key_Num_Multiply, /* * */
  Key_Num_Add,      /* + */
  Key_Num_Subtract, /* - */
  Key_Num_Decimal,  /* . */
  Key_Num_Divide,   /* / */
  Key_Num_Enter,

  Key_F1,  Key_F2,  Key_F3,  Key_F4,  Key_F5,  Key_F6,  Key_F7,  Key_F8,
  Key_F9,  Key_F10, Key_F11, Key_F12, Key_F13, Key_F14, Key_F15, Key_F16,
  Key_F17, Key_F18, Key_F19, Key_F20, Key_F21, Key_F22, Key_F23, Key_F24,

  Key_Arrow_Up,
  Key_Arrow_Right,
  Key_Arrow_Down,
  Key_Arrow_Left,

  Key_Grave,        /* ` */
  Key_Hyphen,       /* - */
  Key_Equals,       /* = */
  Key_Bracket_L,    /* [ */
  Key_Bracket_R,    /* ] */
  Key_BackSlash,    /* \ */
  Key_Semicolon,    /* ; */
  Key_Apostrophe,   /* ' */
  Key_Comma,        /* , */
  Key_Period,       /* . */
  Key_ForwardSlash, /* / */

  Key_Escape,
  Key_Enter,
  Key_Space,
  Key_Backspace,
  Key_Tab,
  Key_CapsLock,
  Key_NumLock,
  Key_ScrollLock,
  Key_Insert,
  Key_Delete,
  Key_Home,
  Key_End,
  Key_PageUp,
  Key_PageDown,
  Key_Shift_L,
  Key_Shift_R,
  Key_Ctrl_L,
  Key_Ctrl_R,
  Key_Alt_L,
  Key_Alt_R,
  Key_Menu,

  Mouse_0,
  Mouse_1,
  Mouse_2,
  Mouse_3,
  Mouse_4,
  Mouse_Left    = Mouse_0,
  Mouse_Right   = Mouse_1,
  Mouse_Middle  = Mouse_2,
  Mouse_Back    = Mouse_3,
  Mouse_Forward = Mouse_4,

  Button_COUNT
};

} // namespace Quartz