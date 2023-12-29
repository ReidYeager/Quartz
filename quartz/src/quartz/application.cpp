
#include "quartz.h"

#include "quartz/platform/window/window.h"
#include "quartz/ecs/ecs.h"

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

  QTZ_LOG_CORE_INFO("Mainloop end. Shutting down");

  m_isRunning = false;
  CoreShutdown();
}

struct TestA
{
  uint32_t x, y, z;
};

struct TestB
{
  int x, y;
};

struct TestC
{
  uint32_t x;
};

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

  EcsWorld ecs;
  ComponentId testAId = ecs.DefineComponent("TestA", sizeof(TestA));
  ComponentId testBId = ecs.DefineComponent("TestB", sizeof(TestB));
  ComponentId testCId = ecs.DefineComponent("TestC", sizeof(TestC));
  Entity e = ecs.CreateEntity();
  Entity e2 = ecs.CreateEntity();
  Entity e3 = ecs.CreateEntity();
  Entity e4 = ecs.CreateEntity();

  TestA tmpA = { 0x11111111, 0x22222222, 0x33333333 };
  TestA tmpA2 = { 0x44444444, 0x55555555, 0x66666666 };
  TestA tmpA3 = { 0x77777777, 0x88888888, 0x99999999 };
  TestA tmpA4 = { 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc };

  ecs.SetComponent(e, testAId, &tmpA);
  ecs.SetComponent(e2, testAId, &tmpA2);
  ecs.SetComponent(e3, testAId, &tmpA3);
  ecs.SetComponent(e4, testAId, &tmpA4);

  ecs.AddComponent(e, testBId);
  ecs.AddComponent(e2, testBId);
  ecs.AddComponent(e3, testBId);
  ecs.AddComponent(e4, testBId);

  ecs.AddComponent(e, testCId);
  ecs.AddComponent(e3, testCId);
  ecs.AddComponent(e4, testCId);

  ecs.RemoveComponent(e3, testBId);
  ecs.RemoveComponent(e, testBId);
  ecs.RemoveComponent(e, testCId);

  ecs.PrintArchOwners();

  EcsIterator itr(&ecs, {testAId});
  while (!itr.AtEnd())
  {
    TestA* a = (TestA*)itr.GetComponent(testAId);

    QTZ_LOG_WARNING("{}:{} {:x}, {:x}, {:x}", itr.CurrentEntity(), fmt::ptr(a), a->x, a->y, a->z);

    itr.StepNextElement();
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
  
}

void Application::OnEvent(Event& event)
{
  //QTZ_LOG_CORE_DEBUG(event);

  if (event.GetType() == Quartz::Event_Window_Close)
  {
    m_window->Shutdown();
    m_isRunning = false;
  }

}

} // namespace Quartz
