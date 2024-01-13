#pragma once

#include "quartz/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/window/window.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/texture.h"

#include <peridot.h>

namespace Quartz
{

struct ScenePacket
{
  Mat4 cameraViewProjectionMatrix;
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
