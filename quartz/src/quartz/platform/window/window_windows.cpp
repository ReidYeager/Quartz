
#include "quartz/defines.h"
#include "quartz/platform/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/window/window.h"

#ifdef QTZ_PLATFORM_WIN32
namespace Quartz
{

#define QTZ_WIN32_WINDOW_CLASS_NAME "QuartzWin32WindowClass"
LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam);
class WindowWin32* thisWindow;

class WindowWin32 : public Window
{
  friend LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam);

public:
  QuartzResult Init() override;
  void Shutdown() override;
  void PollEvents() override;

private:
  QuartzResult Register();
  QuartzResult CreateWindow();
  QuartzResult RegisterInput();
}; // class WindowWin32

Window* CreateWindow()
{
  Window* win = new WindowWin32();
  if (win->Init())
  {
    QTZ_ERROR("Failed to create a win32 window");
    return nullptr;
  }

  return win;
}

// ============================================================
// Init
// ============================================================

QuartzResult WindowWin32::Init()
{
  QTZ_ATTEMPT(Register());
  QTZ_ATTEMPT(CreateWindow());
  QTZ_ATTEMPT(RegisterInput());

  // Set visibility
  ShowWindow(m_platformInfo.hwnd, SW_SHOW);

  QTZ_INFO("Window init");
  thisWindow = this;

  return Quartz_Success;
}

QuartzResult WindowWin32::Register()
{
  m_platformInfo.hinstance = GetModuleHandleA(0);

  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = Win32InputCallback;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = m_platformInfo.hinstance;
  wc.hIcon = LoadIcon(m_platformInfo.hinstance, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = QTZ_WIN32_WINDOW_CLASS_NAME;
  wc.lpszMenuName = NULL;

  int x = RegisterClassA(&wc);

  if (x == 0)
  {
    QTZ_ERROR("Failed to register win32 window");
    return Quartz_Failure;
  }
  return Quartz_Success;
}

QuartzResult WindowWin32::CreateWindow()
{
  uint32_t resizability = (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX); // * canResize

  uint32_t windowStyle = resizability | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  uint32_t windowExStyle = WS_EX_APPWINDOW;

  // Adjust extents so the canvas matches the input extents
  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);
  uint32_t adjustedWidth = m_width + borderRect.right - borderRect.left;
  uint32_t adjustedHeight = m_height + borderRect.bottom - borderRect.top;

  m_platformInfo.hwnd = CreateWindowExA(
    windowExStyle,
    QTZ_WIN32_WINDOW_CLASS_NAME,
    m_title,
    windowStyle,
    50, // X screen position
    50, // Y screen position
    adjustedWidth,
    adjustedHeight,
    0,
    0,
    m_platformInfo.hinstance,
    0);

  if (m_platformInfo.hwnd == 0)
  {
    return Quartz_Failure;
  }

  return Quartz_Success;
}

QuartzResult WindowWin32::RegisterInput()
{
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT) 0x02)
#endif

  RAWINPUTDEVICE inputDevice;
  inputDevice.usUsagePage = HID_USAGE_PAGE_GENERIC;
  inputDevice.usUsage = HID_USAGE_GENERIC_MOUSE;
  inputDevice.dwFlags = RIDEV_INPUTSINK;
  inputDevice.hwndTarget = m_platformInfo.hwnd;
  if (!RegisterRawInputDevices(&inputDevice, 1, sizeof(inputDevice)))
  {
    return Quartz_Failure;
  }

  return Quartz_Success;
}

// ============================================================
// Update
// ============================================================

