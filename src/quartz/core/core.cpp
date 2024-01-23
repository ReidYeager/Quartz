
#include "quartz/core/core.h"

namespace Quartz
{

// Variables
// ============================================================

// Declarations
// ============================================================

// Core
// void RequestQuit(); <-- Declared in quartz.h
// double DeltaTime(); <-- Declared in quartz.h
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
