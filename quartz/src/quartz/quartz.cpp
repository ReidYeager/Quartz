
#include "quartz/defines.h"
#include "quartz.h"

#include "quartz/events/event.h"
#include "quartz/platform/platform.h"
#include "quartz/rendering/rendering.h"
#include "quartz/layers/layer.h"
#include "quartz/layers/layer_stack.h"

#include <chrono>
#include <math.h>

namespace Quartz
{
uint32_t g_quartzAttemptDepth = 0;
Time_T time;

Window* window;
Renderer renderer;
std::vector<Renderable> renderables;
LayerStack layerStack;
bool updateBlockedByOsInput = false;

void EventCallback(Event& e)
{
  //QTZ_DEBUG("{} : {}", e.GetTypeNameDebug(), e.ToString_Debug());
  updateBlockedByOsInput |= e.HasCategory(Event_Category_Window);
  if (e.GetType() == Event_Window_Resize)
  {
    EventWindowResize* resize = (EventWindowResize*)&e;
    // Adjust camera projections that point to the swapbuffer
    renderer.Resize(resize->GetWidth(), resize->GetHeight());
    renderer.Render(renderables);
  }

  for (auto iterator = layerStack.EndIterator(); iterator != layerStack.BeginIterator(); )
  {
    iterator--;
    (*iterator)->OnEvent(e);
    if (e.GetHandled())
      break;
  }
}

void Run()
{
  // Init
  // ============================================================

  Logger::Init();

  // Create window
  window = CreateWindow();
  window->SetEventCallbackFunction(EventCallback);

  // Init rendering api
  renderer.Init(window);

  PushLayer(GetGameLayer());

  // Update
  // ============================================================

  auto realStart = std::chrono::high_resolution_clock::now();
  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();

  time.frameCount = 0;
  time.totalTimeDeltaSum = 0.0;

  while (!window->ShouldClose())
  {
    end = std::chrono::high_resolution_clock::now();
    time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001;
    if (time.deltaTime >= 1.0f || updateBlockedByOsInput)
      time.deltaTime = 0.016f; // Fake 60fps for stepping through / moving or resizing the window
    updateBlockedByOsInput = false;
    time.totalTimeDeltaSum += time.deltaTime;
    time.totalRealTime = std::chrono::duration_cast<std::chrono::microseconds>(end - realStart).count() * 0.000001;
    start = end;

    window->PollEvents();
    renderables.clear();

    for (auto iterator = layerStack.BeginIterator(); iterator != layerStack.EndIterator(); )
    {
      (*iterator)->OnUpdate();
      iterator++;
    }

    renderer.Render(renderables);

    time.frameCount++;
  }

  QTZ_DEBUG("Average frame time : {} sec", time.totalTimeDeltaSum / time.frameCount);

  // Shutdown
  // ============================================================

  PopLayer(GetGameLayer());

  renderer.Shutdown();
  window->Shutdown();
  delete(window);
}

void Quit()
{
  window->MarkForClosure();
}

// ============================================================
// Window
// ============================================================

uint32_t WindowGetWidth()
{
  return window->Width();
}
uint32_t WindowGetHeight()
{
  return window->Height();
}

// ============================================================
// Rendering
// ============================================================

Material CreateMaterial(const std::vector<const char*>& shaderPaths)
{
  return renderer.CreateMaterial(shaderPaths);
}

//Mesh CreateMesh(const char* path)
//{
//  return g_renderer.CreateMesh(path);
//}
Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  return renderer.CreateMesh(vertices, indices);
}

void SubmitForRender(Renderable& renderable)
{
  renderables.push_back(renderable);
}

// ============================================================
// Layers
// ============================================================

void PushLayer(Layer* layer)
{
  layerStack.PushLayer(layer);
}

void PopLayer(Layer* layer)
{
  layerStack.PopLayer(layer);
}


} // namespace Quartz
