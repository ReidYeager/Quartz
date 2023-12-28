#pragma once

#include "quartz/defines.h"
#include "quartz/platform/window/window.h"
#include "quartz/render/render.h"

namespace Quartz {

class Application
{
public:
  Application();
  virtual ~Application() {}

  virtual QuartzResult Init() = 0;
  virtual QuartzResult Update() = 0;
  virtual void Shutdown() = 0;

  inline static Application* Get() { return m_instance; }

  void OnEvent(Event& event);

private:
  QuartzResult CoreInit();
  QuartzResult MainLoop();
  void CoreShutdown();

  static Application* m_instance;
  bool m_isRunning = false;

  Window* m_window;
  Renderer m_renderer;
};

} // namespace Quartz
