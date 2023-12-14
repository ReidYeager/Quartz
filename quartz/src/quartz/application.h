#pragma once

#include "quartz/core.h"

namespace Quartz {

class Application
{
public:
  Application();
  virtual ~Application() {}

  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  inline static Application* Get() { return m_instance; }

private:
  static Application* m_instance;
  bool m_isRunning = false;
};

} // namespace Quartz
