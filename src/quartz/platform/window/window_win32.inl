class Window
{
friend LRESULT CALLBACK Win32InputCallback(HWND hwnd, uint32_t message, WPARAM wparam, LPARAM lparam);

private:
  uint32_t           m_width;
  uint32_t           m_height;
  char*              m_title;
  WindowPlatformInfo m_platformInfo;
  bool               m_minimized;
  bool               m_shouldClose;

  Input              m_input;
  std::function<void(Event&)> m_eventCallbackFunction;

public:
  QuartzResult Init(WindowInitInfo initInfo);
  QuartzResult PollEvents();
  void         Shutdown();

  inline uint32_t           Width()          const { return m_width; }
  inline uint32_t           Height()         const { return m_height; }
  inline const char*        Title()          const { return m_title; }
  inline WindowPlatformInfo PlatformInfo()   const { return m_platformInfo; }
  inline bool               Minimized()      const { return m_minimized; }
  inline bool               ShouldClose()    const { return m_shouldClose; }
  inline void               MarkForClosure()       { m_shouldClose = true; }

// Win32 specific
// ============================================================
private:
  QuartzResult Register();
  QuartzResult CreatePlatformWindow(WindowInitInfo initInfo);
  QuartzResult RegisterInput();
};