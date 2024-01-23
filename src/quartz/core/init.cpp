
#include "quartz.h"
#include "quartz/defines.h"
#include "quartz/core/core.h"

#include "quartz/platform/window/window.h"

namespace Quartz
{

// Global variables
// ============================================================

CoreState g_coreState;
uint32_t g_quartzAttemptDepth;

// Quartz entrypoint
// ============================================================

void Run()
{
  QTZ_ATTEMPT_VOID(CoreInit());
  QTZ_ATTEMPT_VOID(CoreMainLoop());
  CoreShutdown();
}

// Declarations
// ============================================================

// Platform
QuartzResult InitWindow();
// Rendering
QuartzResult InitRenderer();
// Engine
void InitClocks();
QuartzResult InitLayers();

// Core
// ============================================================

QuartzResult CoreInit()
{
  Logger::Init();

  QTZ_ATTEMPT(InitWindow());
  QTZ_ATTEMPT(InitRenderer());
  // init resource pools
  // init ecs

  InitClocks();
  QTZ_ATTEMPT(InitLayers());

  QTZ_INFO("Core initialized");

  g_coreState.clientApp = GetClientApplication();
  QTZ_ATTEMPT(g_coreState.clientApp->Init());

  return Quartz_Success;
}

// Platform
// ============================================================

QuartzResult InitWindow()
{
  WindowInitInfo windowInfo = {};
  windowInfo.width = 1280;
  windowInfo.height = 720;
  windowInfo.posX = 1280;
  windowInfo.posY = 50;
  windowInfo.title = "Test new window";
  windowInfo.eventCallbackFunction = EventCallback;

  QTZ_ATTEMPT(g_coreState.mainWindow.Init(windowInfo));

  return Quartz_Success;
}

// Rendering
// ============================================================

QuartzResult InitRenderer()
{
  QTZ_ATTEMPT(g_coreState.renderer.Init(&g_coreState.mainWindow));
  return Quartz_Success;
}

// Engine
// ============================================================

void InitClocks()
{
  g_coreState.time.clocks.engineStart = std::chrono::high_resolution_clock::now();
  g_coreState.time.clocks.frameStart = std::chrono::high_resolution_clock::now();
  // frameEnd set at the start of the main loop
  g_coreState.time.frameIndex = ~0ll; // Will overflow to 0 at start of the first frame
}

QuartzResult InitLayers()
{
  // Init input layer
  // Init debug ui layer

  return Quartz_Success;
}

} // namespace Quartz
