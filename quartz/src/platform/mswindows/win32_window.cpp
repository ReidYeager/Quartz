
#include "quartz.h"
#include "platform/mswindows/win32_window.h"

#ifdef QTZ_PLATFORM_WIN32

namespace Quartz
{

Window* Window::Create(const WindowInitInfo& initInfo)
{
  return new Win32Window(initInfo);
}

Win32Window::Win32Window(const WindowInitInfo& initInfo)
{
  Init(initInfo);
}

Win32Window::~Win32Window()
{
  QTZ_LOG_CORE_INFO("Attempting to shut down window \"{}\"", m_title.c_str());
  Shutdown();
}

LRESULT CALLBACK Win32WindowProcessInputMessage(HWND _hwnd, uint32_t _message, WPARAM _wparam, LPARAM _lparam)
{
  LRESULT result = DefWindowProcA(_hwnd, _message, _wparam, _lparam);
  return result;
}

QuartzResult Win32Window::RegisterWindow()
{
  m_platformInfo.hinstance = GetModuleHandleA(0);

  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = Win32WindowProcessInputMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = m_platformInfo.hinstance;
  wc.hIcon = LoadIcon(m_platformInfo.hinstance, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = "QuartzWin32WindowClass";
  wc.lpszMenuName = NULL;

  int x = RegisterClassA(&wc);

  if (x == 0)
  {
    return Quartz_Failure;
  }

  return Quartz_Success;
}

QuartzResult Win32Window::CreateWindow()
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
    "QuartzWin32WindowClass",
    m_title.c_str(),
    windowStyle,
    m_xPos, // X screen position
    m_yPos, // Y screen position
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

QuartzResult Win32Window::RegisterInput()
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

void Win32Window::SetVisibility(bool visible)
{
  m_isVisible = visible;
  ShowWindow(m_platformInfo.hwnd, visible ? SW_SHOW : SW_HIDE);
}

QuartzResult Win32Window::Init(const WindowInitInfo& initInfo)
{
  m_title = initInfo.title;
  m_width = initInfo.width;
  m_height = initInfo.height;
  m_xPos = initInfo.xPos;
  m_yPos = initInfo.yPos;

  QTZ_LOG_CORE_INFO("Start creating win32 window \"{}\" ({}, {})", m_title, m_width, m_height);

  QTZ_ATTEMPT(RegisterWindow());
  QTZ_ATTEMPT(CreateWindow());
  QTZ_ATTEMPT(RegisterInput());
  SetVisibility(true);

  QTZ_LOG_CORE_INFO("Win32 window created");

  return Quartz_Success;
}

QuartzResult Win32Window::Update()
{
  MSG message;
  while (PeekMessageA(&message, m_platformInfo.hwnd, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return Quartz_Success;
}

void Win32Window::Shutdown()
{

}

} // namespace

#endif // QTZ_PLATFORM_WIN32
