#pragma once

#include "quartz/defines.h"
#include "quartz/events/event.h"
#include "quartz/platform/defines.h"
#include "quartz/platform/input/input.h"

namespace Quartz
{

struct WindowInitInfo
{
  uint32_t width, height;
  int32_t posX, posY;
  const char* title;

  std::function<void(Event&)> eventCallbackFunction;
};

#ifdef QTZ_PLATFORM_WIN32
struct WindowPlatformInfo
{
  HWND hwnd;
  HINSTANCE hinstance;
};
typedef void(*InputCallbackFunction_T)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "quartz/platform/window/window_win32.inl"

#else
struct WindowPlatformInfo
{
  int none;
};
typedef void(*InputCallbackFunction_T)();

#include "quartz/platform/window/window.inl"
s
#endif // QTZ_PLATFORM_WIN32

}
