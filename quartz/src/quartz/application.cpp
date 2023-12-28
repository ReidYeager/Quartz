
#include "quartz.h"

#include "quartz/platform/window/window.h"

#include <stdio.h>
#include <memory>

namespace Quartz
{

uint32_t globalFunctionCallAttemptDepth = 0;
Application* Application::m_instance = nullptr;

Application::Application()
{
  if (m_instance != nullptr)
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

  QTZ_LOG_CORE_INFO("Mainloop end. Shutting down");

  m_isRunning = false;
  CoreShutdown();
}

QuartzResult Application::CoreInit()
{
  QTZ_LOG_CORE_INFO("Creating window");
  WindowInitInfo windowInfo;
  windowInfo.title = "Quartz christmas app";
  windowInfo.width = 1280;
  windowInfo.height = 720;
  windowInfo.xPos = 50;
  windowInfo.yPos = 50;
  m_window = Window::InitNew(windowInfo);
  if (!m_window->IsValid())
  {
    QTZ_LOG_CORE_ERROR("Failed to create the window");
    return Quartz_Failure;
  }
  m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

  m_renderer.Init(m_window);

  return Quartz_Success;
}

QuartzResult Application::MainLoop()
{
  while (m_isRunning)
  {
    m_window->Update();

    if (m_window->Width() == 0 || m_window->Height() == 0)
      continue;

    m_renderer.StartFrame();
    m_renderer.Render();
    m_renderer.EndFrame();
  }

  return Quartz_Success;
}

void Application::CoreShutdown()
{
  m_renderer.Shutdown();
  m_window->Shutdown();
}

void Application::OnEvent(Event& event)
{
  //QTZ_LOG_CORE_DEBUG(event);

  if (event.GetType() == Quartz::Event_Window_Resize)
  {
    EventWindowResize* resizeEvent = (EventWindowResize*)(&event);
    m_renderer.Resize(resizeEvent->GetWidth(), resizeEvent->GetHeight());
  }

  if (event.GetType() == Quartz::Event_Window_Close)
  {
    m_isRunning = false;
  }

}

} // namespace Quartz
