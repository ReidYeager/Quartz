
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
Diamond::EcsWorld globalEcsWorld;
std::vector<Diamond::Entity> entities;
ComponentId cameraComponentId;
ComponentId renderableComponentId;
ComponentId transformComponentId;

void EventCallback(Event& e)
{
  updateBlockedByOsInput |= e.HasCategory(Event_Category_Window);
  if (e.GetType() == Event_Window_Resize)
  {
    EventWindowResize* resize = (EventWindowResize*)&e;
    // Adjust camera projections that point to the swapbuffer
    renderer.Resize(resize->GetWidth(), resize->GetHeight());
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

  transformComponentId = QuartzDefineComponent(Transform);
  renderableComponentId = QuartzDefineComponent(Renderable);
  cameraComponentId = QuartzDefineComponent(Camera);

  PushLayer(GetGameLayer());

  // Update
  // ============================================================

  auto realStart = std::chrono::high_resolution_clock::now();
  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();

  time.frameCount = 0;
  time.totalTimeDeltaSum = 0.0;

  ScenePacket scenePacket = {};

  while (!window->ShouldClose())
  {
    end = std::chrono::high_resolution_clock::now();
    time.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001;
    if (time.deltaTime >= 1.0f || updateBlockedByOsInput)
      time.deltaTime = 0.016f; // Fake 60fps for stepping through or moving/resizing the window
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

    Diamond::EcsIterator camerasIter(&globalEcsWorld, { cameraComponentId, transformComponentId });
    Diamond::EcsIterator renderableIter(&globalEcsWorld, { renderableComponentId, transformComponentId });

    if (!window->Minimized())
    {
      renderer.StartFrame();
    }

    while (!camerasIter.AtEnd())
    {
      Transform* camTransform = (Transform*)camerasIter.GetComponent(transformComponentId);
      Camera* cam = (Camera*)camerasIter.GetComponent(cameraComponentId);
      scenePacket.cameraViewProjectionMatrix = Mat4MuliplyMat4(cam->projectionMatrix, Mat4Invert(TransformToMat4(*camTransform)));
      renderer.PushSceneData(&scenePacket);

      while (!renderableIter.AtEnd())
      {
        Renderable* r = (Renderable*)renderableIter.GetComponent(renderableComponentId);
        Transform* t = (Transform*)renderableIter.GetComponent(transformComponentId);

        r->transformMatrix = TransformToMat4(*t);

        if (!window->Minimized())
        {
          renderer.Render(r);
        }

        renderableIter.StepNextElement();
      }

      camerasIter.StepNextElement();
    }

    if (!window->Minimized())
    {
      renderer.EndFrame();
    }

    time.frameCount++;
  }

  QTZ_DEBUG("Average frame time : {} sec", time.totalTimeDeltaSum / time.frameCount);

  // Shutdown
  // ============================================================

  PopLayer(GetGameLayer());

  //for (auto e : entities)
  //{
  //  globalEcsWorld.DestroyEntity(e);
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

ComponentId _DefineComponent(const char* name, size_t size)
{
  return (ComponentId)globalEcsWorld.DefineComponent(name, size);
}

ComponentId _ComponentId(const char* name)
{
  return globalEcsWorld.GetComponentId(name);
}

Entity::Entity()
{
  m_id = globalEcsWorld.CreateEntity();
  AddComponent(m_id, QuartzComponentId(Transform));
  //entities.push_back(m_id);
}

Entity::~Entity()
{
  globalEcsWorld.DestroyEntity(m_id);
}

bool HasComponent(Diamond::Entity e, ComponentId id)
{
  return globalEcsWorld.EntityHasComponent(e, (Diamond::ComponentId)id);
}

void* AddComponent(Diamond::Entity e, ComponentId id)
{
  return globalEcsWorld.AddComponent(e, (Diamond::ComponentId)id);
}

void* GetComponent(Diamond::Entity e, ComponentId id)
{
  return globalEcsWorld.GetComponent(e, (Diamond::ComponentId)id);
}

void RemoveComponent(Diamond::Entity e, ComponentId id)
{
  globalEcsWorld.RemoveComponent(e, (Diamond::ComponentId)id);
}

ObjectIterator::ObjectIterator(const std::vector<ComponentId>& componentIds)
{
  m_iterator = new Diamond::EcsIterator(&globalEcsWorld, componentIds);
}

} // namespace Quartz
