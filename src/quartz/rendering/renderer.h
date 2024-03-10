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
  Vec3 camPos; int pad0;
  Vec3 camForward; int pad1;
  Vec3 ambientColor; int pad2;

  struct
  {
    LightDirectional directional; //[QTZ_LIGHT_DIRECTIONAL_MAX_COUNT];

    uint32_t pointCount;
    LightPoint pPoints[QTZ_LIGHT_POINT_MAX_COUNT];

    uint32_t spotCount;
    LightSpot pSpots[QTZ_LIGHT_SPOT_MAX_COUNT];
  } lights;
};

//static const std::vector<OpalBufferElement> packetElements = {
//  Opal_Buffer_Mat4,   /*ViewProj*/
//
//  Opal_Buffer_Float3, /*CamPos*/
//  Opal_Buffer_Float3, /*CamFwd*/
//  Opal_Buffer_Float3, /*Ambient color*/
//
//  Opal_Buffer_Float3, /*Directional - color*/
//  Opal_Buffer_Float,  /*Directional - intensity*/
//  Opal_Buffer_Float3, /*Directional - direction*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Uint, /*Point count*/
//
//  Opal_Buffer_Float3, /*Point - color*/
//  Opal_Buffer_Float,  /*Point - intensity*/
//  Opal_Buffer_Float3, /*Point - position*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Float3, /*Point - color*/
//  Opal_Buffer_Float,  /*Point - intensity*/
//  Opal_Buffer_Float3, /*Point - position*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Float3, /*Point - color*/
//  Opal_Buffer_Float,  /*Point - intensity*/
//  Opal_Buffer_Float3, /*Point - position*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Float3, /*Point - color*/
//  Opal_Buffer_Float,  /*Point - intensity*/
//  Opal_Buffer_Float3, /*Point - position*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Uint, /*Spot count*/
//
//  Opal_Buffer_Float3, /*Spot - color*/
//  Opal_Buffer_Float,  /*Spot - intensity*/
//  Opal_Buffer_Float3, /*Spot - position*/
//  Opal_Buffer_Float3, /*Spot - direction*/
//  Opal_Buffer_Float,  /*Spot - inner*/
//  Opal_Buffer_Float,  /*Spot - outer*/
//  Opal_Buffer_Structure_End,
//
//  Opal_Buffer_Float3, /*Spot - color*/
//  Opal_Buffer_Float,  /*Spot - intensity*/
//  Opal_Buffer_Float3, /*Spot - position*/
//  Opal_Buffer_Float3, /*Spot - direction*/
//  Opal_Buffer_Float,  /*Spot - inner*/
//  Opal_Buffer_Float,  /*Spot - outer*/
//  Opal_Buffer_Structure_End,
//};

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
