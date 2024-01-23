#pragma once

#include "quartz/defines.h"
#include "quartz/core/application.h"
#include "quartz/logging/logger.h"
#include "quartz/platform/input/input.h"

#include <imgui.h> // For use in Application::RenderImgui()

namespace Quartz {

// Entry
void Run();
// Core
void RequestQuit();
double DeltaTime();

} // namespace Quartz

extern Quartz::Application* GetClientApplication();

#define QUARTZ_ENTRY(ApplicationClass)                            \
Quartz::Application* GetClientApplication()                       \
{                                                                 \
  static Quartz::Application* clientApp = new ApplicationClass(); \
  return clientApp;                                               \
}                                                                 \
int main()                                                        \
{                                                                 \
  Quartz::Run();                                                  \
  return 0;                                                       \
}
