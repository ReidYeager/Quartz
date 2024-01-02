#pragma once

#include "quartz/defines.h"
#include "quartz/platform/defines.h"
#include "quartz/events/event.h"
#include "quartz/platform/input/input.h"
#include <stdint.h>

namespace Quartz
{

class Window* CreateWindow();

#ifdef QTZ_PLATFORM_WIN32
struct WindowPlatformInfo
{
  HWND hwnd;
  HINSTANCE hinstance;
};
#else
struct WindowPlatformInfo
{
  int none;
};
#endif // QTZ_PLATFORM_WIN32

class Window
{
public:
  virtual ~Window() {}

  virtual QuartzResult Init() = 0;
  void SetEventCallbackFunction(const std::function<void(Event&)>& function) { m_eventCallbackFunction = function; }
  virtual void Shutdown() = 0;
  virtual void PollEvents() = 0;
  virtual bool ShouldClose() = 0;
  virtual void MarkForClosure() = 0;

  inline const WindowPlatformInfo* PlatformInfo() { return &m_platformInfo; }
  inline const uint32_t Width() { return m_width; }
  inline const uint32_t Height() { return m_height; }

protected:
  Input m_input;
  std::function<void(Event&)> m_eventCallbackFunction;
  WindowPlatformInfo m_platformInfo;
  //uint32_t m_width = 480, m_height = 360;
  uint32_t m_width = 1280, m_height = 720;
  const char* m_title = "Quartz test (Caveman version)";
};

}
