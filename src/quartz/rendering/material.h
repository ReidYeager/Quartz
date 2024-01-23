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
  Material() : m_isValid(false) {}
  Material(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs);
  QuartzResult Init(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs);

  ~Material();

  void Shutdown();
  void Reload();
  inline bool IsValid() const { return m_isValid; }

private:
  bool m_isValid = false;
  OpalRenderpass m_renderpass;
  OpalInputLayout m_layout;
  OpalBuffer m_buffer;
  OpalInputSet m_set;
  std::vector<OpalShader> m_shaders;
  std::vector<MaterialInput> m_inputs;
  OpalMaterial m_material;

  std::vector<std::string> m_shaderPaths;

  QuartzResult InitInputs(const std::vector<MaterialInput>& inputs);
  QuartzResult InitShaders(const std::vector<std::string>& shaderPaths);
  QuartzResult InitMaterial();

  QuartzResult Bind() const;
};

} // namespace Quartz4