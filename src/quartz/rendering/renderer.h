#pragma once

#include "quartz/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/window/window.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/texture.h"
#include "quartz/rendering/buffer.h"

namespace Quartz
{

struct alignas(16) ScenePacket
{
  Mat4 viewProjectionMatrix;
  Vec3 camPos; int _pad0;
  Vec3 camForward; int _pad1;
  Vec3 ambientColor; int _pad2;

  struct
  {
    LightDirectional directional; //[QTZ_LIGHT_DIRECTIONAL_MAX_COUNT];

    uint32_t pointCount;
    LightPoint pPoints[QTZ_LIGHT_POINT_MAX_COUNT];

    uint32_t spotCount;
    LightSpot pSpots[QTZ_LIGHT_SPOT_MAX_COUNT];
  } lights;
};

class Renderer
{
public:
  QuartzResult Init(Window* window);
  void Shutdown();

  QuartzResult StartFrame();
  QuartzResult EndFrame();
  void StartSceneRender();
  void EndSceneRender();
  void StartImguiRender();
  void EndImguiRender();

  QuartzResult Render(Renderable* renderable);

  static OpalShaderInputLayout SceneLayout() { return m_sceneLayout; }
  static OpalShaderInput* SceneSet() { return &m_sceneSet; }

  QuartzResult PushSceneData(ScenePacket* sceneInfo);

  QuartzResult Resize(uint32_t width, uint32_t height);

  OpalShaderInputLayout GetSingleImageLayout() const { return m_imguiImageLayout; }
  OpalRenderpass GetRenderpass() const { return m_renderpass; } // TODO : Replace for flexibility

private:
  QuartzResult InitImgui();

private:
  Window* m_qWindow;

  OpalWindow m_window;
  Texture m_windowBufferTexture;
  Texture m_depthTexture;

  uint32_t imageIndex;

  OpalRenderpass m_renderpass;
  std::vector<OpalFramebuffer> m_framebuffers;

  static OpalShaderInputLayout m_sceneLayout;
  static OpalShaderInput m_sceneSet;
  OpalBuffer m_sceneBuffer;

  OpalRenderpass m_imguiRenderpass;
  std::vector<OpalFramebuffer> m_imguiFramebuffers;
  OpalShaderInputLayout m_imguiImageLayout;
};

} // namespace Quartz
