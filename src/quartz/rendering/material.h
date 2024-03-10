#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/texture.h"
#include "quartz/rendering/buffer.h"

#include <vector>

namespace Quartz
{

struct ShaderSourceInfo
{
  uint64_t size;
  const void* data;
};

enum MaterialInputType
{
  Input_Buffer,
  Input_Texture
};

union MaterialInputValue
{
  const Texture* texture;
  const Buffer* buffer;
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
friend class TextureSkybox;

private:
  bool m_isValid = false;
  bool m_isBase = false;
  QuartzPipelineSettingFlags m_pipelineSettings = Pipeline_Cull_Back;

  std::vector<std::string> m_shaderPaths;
  std::vector<OpalShader> m_shaders;
  std::vector<MaterialInput> m_inputs;

  OpalRenderpass m_renderpass;
  OpalShaderGroup m_group;
  OpalShaderInputLayout m_inputLayout;
  OpalShaderInput m_inputSet;

public:
  inline bool IsValid() const { return m_isValid; }

  Material() : m_isValid(false), m_isBase(true) {}
  Material(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs);
  QuartzResult Init(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs, QuartzPipelineSettingFlags pipelineSettings = 0);
  // Init instance
  Material(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs);
  QuartzResult Init(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs);

  ~Material();
  void Shutdown();

  QuartzResult Reload();
  QuartzResult UpdateInputs();
  QuartzResult UpdateInputs(const std::vector<MaterialInputValue>& inputs);
  void SetSingleInput(uint32_t index, MaterialInputValue input);

private:
  QuartzResult Init(ShaderSourceInfo vertInfo, ShaderSourceInfo fragInfo, const std::vector<MaterialInput>& inputs, OpalRenderpass renderpass, QuartzPipelineSettingFlags pipelineSettings = 0);

  QuartzResult InitInputs(const std::vector<MaterialInput>& inputs);
  QuartzResult InitShaderFiles(const std::vector<std::string>& shaderPaths);
  QuartzResult InitMaterial();

  QuartzResult InitShader(uint32_t size, const void* source, OpalShaderType type, OpalShader* outShader);

  QuartzResult Bind() const;
};

} // namespace Quartz4