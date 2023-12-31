
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
  bool ShouldClose() override;

private:
  QuartzResult Register();
  QuartzResult CreateWindow();
  QuartzResult RegisterInput();

  bool m_shouldClose = false;
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
  uint32_t allowance = 10; // Limit number of messages allowed to process per call
  while (allowance > 0 && PeekMessageA(&message, m_platformInfo.hwnd, 0, 0, PM_REMOVE))
  {
    allowance--;
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_CLOSE:
  {
    QTZ_DEBUG("Quit window");
    thisWindow->m_shouldClose = true;
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

bool WindowWin32::ShouldClose()
{
  return m_shouldClose;
}

} // namespace Quartz
#endif // QTZ_PLATFORM_WIN32
