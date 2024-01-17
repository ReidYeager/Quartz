#pragma once

#include "quartz/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/window/window.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/texture.h"
#include "quartz/rendering/buffer.h"

#include <peridot.h>

namespace Quartz
{

struct LightDirectional
{
  Vec3 color;
  Vec3 direction;
};

#define QTZ_LIGHT_DIRECTIONAL_MAX_COUNT 1

struct LightPoint
{
  Vec3 color;
  Vec3 position;

  float linear;
  float quadratic;
};

#define QTZ_LIGHT_POINT_MAX_COUNT 4

struct LightSpot
{
  Vec3 color;
  Vec3 position;
  Vec3 direction;

  float inner;
  float outer;

  float linear;
  float quadratic;
};

#define QTZ_LIGHT_SPOT_MAX_COUNT 2

struct ScenePacket
{
  Mat4 viewProjectionMatrix;
  Vec3 camPos;
  Vec3 camForward;
  Vec3 ambientColor;

  struct
  {
    LightDirectional directional; //[QTZ_LIGHT_DIRECTIONAL_MAX_COUNT];
    LightPoint pPoints[QTZ_LIGHT_POINT_MAX_COUNT];
    LightSpot pSpots[QTZ_LIGHT_SPOT_MAX_COUNT];
  } lights;
};

static const std::vector<OpalBufferElement> packetElements = {
  Opal_Buffer_Mat4,   /*ViewProj*/
  Opal_Buffer_Float3, /*CamPos*/
  Opal_Buffer_Float3, /*CamFwd*/
  Opal_Buffer_Float3, /*Ambient color*/

  Opal_Buffer_Float3, /*Directional - color*/
  Opal_Buffer_Float3, /*Directional - direction*/

  Opal_Buffer_Float3, /*Point - color*/
  Opal_Buffer_Float3, /*Point - position*/
  Opal_Buffer_Float,  /*Point - linear*/
  Opal_Buffer_Float,  /*Point - quadratic*/

  Opal_Buffer_Float3, /*Point - color*/
  Opal_Buffer_Float3, /*Point - position*/
  Opal_Buffer_Float,  /*Point - linear*/
  Opal_Buffer_Float,  /*Point - quadratic*/

  Opal_Buffer_Float3, /*Point - color*/
  Opal_Buffer_Float3, /*Point - position*/
  Opal_Buffer_Float,  /*Point - linear*/
  Opal_Buffer_Float,  /*Point - quadratic*/

  Opal_Buffer_Float3, /*Point - color*/
  Opal_Buffer_Float3, /*Point - position*/
  Opal_Buffer_Float,  /*Point - linear*/
  Opal_Buffer_Float,  /*Point - quadratic*/

  Opal_Buffer_Float3, /*Spot - color*/
  Opal_Buffer_Float3, /*Spot - position*/
  Opal_Buffer_Float3, /*Spot - direction*/
  Opal_Buffer_Float,  /*Spot - inner*/
  Opal_Buffer_Float,  /*Spot - outer*/
  Opal_Buffer_Float,  /*Spot - linear*/
  Opal_Buffer_Float,  /*Spot - quadratic*/

  Opal_Buffer_Float3, /*Spot - color*/
  Opal_Buffer_Float3, /*Spot - position*/
  Opal_Buffer_Float3, /*Spot - direction*/
  Opal_Buffer_Float,  /*Spot - inner*/
  Opal_Buffer_Float,  /*Spot - outer*/
  Opal_Buffer_Float,  /*Spot - linear*/
  Opal_Buffer_Float,  /*Spot - quadratic*/
};

struct Renderable
{
  class Mesh* mesh;
  class Material* material;
  Mat4 transformMatrix;
};

class Renderer
{
public:
  QuartzResult Init(Window* window);
  QuartzResult InitImgui();
  void Shutdown();

  QuartzResult StartFrame();
  QuartzResult EndFrame();
  void StartSceneRender();
  void EndSceneRender();
  void StartImguiRender();
  void EndImguiRender();

  QuartzResult Render(Renderable* renderable);

  static OpalInputLayout SceneLayout() { return m_sceneLayout; }
  static OpalInputSet SceneSet() { return m_sceneSet; }

  QuartzResult PushSceneData(ScenePacket* sceneInfo);

  QuartzResult Resize(uint32_t width, uint32_t height);

  Material CreateMaterial(const std::vector<const char*>& shaderPaths, const std::vector<MaterialInput>& inputs);
  Mesh CreateMesh(const char* path);
  Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
  Texture CreateTexture(const char* path);
  Buffer CreateBuffer(uint32_t size);

private:
  Window* m_qWindow;

  OpalWindow m_window;
  OpalImage m_windowBufferImage;
  OpalImage m_depthImage;

  OpalRenderpass m_renderpass;
  OpalFramebuffer m_framebuffer;

  static OpalInputLayout m_sceneLayout;
  static OpalInputSet m_sceneSet;
  OpalBuffer m_sceneBuffer;

  OpalRenderpass m_imguiRenderpass;
  OpalFramebuffer m_imguiFramebuffer;
  OpalInputLayout m_imguiImageLayout;
};

} // namespace Quartz
