#pragma once

#include "quartz/defines.h"
#include "quartz/core/application.h"
#include "quartz/core/ecs.h"
#include "quartz/logging/logger.h"
#include "quartz/platform/input/input.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/material.h"

#include <imgui.h> // For use in Application::RenderImgui()

namespace Quartz {

// Entry
void Run();
// Core
void RequestQuit();
double DeltaTime(); // Previous frame's delta time
double Time();      // Total real time
uint32_t WindowWidth();
uint32_t WindowHeight();

} // namespace Quartz

// Entry point
// ============================================================

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
