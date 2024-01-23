
#ifdef QTZ_PLATFORM_WIN32

#include "quartz/defines.h"
#include "quartz/platform/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/input/input.h"
#include "quartz/events/event.h"
#include "quartz/platform/window/window.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Quartz
{

// Global variables
// ============================================================

static Window* g_pollingWindow = nullptr;
#define QTZ_WIN32_WINDOW_CLASS_NAME "QuartzWin32WindowClass"

// Declarations
// ============================================================

LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam);

// Init
// ============================================================

QuartzResult Window::Init(WindowInitInfo initInfo)
{
  QTZ_ATTEMPT(Register());
  QTZ_ATTEMPT(CreatePlatformWindow(initInfo));
  QTZ_ATTEMPT(RegisterInput());

  // Set visibility
  ShowWindow(m_platformInfo.hwnd, SW_SHOW);

  // Set basic information
  m_eventCallbackFunction = initInfo.eventCallbackFunction;
  m_width = initInfo.width;
  m_height = initInfo.height;

  uint32_t titleLength = strlen(initInfo.title) + 1;
  while (m_title == nullptr)
  {
    m_title = (char*)malloc(titleLength);
  }
  memcpy(m_title, initInfo.title, titleLength);

  QTZ_INFO("Window init complete (\"{}\")", m_title);

  return Quartz_Success;
}

QuartzResult Window::Register()
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

  int result = RegisterClassA(&wc);

  if (result == 0)
  {
    QTZ_ERROR("Failed to register win32 window -- err {}", result);
    return Quartz_Failure;
  }
  return Quartz_Success;
}

QuartzResult Window::CreatePlatformWindow(WindowInitInfo initInfo)
{
  uint32_t resizability = (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

  uint32_t windowStyle = resizability | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
  uint32_t windowExStyle = WS_EX_APPWINDOW;

  // Adjust extents so the canvas matches the input extents
  RECT borderRect = { 0, 0, 0, 0 };
  AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);
  uint32_t adjustedWidth = initInfo.width + borderRect.right - borderRect.left;
  uint32_t adjustedHeight = initInfo.height + borderRect.bottom - borderRect.top;

  m_platformInfo.hwnd = CreateWindowExA(
    windowExStyle,
    QTZ_WIN32_WINDOW_CLASS_NAME,
    initInfo.title,
    windowStyle,
    initInfo.posX,
    initInfo.posY,
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

QuartzResult Window::RegisterInput()
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

// Update
// ============================================================

QuartzResult Window::PollEvents()
{
  MSG message;
  uint32_t allowance = 100; // Limit number of messages allowed to process per call
  m_input.UpdateState();

  g_pollingWindow = this;

  while (allowance > 0 && PeekMessageA(&message, m_platformInfo.hwnd, 0, 0, PM_REMOVE))
  {
    allowance--;
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  g_pollingWindow = nullptr;

  return Quartz_Success;
}

LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam)
{
  if (g_pollingWindow == nullptr)
    return DefWindowProcA(hwnd, message, wparam, lparam);

  ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam);

  switch (message)
  {
  case WM_CLOSE:
  {
    g_pollingWindow->m_shouldClose = true;
    EventWindowClose e;
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_SIZE:
  {
    bool wasMinimized = g_pollingWindow->m_minimized;
    g_pollingWindow->m_minimized = wparam == SIZE_MINIMIZED;
    if (wparam == SIZE_MINIMIZED)
    {
      EventWindowMinimize e(true);
      g_pollingWindow->m_eventCallbackFunction(e);
    }
    else
    {
      if (wasMinimized)
      {
        EventWindowMinimize e(false);
        g_pollingWindow->m_eventCallbackFunction(e);
      }

      uint32_t w = LOWORD(lparam);
      uint32_t h = HIWORD(lparam);
      g_pollingWindow->m_width = w;
      g_pollingWindow->m_height = h;
      EventWindowResize e(w, h);
      g_pollingWindow->m_eventCallbackFunction(e);
    }
  } return 0;
  case WM_KILLFOCUS:
  case WM_SETFOCUS:
  {
    EventWindowFocus e(message == WM_SETFOCUS);
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOVE:
  {
    int32_t x = LOWORD(lparam);
    int32_t y = HIWORD(lparam);
    EventWindowMove e(x, y);
    g_pollingWindow->m_eventCallbackFunction(e);
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

    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandlePress(PlatformKeyToQuartzInputCode[keycode]);
    EventKeyPressed e(PlatformKeyToQuartzInputCode[keycode]);
    g_pollingWindow->m_eventCallbackFunction(e);
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

    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleRelease(PlatformKeyToQuartzInputCode[keycode]);
    EventKeyReleased e(PlatformKeyToQuartzInputCode[keycode]);
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;

  // Mouse
  // ===============
  case WM_MOUSEMOVE:
  {
    int32_t x = GET_X_LPARAM(lparam);
    int32_t y = GET_Y_LPARAM(lparam);
    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleMouseMove({x, y});
    EventMouseMove e(x, y);
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOUSEWHEEL:
  {
    int32_t y = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleMouseScroll(Vec2I{ 0, y });
    EventMouseScroll e(0, y);
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;
  case WM_MOUSEHWHEEL:
  {
    int32_t x = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleMouseScroll(Vec2I{ x, 0 });
    EventMouseScroll e(x, 0);
    g_pollingWindow->m_eventCallbackFunction(e);
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
    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleMousePress(button);
    EventMouseButtonPress e(button);
    g_pollingWindow->m_eventCallbackFunction(e);
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
    // TODO : Remove input handling here. Replace with layer that uses the events
    g_pollingWindow->m_input.HandleMouseRelease(button);
    EventMouseButtonRelease e(button);
    g_pollingWindow->m_eventCallbackFunction(e);
  } return 0;
  default: return DefWindowProcA(hwnd, message, wparam, lparam);
  }
}

// Shutdown
// ============================================================

void Window::Shutdown()
{
  std::string title(m_title);

  DestroyWindow(m_platformInfo.hwnd);
  UnregisterClassA(QTZ_WIN32_WINDOW_CLASS_NAME, m_platformInfo.hinstance);
  free(m_title);

  QTZ_INFO("Window shutdown complete (\"{}\")", title.c_str());
}

} // namespace Quartz
#endif // QTZ_PLATFORM_WIN32
