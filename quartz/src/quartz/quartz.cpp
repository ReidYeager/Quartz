
#include "quartz/defines.h"
#include "quartz.h"

#include "quartz/events/event.h"
#include "quartz/platform/platform.h"
#include "quartz/rendering/rendering.h"
#include "quartz/layers/layer.h"
#include "quartz/layers/layer_stack.h"

#include <diamond.h>

#include <chrono>
#include <math.h>

namespace Quartz
{
uint32_t g_quartzAttemptDepth = 0;
Time_T time;

Window* window;
Renderer renderer;
LayerStack layerStack;
bool updateBlockedByOsInput = false;
Diamond::EcsWorld ecsWorld;
std::vector<Diamond::Entity> entities;
Diamond::ComponentId renderableComponentId;

void EventCallback(Event& e)
{
  //QTZ_DEBUG("{} : {}", e.GetTypeNameDebug(), e.ToString_Debug());
  updateBlockedByOsInput |= e.HasCategory(Event_Category_Window);
  if (e.GetType() == Event_Window_Resize)
  {
    EventWindowResize* resize = (EventWindowResize*)&e;
    // Adjust camera projections that point to the swapbuffer
    renderer.Resize(resize->GetWidth(), resize->GetHeight());
    renderer.Render();
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

  renderableComponentId = ecsWorld.DefineComponent("renderable", sizeof(Renderable));

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

    for (auto iterator = layerStack.BeginIterator(); iterator != layerStack.EndIterator(); )
    {
      (*iterator)->OnUpdate();
      iterator++;
    }

    Diamond::EcsIterator renderableIter(&ecsWorld, {renderableComponentId});
    renderer.ClearRenderables();
    while (!renderableIter.AtEnd())
    {
      renderer.SubmitRenderable((Renderable*)renderableIter.GetComponent(renderableComponentId));
      renderableIter.StepNextElement();
    }
    renderer.Render();

    time.frameCount++;
  }

  QTZ_DEBUG("Average frame time : {} sec", time.totalTimeDeltaSum / time.frameCount);

  // Shutdown
  // ============================================================

  PopLayer(GetGameLayer());

  //for (auto e : entities)
  //{
  //  ecsWorld.DestroyEntity(e);
  //}

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

// ============================================================
// Objects
// ============================================================

Renderable* CreateObject()
{
  entities.push_back(ecsWorld.CreateEntity());
  return (Renderable*)ecsWorld.AddComponent(entities.back(), renderableComponentId);
}

ObjectIterator CreateIterator(const std::vector<const char*>& componentNames)
{
  ObjectIterator iter(&ecsWorld, componentNames);
  return iter;
}

} // namespace Quartz
