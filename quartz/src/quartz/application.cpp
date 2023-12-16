
#include "quartz.h"

#include "platform/mswindows/win32_window.h"

#include <stdio.h>
#include <memory>

namespace Quartz
{

uint32_t attemptDepth = 0;
Application* Application::m_instance = NULL;

Application::Application()
{
  if (m_instance != NULL)
  {
    QTZ_LOG_CORE_ERROR("Application already exists\n");
    return;
  }
  m_instance = this;
  m_isRunning = false;

  Quartz::Logger logger;
  logger.Init();

  if (CoreInit() != Quartz_Success)
  {
    QTZ_LOG_CORE_ERROR("Failed to initialze Quartz core");
    return;
  }

  m_isRunning = true;
  if (MainLoop() != Quartz_Success)
  {
    QTZ_LOG_CORE_ERROR("Failed during main loop");
    return;
  }

  m_isRunning = false;
  CoreShutdown();

  QTZ_LOG_CORE_INFO("Application init complete");
}

QuartzResult Application::CoreInit()
{
  QTZ_LOG_CORE_INFO("Creating window");
  WindowInitInfo windowInfo;
  windowInfo.title = "Quartz test app";
  windowInfo.width = 1280;
  windowInfo.height = 720;
  m_window = Window::Create();
  if (!m_window->GetIsValid())
  {
    QTZ_LOG_CORE_ERROR("Failed to create the window");
    return Quartz_Failure;
  }

  return Quartz_Success;
}

QuartzResult Application::MainLoop()
{
  while (m_isRunning)
  {
    m_window->Update();
  }

  return Quartz_Success;
}

void Application::CoreShutdown()
{
  Shutdown();
}

} // namespace Quartz