void WindowWin32::PollEvents()
{
  MSG message;
  uint32_t allowance = 100; // Limit number of messages allowed to process per call
  m_input.UpdateState();
  while (allowance > 0 && PeekMessageA(&message, m_platformInfo.hwnd, 0, 0, PM_REMOVE))
  {
    allowance--;
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam)
{
  if (thisWindow == nullptr)
    return DefWindowProcA(hwnd, message, wparam, lparam);

  switch (message)
  {
  case WM_CLOSE:
  {
    EventWindowClose e;
    thisWindow->m_shouldClose = true;
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_SIZE:
  {
    bool wasMinimized = thisWindow->m_minimized;
    thisWindow->m_minimized = wparam == SIZE_MINIMIZED;
    if (wparam == SIZE_MINIMIZED)
    {
      EventWindowMinimize e(true);
      thisWindow->m_eventCallbackFunction(e);
    }
    else
    {
      if (wasMinimized)
      {
        EventWindowMinimize e(false);
        thisWindow->m_eventCallbackFunction(e);
      }

      uint32_t w = LOWORD(lparam);
      uint32_t h = HIWORD(lparam);
      thisWindow->m_width = w;
      thisWindow->m_height = h;
      EventWindowResize e(w, h);
      thisWindow->m_eventCallbackFunction(e);
    }
  } return 0;
  case WM_KILLFOCUS:
  case WM_SETFOCUS:
  {
    EventWindowFocus e(message == WM_SETFOCUS);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOVE:
  {
    int32_t x = LOWORD(lparam);
    int32_t y = HIWORD(lparam);
    EventWindowMove e(x, y);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;

  // Keyboard
  // ===============
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  {
    WPARAM keycode = wparam;
    UINT scancode = (lparam & 0x00ff0000) >> 16;
    int isExtented = (lparam & 0x01000000) != 0;

    switch (keycode)
    {
    case VK_SHIFT: keycode = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX); break;
    case VK_CONTROL: keycode = isExtented ? VK_RCONTROL : VK_LCONTROL; break;
    case VK_MENU: keycode = isExtented ? VK_RMENU : VK_LMENU; break;
    default: break;
    }

    // NOTE : Want to separate input state update and input event call?
    thisWindow->m_input.HandlePress(PlatformKeyToQuartzInputCode[keycode]);
    EventKeyPressed e(PlatformKeyToQuartzInputCode[keycode]);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    WPARAM keycode = wparam;
    UINT scancode = (lparam & 0x00ff0000) >> 16;
    int isExtented = (lparam & 0x01000000) != 0;

    switch (keycode)
    {
    case VK_SHIFT: keycode = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX); break;
    case VK_CONTROL: keycode = isExtented ? VK_RCONTROL : VK_LCONTROL; break;
    case VK_MENU: keycode = isExtented ? VK_RMENU : VK_LMENU; break;
    default: break;
    }

    // NOTE : Want to separate input state update and input event call?
    thisWindow->m_input.HandleRelease(PlatformKeyToQuartzInputCode[keycode]);
    EventKeyReleased e(PlatformKeyToQuartzInputCode[keycode]);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;

  // Mouse
  // ===============
  case WM_MOUSEMOVE:
  {
    // NOTE : Want to separate input state update and input event call?
    int32_t x = GET_X_LPARAM(lparam);
    int32_t y = GET_Y_LPARAM(lparam);
    thisWindow->m_input.HandleMouseMove({x, y});
    EventMouseMove e(x, y);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOUSEWHEEL:
  {
    // NOTE : Want to separate input state update and input event call?
    int32_t y = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    thisWindow->m_input.HandleMouseScroll(Vec2I{ 0, y });
    EventMouseScroll e(0, y);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOUSEHWHEEL:
  {
    // NOTE : Want to separate input state update and input event call?
    int32_t x = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    thisWindow->m_input.HandleMouseScroll(Vec2I{ x, 0 });
    EventMouseScroll e(x, 0);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
  case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
  case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
  case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
  {
    uint32_t button = 0;
    switch (message)
    {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: button = 0; break;
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: button = 1; break;
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: button = 2; break;
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: button = GET_XBUTTON_WPARAM(wparam) + 2; break;
    default: break;
    }
    // NOTE : Want to separate input state update and input event call?
    thisWindow->m_input.HandleMousePress(button);
    EventMouseButtonPress e(button);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
  case WM_XBUTTONUP:
  {
    uint32_t button = 0;
    switch (message)
    {
    case WM_LBUTTONUP: button = 0; break;
    case WM_RBUTTONUP: button = 1; break;
    case WM_MBUTTONUP: button = 2; break;
    case WM_XBUTTONUP: button = GET_XBUTTON_WPARAM(wparam) + 2; break;
    default: break;
    }
    // NOTE : Want to separate input state update and input event call?
    thisWindow->m_input.HandleMouseRelease(button);
    EventMouseButtonRelease e(button);
    thisWindow->m_eventCallbackFunction(e);
  } return 0;
  default: return DefWindowProcA(hwnd, message, wparam, lparam);
  }
}

// ============================================================
// Shutdown
// ============================================================

void WindowWin32::Shutdown()
{
  DestroyWindow(m_platformInfo.hwnd);
  UnregisterClassA(QTZ_WIN32_WINDOW_CLASS_NAME, m_platformInfo.hinstance);
  QTZ_INFO("Window shutdown");
}

} // namespace Quartz
#endif // QTZ_PLATFORM_WIN32
