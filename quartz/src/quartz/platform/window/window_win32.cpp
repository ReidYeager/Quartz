
#include "quartz.h"
#include "quartz/platform/window/window.h"

#ifdef QTZ_PLATFORM_WIN32

#undef CreateWindow

namespace Quartz
{

// ==========================================================================================
// Class, defines
// ==========================================================================================

class Win32Window : public Window
{
private:
  bool m_isVisible = false;
  bool m_isOpen = false;

  std::string m_title;
  uint32_t m_width, m_height;
  int32_t m_xPos, m_yPos;

  QuartzEventCallbackFunction m_fnEventCallback;

  WindowPlatformInfo m_platformInfo;

  // ===============
  // Functions
  // ===============
public:
  Win32Window(const WindowInitInfo& initInfo);
  virtual ~Win32Window();

  virtual QuartzResult Update() override;
  virtual void Shutdown() override;

  virtual const WindowPlatformInfo* GetPlatformInfo() override { return &m_platformInfo; }
  virtual inline void SetEventCallback(const QuartzEventCallbackFunction& callback) override
  {
    m_fnEventCallback = callback;
  }

  virtual inline bool IsValid() const override { return m_platformInfo.hwnd != 0; }
  virtual inline uint32_t Width() const override { return m_width; }
  virtual inline uint32_t Height() const override { return m_height; }

private:
  // Initialization ===============
  QuartzResult RegisterWindow();
  QuartzResult CreateWindow();
  QuartzResult RegisterInput();
  void SetVisibility(bool visible);

  // Update ===============
  friend LRESULT CALLBACK Win32InputCallback(HWND _hwnd, uint32_t _message, WPARAM _wparam, LPARAM _lparam);
};

static const char* win32WindowClassName = "QuartzWin32WindowClass";
Win32Window* ActiveWin32Window;
LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam);

// ==========================================================================================
// Initialization
// ==========================================================================================

Window* Window::InitNew(const WindowInitInfo& initInfo)
{
  return new Win32Window(initInfo);
}

Win32Window::Win32Window(const WindowInitInfo& initInfo)
{
  m_title = initInfo.title;
  m_width = initInfo.width;
  m_height = initInfo.height;
  m_xPos = initInfo.xPos;
  m_yPos = initInfo.yPos;

  QTZ_LOG_CORE_INFO("Start creating win32 window \"{}\" ({}, {})", m_title, m_width, m_height);

  if (RegisterWindow())
  {
    QTZ_LOG_CORE_ERROR("Window :: Failed to initialize with windows");
    return;
  }

  if (CreateWindow())
  {
    QTZ_LOG_CORE_ERROR("Window :: Failed to initialize the window");
    return;
  }

  SetVisibility(true);
  m_isOpen = true;

  if (RegisterInput())
  {
    QTZ_LOG_CORE_ERROR("Window :: Failed to initialize raw input");
    return;
  }


  QTZ_LOG_CORE_INFO("Win32 window created");
}

QuartzResult Win32Window::RegisterWindow()
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
  wc.lpszClassName = win32WindowClassName;
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
    win32WindowClassName,
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

// ==========================================================================================
// Shutdown
// ==========================================================================================

Win32Window::~Win32Window()
{
  QTZ_LOG_CORE_INFO("Attempting to shut down window \"{}\"", m_title.c_str());
  Shutdown();
}

void Win32Window::Shutdown()
{
  DestroyWindow(m_platformInfo.hwnd);
  UnregisterClassA(win32WindowClassName, m_platformInfo.hinstance);
}

// ==========================================================================================
// Update
// ==========================================================================================

void Win32Window::SetVisibility(bool visible)
{
  ShowWindow(m_platformInfo.hwnd, visible ? SW_SHOW : SW_HIDE);
  m_isVisible = visible;
}

QuartzResult Win32Window::Update()
{
  // PollWindowEvents ===============
  MSG message;
  ActiveWin32Window = this;
  while (PeekMessageA(&message, m_platformInfo.hwnd, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  ActiveWin32Window = nullptr;

  return Quartz_Success;
}

LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam)
{
  LRESULT result = 0;

  if (ActiveWin32Window == NULL)
  {
    return DefWindowProcA(hwnd, message, wparam, lparam);
  }

  switch (message)
  {
  // Window
  // ===============
  case WM_CLOSE:
  {
    PostQuitMessage(0);
    EventWindowClose e;
    ActiveWin32Window->m_fnEventCallback(e);
    ActiveWin32Window->m_isOpen = false;
  } break;
  case WM_SIZE:
  {
    uint32_t w = LOWORD(lparam);
    uint32_t h = HIWORD(lparam);
    ActiveWin32Window->m_width = w;
    ActiveWin32Window->m_height = h;
    EventWindowResize e(w, h);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
  case WM_KILLFOCUS:
  case WM_SETFOCUS:
  {
    EventWindowFocus e(message == WM_SETFOCUS);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
  case WM_MOVE:
  {
    int32_t x = LOWORD(lparam);
    int32_t y = HIWORD(lparam);
    EventWindowMove e(x, y);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;

  // Keyboard
  // ===============
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  {
    // TODO : Translate the keycode to a custom value
    EventKeyPress e(wparam);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    // TODO : Translate the keycode to a custom value
    EventKeyRelease e(wparam);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;

  // Mouse
  // ===============
  case WM_MOUSEMOVE:
  case WM_NCMOUSEMOVE:
  {
    int32_t x = GET_X_LPARAM(lparam);
    int32_t y = GET_Y_LPARAM(lparam);
    EventMouseMove e(x, y);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
  case WM_MOUSEWHEEL:
  {
    int32_t y = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    EventMouseScroll e(0, y);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
  case WM_MOUSEHWHEEL:
  {
    int32_t x = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
    EventMouseScroll e(x, 0);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
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
    EventMouseButtonPress e(button);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;
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
    EventMouseButtonPress e(button);
    ActiveWin32Window->m_fnEventCallback(e);
  } break;

  // Default
  // ===============
  default:
  {
    result = DefWindowProcA(hwnd, message, wparam, lparam);
  } break;
  }

  return result;
}

} // namespace

#endif // QTZ_PLATFORM_WIN32
