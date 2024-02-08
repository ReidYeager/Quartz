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

union MaterialInputValue
{
  Texture* texture;
  Buffer* buffer;
};

struct MaterialInput
{
  MaterialInputType type;
  MaterialInputValue value;
};

enum QuartzPipelineSettingFlagBits
{
  Pipeline_Cull_Back  = (0 << 0),
  Pipeline_Cull_Front = (1 << 0),
  Pipeline_Cull_Both  = (2 << 0),
  Pipeline_Cull_None  = (3 << 0),
  Pipeline_Cull_BITS  = (3 << 0), // 2 bits - 2 total

  Pipeline_Depth_Compare_Less      = (0 << 2),
  Pipeline_Depth_Compare_LessEqual = (1 << 2),
  Pipeline_Depth_Compare_BITS      = (1 << 2), // 1 bit - 3 total
};
typedef uint64_t QuartzPipelineSettingFlags;


class Material
{
friend class Renderer;
friend class MaterialInstance;
friend QuartzResult ConvolveHdri();

public:
  Material() : m_isValid(false), m_isBase(true) {}
  Material(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs);
  QuartzResult Init(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs, QuartzPipelineSettingFlags pipelineSettings = 0);
  Material(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs);
  QuartzResult Init(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs);

  ~Material();

  void Shutdown();
  QuartzResult Reload();
  inline bool IsValid() const { return m_isValid; }

private:
  bool m_isValid = false;
  bool m_isBase = false;
  Material* m_parent = nullptr;
  QuartzPipelineSettingFlags m_pipelineSettings = Pipeline_Cull_Back;

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