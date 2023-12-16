#pragma once

#include "quartz/core.h"
#include "quartz/window.h"

#ifdef QTZ_PLATFORM_WIN32

#include <windows.h>
#include <windowsx.h>
#undef CreateWindow

namespace Quartz
{

class Win32Window : public Window
{
public:
  Win32Window(const WindowInitInfo& initInfo);
  virtual ~Win32Window();

  virtual bool GetIsValid() override { return m_platformInfo.hwnd != 0; }

  virtual QuartzResult Update() override;
  inline uint32_t GetWidth() const override { return m_width; }
  inline uint32_t GetHeight() const override { return m_height; }
  inline void SetEventCallback(const fnEventCallback& callback) override { m_fnEventCallback = callback; }

private:
  QuartzResult Init(const WindowInitInfo& initInfo);
  void Shutdown();

  QuartzResult RegisterWindow();
  QuartzResult CreateWindow();
  QuartzResult RegisterInput();
  void SetVisibility(bool visible);

  bool m_isVisible = false;

  std::string m_title;
  uint32_t m_width, m_height;
  uint32_t m_xPos, m_yPos;

  fnEventCallback m_fnEventCallback;

  struct
  {
    HINSTANCE hinstance;
    HWND hwnd;
  } m_platformInfo;

};

} // namespace Quartz

#endif // QTZ_PLATFORM_WIN32
