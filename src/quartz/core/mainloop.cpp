
#include "quartz/core/core.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#ifdef QTZ_PLATFORM_WIN32
#include <backends/imgui_impl_win32.h>
#endif // QTZ_PLATFORM_WIN32

namespace Quartz
{

// Variables
// ============================================================

ScenePacket g_packet = {};

// Declarations
// ============================================================

// Core
// QuartzResult CoreMainLoop() <-- Declared in quartz/core/core.h

// Preparation
void UpdateTimes();
QuartzResult UpdateLayers();
QuartzResult UpdateClient();
QuartzResult UpdateTransforms();
QuartzResult UpdateCameraVisibility();
QuartzResult UpdatePacket();

// Rendering
QuartzResult Render();
QuartzResult RenderScene();
QuartzResult RenderImgui();

// Core
// ============================================================

QuartzResult CoreMainLoop()
{
  while (!g_coreState.mainWindow.ShouldClose())
  {
    UpdateTimes();
    g_coreState.mainWindow.PollEvents();
    QTZ_ATTEMPT(UpdateLayers());
    QTZ_ATTEMPT(UpdateClient());
    QTZ_ATTEMPT(UpdateTransforms());
    QTZ_ATTEMPT(UpdateCameraVisibility());
    QTZ_ATTEMPT(UpdatePacket());
    QTZ_ATTEMPT(Render());
  }

  return Quartz_Success;
}

// Prep
// ============================================================

void UpdateTimes()
{
  g_coreState.time.clocks.frameEnd = std::chrono::high_resolution_clock::now();

  auto diff = g_coreState.time.clocks.frameEnd - g_coreState.time.clocks.frameStart;
  g_coreState.time.delta = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() * 0.000001;

#ifdef QTZ_DEBUG
  // Ignore large deltas that may be caused by debugging
  if (g_coreState.time.delta >= 1.0)
  {
    g_coreState.time.delta = 0.016;
  }
#endif // QTZ_DEBUG

  g_coreState.time.deltaSum += g_coreState.time.delta;

  g_coreState.time.clocks.frameStart = g_coreState.time.clocks.frameEnd;
  g_coreState.time.frameIndex++;
}

QuartzResult UpdateLayers()
{
  for (auto iterator = g_coreState.layerStack.BeginIterator(); iterator != g_coreState.layerStack.EndIterator();)
  {
    (*iterator)->OnUpdate();
    iterator++;
  }

  return Quartz_Success;
}

QuartzResult UpdateClient()
{
  QTZ_ATTEMPT(g_coreState.clientApp->Update(g_coreState.time.delta));
  return Quartz_Success;
}

QuartzResult UpdateTransforms()
{
  // Iterate through all Transform components
  //   Update the transformation matrix associated with transform[i]

  ObjectIterator renderableIter({g_coreState.ecsIds.transform, g_coreState.ecsIds.renderable});

  while (!renderableIter.AtEnd())
  {
    Transform* t = renderableIter.Get<Transform>();
    Renderable* r = renderableIter.Get<Renderable>();
    r->transformMatrix = t->Matrix();
    renderableIter.NextElement();
  }

  ObjectIterator camerasIter({ g_coreState.ecsIds.transform, g_coreState.ecsIds.camera });

  while (!camerasIter.AtEnd())
  {
    Transform* t = camerasIter.Get<Transform>();
    Camera* c = camerasIter.Get<Camera>();
    c->viewProjectionMatrix = c->projectionMatrix * t->Matrix().Inverted();
    c->pos = t->position;
    camerasIter.NextElement();
  }

  return Quartz_Success;
}

QuartzResult UpdateCameraVisibility()
{
  // Iterate through all cameras
  //   Determine objects visible to camera[i]
  //   Add objects' information for rendering

  return Quartz_Success;
}

QuartzResult UpdatePacket()
{
  ObjectIterator lightDirIter({g_coreState.ecsIds.lightDir});
  while (!lightDirIter.AtEnd())
  {
    g_packet.lights.directional = *lightDirIter.Get<LightDirectional>();
    break; // only one directional light supported
  }

  uint32_t pointIndex = 0;
  ObjectIterator lightPointIter({ g_coreState.ecsIds.lightPoint });
  while (!lightPointIter.AtEnd() && pointIndex < 4)
  {
    g_packet.lights.pPoints[pointIndex] = *lightPointIter.Get<LightPoint>();
    pointIndex++;
    lightPointIter.NextElement();
  }
  g_packet.lights.pointCount = pointIndex;

  uint32_t spotIndex = 0;
  ObjectIterator lightSpotIter({ g_coreState.ecsIds.lightSpot });
  while (!lightSpotIter.AtEnd() && spotIndex < 2)
  {
    g_packet.lights.pSpots[spotIndex] = *lightSpotIter.Get<LightSpot>();
    spotIndex++;
    lightSpotIter.NextElement();
  }
  g_packet.lights.spotCount = spotIndex;

  return Quartz_Success;
}

// Rendering
// ============================================================

QuartzResult Render()
{
  if (g_coreState.mainWindow.Minimized())
    return Quartz_Success;

  QTZ_ATTEMPT(g_coreState.renderer.StartFrame());

  QTZ_ATTEMPT(RenderScene());
  QTZ_ATTEMPT(RenderImgui());

  QTZ_ATTEMPT(g_coreState.renderer.EndFrame());

  return Quartz_Success;
}

QuartzResult RenderScene()
{
  g_coreState.renderer.StartSceneRender();

  ObjectIterator cameraIter({g_coreState.ecsIds.camera});
  ObjectIterator renderableIter({g_coreState.ecsIds.renderable});

  while (!cameraIter.AtEnd())
  {
    Camera* c = cameraIter.Get<Camera>();
    g_packet.viewProjectionMatrix = c->viewProjectionMatrix;
    g_packet.camPos = c->pos;

    QTZ_ATTEMPT(g_coreState.renderer.PushSceneData(&g_packet));

    while (!renderableIter.AtEnd())
    {
      Renderable* r = renderableIter.Get<Renderable>();
      g_coreState.renderer.Render(r);
      renderableIter.NextElement();
    }
    cameraIter.NextElement();
  }

  g_coreState.renderer.EndSceneRender();
  return Quartz_Success;
}

QuartzResult RenderImgui()
{
  g_coreState.renderer.StartImguiRender();
  ImGui_ImplVulkan_NewFrame();
#ifdef QTZ_PLATFORM_WIN32
  ImGui_ImplWin32_NewFrame();
#endif // QTZ_PLATFORM_WIN32
  ImGui::NewFrame();

  g_coreState.clientApp->RenderImgui();
  for (auto iterator = g_coreState.layerStack.BeginIterator(); iterator != g_coreState.layerStack.EndIterator();)
  {
    (*iterator)->OnUpdateImgui();
    iterator++;
  }

  //ImGui::Begin("Scene");
  //ImGui::DragFloat3("Ambient", (float*)&g_packet.ambientColor, 0.001f);
  //ImGui::End();

  ImGui::EndFrame();
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());
  g_coreState.renderer.EndImguiRender();

  return Quartz_Success;
}

} // namespace Quartz
