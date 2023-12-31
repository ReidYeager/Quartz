#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <vector>

namespace Quartz
{

class Material
{
friend class Renderer;

public:
  void Shutdown();
  QuartzResult PushData(void* data);

private:
  OpalInputLayout m_layout;
  OpalBuffer m_buffer;
  OpalInputSet m_set;
  OpalShader m_shaders[2] = {};
  OpalMaterial m_material;

  QuartzResult Init(OpalRenderpass renderpass, const std::vector<const char*>& shaderPaths);
  QuartzResult InitInputLayout();
  QuartzResult InitBuffer();
  QuartzResult InitInputSet();
  QuartzResult InitShaders(const std::vector<const char*>& shaderPaths);
  QuartzResult InitMaterial(OpalRenderpass renderpass);

  QuartzResult Bind() const;
};

} // namespace Quartz4