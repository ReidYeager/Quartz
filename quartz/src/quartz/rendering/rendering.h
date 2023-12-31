#pragma once

#include "quartz/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/window/window.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"

#include <peridot.h>

namespace Quartz
{

struct Renderable
{
  Mesh mesh;
  Material material;

  Transform transform;
  Mat4 transformMatrix;
};

class Renderer
{
public:
  QuartzResult Init(Window* window);
  QuartzResult Render(float deltaTime, const std::vector<Renderable>& renderables);
  void Shutdown();

  Material CreateMaterial(const std::vector<const char*>& shaderPaths);
  Mesh CreateMesh(const char* path);
  Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

private:
  Window* m_qWindow;

  OpalWindow m_window;
  OpalImage m_windowBufferImage;
  OpalImage m_depthImage;

  OpalRenderpass m_renderpass;
  OpalFramebuffer m_framebuffer;

  Transform m_camTransform = transformIdentity;
  Mat4 m_camProjection = mat4Identity;
  Mat4 m_cameraMatrix = mat4Identity;

  Transform m_objectTransform = transformIdentity;

};

} // namespace Quartz
