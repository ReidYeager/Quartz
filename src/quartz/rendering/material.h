#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/texture.h"
#include "quartz/rendering/buffer.h"

#include <vector>

namespace Quartz
{

enum MaterialInputType
{
  Input_Buffer,
  Input_Texture
};

struct MaterialInput
{
  MaterialInputType type;
  union
  {
    Texture texture;
    Buffer buffer;
  };
};

class Material
{
friend class Renderer;

public:
  void Shutdown();
  QuartzResult PushData(void* data);

private:
  bool m_valid = false;
  OpalInputLayout m_layout;
  OpalBuffer m_buffer;
  OpalInputSet m_set;
  OpalShader m_shaders[2] = {};
  OpalMaterial m_material;

  QuartzResult Init(OpalRenderpass renderpass, const std::vector<const char*>& shaderPaths, const std::vector<MaterialInput>& inputs);
  QuartzResult InitBuffer();
  QuartzResult InitInputs(const std::vector<MaterialInput>& inputs);
  QuartzResult InitShaders(const std::vector<const char*>& shaderPaths);
  QuartzResult InitMaterial(OpalRenderpass renderpass);

  QuartzResult Bind() const;
};

} // namespace Quartz4