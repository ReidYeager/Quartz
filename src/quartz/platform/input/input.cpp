
#include "quartz/defines.h"
#include "quartz/platform/input/input.h"

namespace Quartz
{
Input::InputState Input::curState;
Input::InputState Input::prevState;

#ifdef QTZ_PLATFORM_WIN32
const ButtonCode PlatformKeyToQuartzInputCode[256] =
{
// TODO : Add better support for OEM keys
  /*0x00*/ Button_Unknown, /*0x01*/ Mouse_Left,       /*0x02*/ Mouse_Right,      /*0x03*/ Button_Unknown,
  /*0x04*/ Mouse_Middle,   /*0x05*/ Mouse_3,          /*0x06*/ Mouse_4,          /*0x07*/ Button_Unknown,
  /*0x08*/ Key_Backspace,  /*0x09*/ Key_Tab,          /*0x0a*/ Button_Unknown,   /*0x0b*/ Button_Unknown,
  /*0x0c*/ Button_Unknown, /*0x0d*/ Key_Enter,        /*0x0e*/ Button_Unknown,   /*0x0f*/ Button_Unknown,

  /*0x10*/ Button_Unknown, /*0x11*/ Button_Unknown,   /*0x12*/ Button_Unknown,   /*0x13*/ Button_Unknown,
  /*0x14*/ Key_CapsLock,   /*0x15*/ Button_Unknown,   /*0x16*/ Button_Unknown,   /*0x17*/ Button_Unknown,
  /*0x18*/ Button_Unknown, /*0x19*/ Button_Unknown,   /*0x1a*/ Button_Unknown,   /*0x1b*/ Key_Escape,
  /*0x1c*/ Button_Unknown, /*0x1d*/ Button_Unknown,   /*0x1e*/ Button_Unknown,   /*0x1f*/ Button_Unknown,

  /*0x20*/ Key_Space,      /*0x21*/ Key_PageUp,       /*0x22*/ Key_PageDown,     /*0x23*/ Key_End,
  /*0x24*/ Key_Home,       /*0x25*/ Key_Arrow_Left,   /*0x26*/ Key_Arrow_Up,     /*0x27*/ Key_Arrow_Right,
  /*0x28*/ Key_Arrow_Down, /*0x29*/ Button_Unknown,   /*0x2a*/ Button_Unknown,   /*0x2b*/ Button_Unknown,
  /*0x2c*/ Button_Unknown, /*0x2d*/ Key_Insert,       /*0x2e*/ Key_Delete,       /*0x2f*/ Button_Unknown,

  /*0x30*/ Key_0,          /*0x31*/ Key_1,            /*0x32*/ Key_2,            /*0x33*/ Key_3,
  /*0x34*/ Key_4,          /*0x35*/ Key_5,            /*0x36*/ Key_6,            /*0x37*/ Key_7,
  /*0x38*/ Key_8,          /*0x39*/ Key_9,            /*0x3a*/ Button_Unknown,   /*0x3b*/ Button_Unknown,
  /*0x3c*/ Button_Unknown, /*0x3d*/ Button_Unknown,   /*0x3e*/ Button_Unknown,   /*0x3f*/ Button_Unknown,

  /*0x40*/ Button_Unknown, /*0x41*/ Key_A,            /*0x42*/ Key_B,            /*0x43*/ Key_C,
  /*0x44*/ Key_D,          /*0x45*/ Key_E,            /*0x46*/ Key_F,            /*0x47*/ Key_G,
  /*0x48*/ Key_H,          /*0x49*/ Key_I,            /*0x4a*/ Key_J,            /*0x4b*/ Key_K,
  /*0x4c*/ Key_L,          /*0x4d*/ Key_M,            /*0x4e*/ Key_N,            /*0x4f*/ Key_O,

  /*0x50*/ Key_P,          /*0x51*/ Key_Q,            /*0x52*/ Key_R,            /*0x53*/ Key_S,
  /*0x54*/ Key_T,          /*0x55*/ Key_U,            /*0x56*/ Key_V,            /*0x57*/ Key_W,
  /*0x58*/ Key_X,          /*0x59*/ Key_Y,            /*0x5a*/ Key_Z,            /*0x5b*/ Button_Unknown,
  /*0x5c*/ Button_Unknown, /*0x5d*/ Key_Menu,         /*0x5e*/ Button_Unknown,   /*0x5f*/ Button_Unknown,

  /*0x60*/ Key_Num_0,      /*0x61*/ Key_Num_1,        /*0x62*/ Key_Num_2,        /*0x63*/ Key_Num_3,
  /*0x64*/ Key_Num_4,      /*0x65*/ Key_Num_5,        /*0x66*/ Key_Num_6,        /*0x67*/ Key_Num_7,
  /*0x68*/ Key_Num_8,      /*0x69*/ Key_Num_9,        /*0x6a*/ Key_Num_Multiply, /*0x6b*/ Key_Num_Add,
  /*0x6c*/ Button_Unknown, /*0x6d*/ Key_Num_Subtract, /*0x6e*/ Key_Num_Decimal,  /*0x6f*/ Key_Num_Divide,

  /*0x70*/ Key_F1,         /*0x71*/ Key_F2,           /*0x72*/ Key_F3,           /*0x73*/ Key_F4,
  /*0x74*/ Key_F5,         /*0x75*/ Key_F6,           /*0x76*/ Key_F7,           /*0x77*/ Key_F8,
  /*0x78*/ Key_F9,         /*0x79*/ Key_F10,          /*0x7a*/ Key_F11,          /*0x7b*/ Key_F12,
  /*0x7c*/ Key_F13,        /*0x7d*/ Key_F14,          /*0x7e*/ Key_F15,          /*0x7f*/ Key_F16,

  /*0x80*/ Key_F17,        /*0x81*/ Key_F18,          /*0x82*/ Key_F19,          /*0x83*/ Key_F20,
  /*0x84*/ Key_F21,        /*0x85*/ Key_F22,          /*0x86*/ Key_F23,          /*0x87*/ Key_F24,
  /*0x88*/ Button_Unknown, /*0x89*/ Button_Unknown,   /*0x8a*/ Button_Unknown,   /*0x8b*/ Button_Unknown,
  /*0x8c*/ Button_Unknown, /*0x8d*/ Button_Unknown,   /*0x8e*/ Button_Unknown,   /*0x8f*/ Button_Unknown,

  /*0x90*/ Key_NumLock,    /*0x91*/ Key_ScrollLock,   /*0x92*/ Button_Unknown,   /*0x93*/ Button_Unknown,
  /*0x94*/ Button_Unknown, /*0x95*/ Button_Unknown,   /*0x96*/ Button_Unknown,   /*0x97*/ Button_Unknown,
  /*0x98*/ Button_Unknown, /*0x99*/ Button_Unknown,   /*0x9a*/ Button_Unknown,   /*0x9b*/ Button_Unknown,
  /*0x9c*/ Button_Unknown, /*0x9d*/ Button_Unknown,   /*0x9e*/ Button_Unknown,   /*0x9f*/ Button_Unknown,

  /*0xa0*/ Key_Shift_L,    /*0xa1*/ Key_Shift_R,      /*0xa2*/ Key_Ctrl_L,       /*0xa3*/ Key_Ctrl_R,
  /*0xa4*/ Key_Alt_L,      /*0xa5*/ Key_Alt_R,        /*0xa6*/ Button_Unknown,   /*0xa7*/ Button_Unknown,
  /*0xa8*/ Button_Unknown, /*0xa9*/ Button_Unknown,   /*0xaa*/ Button_Unknown,   /*0xab*/ Button_Unknown,
  /*0xac*/ Button_Unknown, /*0xad*/ Button_Unknown,   /*0xae*/ Button_Unknown,   /*0xaf*/ Button_Unknown,

  /*0xb0*/ Button_Unknown, /*0xb1*/ Button_Unknown,   /*0xb2*/ Button_Unknown,   /*0xb3*/ Button_Unknown,
  /*0xb4*/ Button_Unknown, /*0xb5*/ Button_Unknown,   /*0xb6*/ Button_Unknown,   /*0xb7*/ Button_Unknown,
  /*0xb8*/ Button_Unknown, /*0xb9*/ Button_Unknown,   /*0xba*/ Key_Semicolon,    /*0xbb*/ Key_Equals,
  /*0xbc*/ Key_Comma,      /*0xbd*/ Key_Hyphen,       /*0xbe*/ Key_Period,       /*0xbf*/ Key_ForwardSlash,

  /*0xc0*/ Key_Grave,      /*0xc1*/ Button_Unknown,   /*0xc2*/ Button_Unknown,   /*0xc3*/ Button_Unknown,
  /*0xc4*/ Button_Unknown, /*0xc5*/ Button_Unknown,   /*0xc6*/ Button_Unknown,   /*0xc7*/ Button_Unknown,
  /*0xc8*/ Button_Unknown, /*0xc9*/ Button_Unknown,   /*0xca*/ Button_Unknown,   /*0xcb*/ Button_Unknown,
  /*0xcc*/ Button_Unknown, /*0xcd*/ Button_Unknown,   /*0xce*/ Button_Unknown,   /*0xcf*/ Button_Unknown,

  /*0xd0*/ Button_Unknown, /*0xd1*/ Button_Unknown,   /*0xd2*/ Button_Unknown,   /*0xd3*/ Button_Unknown,
  /*0xd4*/ Button_Unknown, /*0xd5*/ Button_Unknown,   /*0xd6*/ Button_Unknown,   /*0xd7*/ Button_Unknown,
  /*0xd8*/ Button_Unknown, /*0xd9*/ Button_Unknown,   /*0xda*/ Button_Unknown,   /*0xdb*/ Key_Bracket_L,
  /*0xdc*/ Key_BackSlash,  /*0xdd*/ Key_Bracket_R,    /*0xde*/ Key_Apostrophe,   /*0xdf*/ Button_Unknown,

  /*0xe0*/ Button_Unknown, /*0xe1*/ Button_Unknown,   /*0xe2*/ Button_Unknown,   /*0xe3*/ Button_Unknown,
  /*0xe4*/ Button_Unknown, /*0xe5*/ Button_Unknown,   /*0xe6*/ Button_Unknown,   /*0xe7*/ Button_Unknown,
  /*0xe8*/ Button_Unknown, /*0xe9*/ Button_Unknown,   /*0xea*/ Button_Unknown,   /*0xeb*/ Button_Unknown,
  /*0xec*/ Button_Unknown, /*0xed*/ Button_Unknown,   /*0xee*/ Button_Unknown,   /*0xef*/ Button_Unknown,

  /*0xf0*/ Button_Unknown, /*0xf1*/ Button_Unknown,   /*0xf2*/ Button_Unknown,   /*0xf3*/ Button_Unknown,
  /*0xf4*/ Button_Unknown, /*0xf5*/ Button_Unknown,   /*0xf6*/ Button_Unknown,   /*0xf7*/ Button_Unknown,
  /*0xf8*/ Button_Unknown, /*0xf9*/ Button_Unknown,   /*0xfa*/ Button_Unknown,   /*0xfb*/ Button_Unknown,
  /*0xfc*/ Button_Unknown, /*0xfd*/ Button_Unknown,   /*0xfe*/ Button_Unknown,   /*0xff*/ Button_Unknown
};
#else
const ButtonCode PlatformKeyToQuartzInputCode[256] = {};
#endif // QTZ_PLATFORM_WIN32

} // namespace Quartz
