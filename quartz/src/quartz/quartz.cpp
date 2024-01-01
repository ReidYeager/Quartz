
#include "quartz/defines.h"
#include "quartz.h"

#include "quartz/events/event.h"
#include "quartz/platform/platform.h"
#include "quartz/rendering/rendering.h"

#include <chrono>
#include <math.h>

namespace Quartz
{
uint32_t g_quartzAttemptDepth = 0;

Window* g_window;
Renderer g_renderer;
std::vector<Renderable> g_renderables;

void EventCallback(Event& e)
{
  QTZ_DEBUG("{} : {}", e.GetTypeNameDebug(), e.ToString_Debug());

  // TODO : Pass the event down the layer stack
}

void Run(bool(*GameInit)(), bool(*GameUpdate)(float deltaTime), bool(*GameShutdown)())
{
  // ============================================================
  // Init
  // ============================================================

  Logger::Init();

  // Create window
  g_window = CreateWindow();
  g_window->SetEventCallbackFunction(EventCallback);

  // Init rendering api
  g_renderer.Init(g_window);

  GameInit();

  // ============================================================
  // Update
  // ============================================================

  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  double totalTime = 0.0f;
  uint32_t frameCount = 0;

  while (!g_window->ShouldClose())
  {
    end = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001f;
    start = end;
    totalTime += deltaTime;
    if (deltaTime >= 1.0f)
      deltaTime = 0.016f; // Fake 60fps for stepping through

    g_window->PollEvents();

    GameUpdate(deltaTime);

    g_renderer.Render(deltaTime, g_renderables);

    g_renderables.clear();
    frameCount++;
  }

  QTZ_DEBUG("Average frame time : {} sec", totalTime / frameCount);

  // ============================================================
  // Shutdown
  // ============================================================

  GameShutdown();

  g_renderer.Shutdown();
  g_window->Shutdown();
  delete(g_window);
}

uint32_t WindowGetWidth()
{
  return g_window->Width();
}
uint32_t WindowGetHeight()
{
  return g_window->Height();
}

Material CreateMaterial(const std::vector<const char*>& shaderPaths)
{
  return g_renderer.CreateMaterial(shaderPaths);
}

//Mesh CreateMesh(const char* path)
//{
//  return g_renderer.CreateMesh(path);
//}
Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  return g_renderer.CreateMesh(vertices, indices);
}

void SubmitForRender(Renderable& renderable)
{
  g_renderables.push_back(renderable);
}

} // namespace Quartz
