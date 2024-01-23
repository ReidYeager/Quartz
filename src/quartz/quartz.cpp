
#if 0

#include "quartz/defines.h"
#include "quartz.h"

#include "quartz/events/event.h"
#include "quartz/platform/platform.h"
#include "quartz/rendering/renderer.h"
#include "quartz/layers/layer.h"
#include "quartz/layers/layer_stack.h"

#include <diamond.h>

#include <backends/imgui_impl_win32.h>

#include <chrono>
#include <math.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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
ComponentId lightDirectionalComponentId;
ComponentId lightPointComponentId;
ComponentId lightSpotComponentId;

void PlatformInputCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

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
  window->SetPlatformInputCallbackFunction(PlatformInputCallback);
  // Init rendering api
  renderer.Init(window);

  transformComponentId = QuartzDefineComponent(Transform);
  renderableComponentId = QuartzDefineComponent(Renderable);
  cameraComponentId = QuartzDefineComponent(Camera);
  lightDirectionalComponentId = QuartzDefineComponent(LightDirectional);
  lightPointComponentId = QuartzDefineComponent(LightPoint);
  lightSpotComponentId = QuartzDefineComponent(LightSpot);

  PushLayer(GetGameLayer());

  // Update
  // ============================================================

  auto realStart = std::chrono::high_resolution_clock::now();
  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();

  time.frameCount = 0;
  time.totalTimeDeltaSum = 0.0;

  ScenePacket scenePacket = {};
  scenePacket.ambientColor = Vec3{ 0.0f, 0.0f, 0.0f };

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
      renderer.StartSceneRender();

      // Rough light preparation
      Diamond::EcsIterator iterLightDir(&globalEcsWorld, { lightDirectionalComponentId });
      Diamond::EcsIterator iterLightPoint(&globalEcsWorld, { lightPointComponentId });
      Diamond::EcsIterator iterLightSpot(&globalEcsWorld, { lightSpotComponentId });

      if (!iterLightDir.AtEnd())
      {
        scenePacket.lights.directional = *(LightDirectional*)iterLightDir.GetComponent(lightDirectionalComponentId);
      }

      for (uint32_t i = 0; i < QTZ_LIGHT_POINT_MAX_COUNT && !iterLightPoint.AtEnd(); i++)
      {
        scenePacket.lights.pPoints[i] = *(LightPoint*)iterLightPoint.GetComponent(lightPointComponentId);
        iterLightPoint.StepNextElement();
      }

      for (uint32_t i = 0; i < QTZ_LIGHT_SPOT_MAX_COUNT && !iterLightSpot.AtEnd(); i++)
      {
        scenePacket.lights.pSpots[i] = *(LightSpot*)iterLightSpot.GetComponent(lightSpotComponentId);
        iterLightSpot.StepNextElement();
      }
    }

    while (!camerasIter.AtEnd())
    {
      Transform* camTransform = (Transform*)camerasIter.GetComponent(transformComponentId);
      Camera* cam = (Camera*)camerasIter.GetComponent(cameraComponentId);
      scenePacket.viewProjectionMatrix = Mat4MuliplyMat4(cam->projectionMatrix, Mat4Invert(TransformToMat4(*camTransform)));
      scenePacket.camPos = camTransform->position;
      scenePacket.camForward = QuaternionMultiplyVec3(QuaternionFromEuler(camTransform->rotation), Vec3{0.0f, 0.0f, -1.0f});
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
      renderer.EndSceneRender();
      renderer.StartImguiRender();

      ImGui_ImplVulkan_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();

      // Scene
      {
        ImGui::Begin("Scene");

        ImGui::DragFloat3("Ambient color", (float*)&scenePacket.ambientColor, 0.01f);

        ImGui::End();
      }

      for (auto iterator = layerStack.BeginIterator(); iterator != layerStack.EndIterator(); )
      {
        (*iterator)->OnUpdateImgui();
        iterator++;
      }

      ImGui::EndFrame();
      ImGui::Render();

      ImDrawData* drawData = ImGui::GetDrawData();
      ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());

      renderer.EndImguiRender();
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

Material CreateMaterial(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs)
{
  return renderer.CreateMaterial(shaderPaths, inputs);
}

Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  return renderer.CreateMesh(vertices, indices);
}

Mesh CreateMesh(const char* path)
{
  return renderer.CreateMesh(path);
}

Texture CreateTexture(const char* path)
{
  return renderer.CreateTexture(path);
}

Buffer CreateBuffer(uint32_t size)
{
  return renderer.CreateBuffer(size);
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

#endif
