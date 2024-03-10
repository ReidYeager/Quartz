
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

void Run(QuartzInitInfo initInfo)
{
  QTZ_ATTEMPT_VOID(CoreInit(initInfo));
  QTZ_ATTEMPT_VOID(CoreMainLoop());
  CoreShutdown();
}

// Declarations
// ============================================================

// Platform
QuartzResult InitWindow(QuartzInitInfo initInfo);
// Rendering
QuartzResult InitRenderer();
// Engine
QuartzResult InitEcs();
void InitClocks();
QuartzResult InitLayers();

// Core
// ============================================================

QuartzResult CoreInit(QuartzInitInfo initInfo)
{
  Logger::Init();

  QTZ_ATTEMPT(InitWindow(initInfo));
  QTZ_ATTEMPT(InitRenderer());
  // init resource pools

  QTZ_ATTEMPT(InitEcs());
  QTZ_ATTEMPT(InitLayers());

  QTZ_INFO("Core initialized");

  g_coreState.clientApp = GetClientApplication();
  QTZ_ATTEMPT(g_coreState.clientApp->Init());

  InitClocks();

  return Quartz_Success;
}

// Platform
// ============================================================

QuartzResult InitWindow(QuartzInitInfo initInfo)
{
  WindowInitInfo windowInfo = {};
  windowInfo.width = initInfo.window.extents.width;
  windowInfo.height = initInfo.window.extents.height;
  windowInfo.posX = initInfo.window.position.x;
  windowInfo.posY = initInfo.window.position.y;
  windowInfo.title = initInfo.window.title;
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

QuartzResult InitEcs()
{
  g_coreState.ecsIds.transform  = QuartzDefineComponent(Transform);
  g_coreState.ecsIds.renderable = QuartzDefineComponent(Renderable);
  g_coreState.ecsIds.camera     = QuartzDefineComponent(Camera);
  g_coreState.ecsIds.lightDir   = QuartzDefineComponent(LightDirectional);
  g_coreState.ecsIds.lightPoint = QuartzDefineComponent(LightPoint);
  g_coreState.ecsIds.lightSpot  = QuartzDefineComponent(LightSpot);

  return Quartz_Success;
}

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
