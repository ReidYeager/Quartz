
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
QuartzResult UpdateTransforms();
QuartzResult UpdateCameraVisibility();

// Rendering
QuartzResult Render();
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
    QTZ_ATTEMPT(g_coreState.clientApp->Update(g_coreState.time.delta));
    QTZ_ATTEMPT(UpdateTransforms());
    QTZ_ATTEMPT(UpdateCameraVisibility());
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

QuartzResult UpdateTransforms()
{
  // Iterate through all Transform components
  //   Update the transformation matrix associated with transform[i]

  return Quartz_Success;
}

QuartzResult UpdateCameraVisibility()
{
  // Iterate through all cameras
  //   Determine objects visible to camera[i]
  //   Add objects' information for rendering

  return Quartz_Success;
}

// Rendering
// ============================================================

QuartzResult Render()
{
  if (g_coreState.mainWindow.Minimized())
    return Quartz_Success;

  QTZ_ATTEMPT(g_coreState.renderer.StartFrame());
  g_coreState.renderer.StartSceneRender();

  QTZ_ATTEMPT(g_coreState.renderer.PushSceneData(&g_packet));

  // Render all cameras' visible objects

  QTZ_ATTEMPT(RenderImgui());
  QTZ_ATTEMPT(g_coreState.renderer.EndFrame());

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

  ImGui::EndFrame();
  ImGui::Render();
  ImDrawData* drawData = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(drawData, OpalRenderGetCommandBuffer());
  g_coreState.renderer.EndImguiRender();

  return Quartz_Success;
}

} // namespace Quartz
