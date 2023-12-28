#pragma once

#ifdef QTZ_PLATFORM_WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>
#include <windowsx.h>

namespace Quartz
{

struct WindowPlatformInfo
{
  HINSTANCE hinstance;
  HWND hwnd;
};

} // namespace Quartz

#else

namespace Quartz
{

struct WindowPlatformInfo
{
  int noInfo;
};

} // namespace Quartz

#endif
