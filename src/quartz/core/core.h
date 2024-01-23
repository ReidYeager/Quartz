#pragma once

#include "quartz/defines.h"

#include "quartz/core/application.h"
#include "quartz/core/ecs.h"
#include "quartz/platform/window/window.h"
#include "quartz/rendering/renderer.h"
#include "quartz/layers/layer_stack.h"

#include <chrono>

namespace Quartz
{

// Types
// ============================================================

struct TimeKeepers
{
  uint64_t frameIndex;
  double delta;

  double deltaSum;

  struct
  {
    std::chrono::steady_clock::time_point engineStart;
    std::chrono::steady_clock::time_point frameStart;
    std::chrono::steady_clock::time_point frameEnd;
  } clocks;
};

struct CoreState
{
  Window mainWindow;
  Renderer renderer;
  TimeKeepers time;
  LayerStack layerStack;
  Diamond::EcsWorld ecsWorld;

  Application* clientApp;
};

// Global variables
// ============================================================

extern CoreState g_coreState;

// Functions
// ============================================================

QuartzResult CoreInit();
QuartzResult CoreMainLoop();
void CoreShutdown();

void EventCallback(Event& e);

} // namespace Quartz
