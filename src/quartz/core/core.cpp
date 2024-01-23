
#include "quartz/core/core.h"

namespace Quartz
{

// Variables
// ============================================================

// Declarations
// ============================================================

// Core
/*
V-- Declared in quartz.h --V
  void RequestQuit();
  double DeltaTime(); // Previous frame's delta time
  double Time();      // Total real time
  uint32_t WindowWidth();
  uint32_t WindowHeight();
^-- Declared in quartz.h --^
*/
// Events
// void EventCallback(Event& e) <-- Declared in quartz/core/core.h

// Core
// ============================================================

void RequestQuit()
{
  g_coreState.mainWindow.MarkForClosure();
}

double DeltaTime()
{
  return g_coreState.time.delta;
}

double Time()
{
  auto diff = g_coreState.time.clocks.frameEnd - g_coreState.time.clocks.engineStart;
  return std::chrono::duration_cast<std::chrono::microseconds>(diff).count() * 0.000001;
}

uint32_t WindowWidth()
{
  return g_coreState.mainWindow.Width();
}

uint32_t WindowHeight()
{
  return g_coreState.mainWindow.Height();
}

// Events
// ============================================================

void EventCallback(Event& e)
{
  if (e.GetType() == Event_Window_Resize)
  {
    g_coreState.renderer.Resize(g_coreState.mainWindow.Width(), g_coreState.mainWindow.Height());
  }

  for (auto iterator = g_coreState.layerStack.EndIterator(); iterator != g_coreState.layerStack.BeginIterator();)
  {
    iterator--;
    (*iterator)->OnEvent(e);
  }
}

} // namespace Quartz
