
#include "quartz/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/renderer.h"
#include "quartz/platform/filesystem/filesystem.h"
#include "quartz/core/core.h"

namespace Quartz
{

Material::Material(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs) :
  m_isValid(false), m_isBase(true)
{
  QTZ_ATTEMPT_VOID(Init(shaderPaths, inputs));
}

QuartzResult Material::Init(ShaderSourceInfo vertInfo, ShaderSourceInfo fragInfo, const std::vector<MaterialInput>& inputs, OpalRenderpass renderpass, QuartzPipelineSettingFlags pipelineSettings)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid material");
    return Quartz_Success;
  }
  m_isBase = true;
  m_pipelineSettings = pipelineSettings;
  m_renderpass = renderpass;

  QTZ_ATTEMPT(InitInputs(inputs));

  if (!m_shaders.size())
  {
    m_shaders.resize(2);
  }
  QTZ_ATTEMPT(InitShader(vertInfo.size, vertInfo.data, Opal_Shader_Vertex, &m_shaders[0]));
  QTZ_ATTEMPT(InitShader(fragInfo.size, fragInfo.data, Opal_Shader_Fragment, &m_shaders[1]));

  QTZ_ATTEMPT(InitMaterial());

  m_inputs = std::vector<MaterialInput>(inputs);

  m_isValid = true;

  return Quartz_Success;
}

QuartzResult Material::Init(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs, QuartzPipelineSettingFlags pipelineSettings)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid material");
    return Quartz_Success;
  }
  m_isBase = true;
  m_pipelineSettings = pipelineSettings;
  m_renderpass = g_coreState.renderer.GetRenderpass();

  QTZ_ATTEMPT(InitInputs(inputs));
  QTZ_ATTEMPT(InitShaderFiles(shaderPaths));
  QTZ_ATTEMPT(InitMaterial());

  m_shaderPaths.resize(shaderPaths.size());
  for (uint32_t i = 0; i < shaderPaths.size(); i++)
  {
    m_shaderPaths[i] = std::string(shaderPaths[i]);
  }

  m_inputs = std::vector<MaterialInput>(inputs);

  m_isValid = true;
  return Quartz_Success;
}

Material::Material(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs) :
  m_isValid(false), m_isBase(false)
{
  QTZ_ATTEMPT_VOID(Init(existingMaterial, inputs));
}

QuartzResult Material::Init(Material& existingMaterial, const std::vector<MaterialInputValue>& inputs)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid material");
    return Quartz_Success;
  }
  if (!existingMaterial.m_isValid)
  {
    QTZ_ERROR("Attempting to create an instance of an invalid material");
    return Quartz_Failure;
  }
  if (!existingMaterial.m_isBase)
  {
    QTZ_ERROR("Attempting to create an instance of a material instance");
    return Quartz_Failure;
  }
  if (existingMaterial.m_inputs.size() != inputs.size())
  {
    QTZ_ERROR(
      "Attempting to create a material instance with an invalid number of inputs ({} should be {}",
      inputs.size(),
      existingMaterial.m_inputs.size());
    return Quartz_Failure;
  }

  m_isBase = false;
  m_inputLayout = existingMaterial.m_inputLayout;
  m_group = existingMaterial.m_group;
  m_renderpass = existingMaterial.m_renderpass;

  m_inputs = existingMaterial.m_inputs;
  for (uint32_t i = 0; i < inputs.size(); i++)
  {
    m_inputs[i].value = inputs[i];
  }

  QTZ_ATTEMPT(InitInputs(m_inputs));

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Material::InitInputs(const std::vector<MaterialInput>& inputs)
{
  std::vector<OpalStageFlags> stages(inputs.size());
  std::vector<OpalShaderInputType> types(inputs.size());
  std::vector<OpalShaderInputValue> values(inputs.size());
  

  for (uint32_t i = 0; i < inputs.size(); i++)
  {
    stages[i] = Opal_Stage_All;

    switch (inputs[i].type)
    {
    case Input_Texture:
    {
      if (!inputs[i].value.texture->IsValid())
      {
        QTZ_ERROR("Attempting to use an invalid texture as material input {}", i);
        return Quartz_Failure;
      }

      types[i] = Opal_Shader_Input_Image;
      values[i].image = &inputs[i].value.texture->m_opalImage;
    } break;
    case Input_Buffer:
    {
      if (!inputs[i].value.buffer->IsValid())
      {
        QTZ_ERROR("Attempting to use an invalid buffer as material input {}", i);
        return Quartz_Failure;
      }

      types[i] = Opal_Shader_Input_Buffer;
      values[i].buffer = &inputs[i].value.buffer->m_opalBuffer;
    } break;
    default: return Quartz_Failure;
    }
  }

  if (m_isBase)
  {
    OpalShaderInputLayoutInitInfo layoutInfo;
    layoutInfo.count = inputs.size();
    layoutInfo.pStages = stages.data();
    layoutInfo.pTypes = types.data();

    QTZ_ATTEMPT_OPAL(OpalShaderInputLayoutInit(&m_inputLayout, layoutInfo));
  }

  OpalShaderInputInitInfo setInfo;
  setInfo.layout = m_inputLayout;
  setInfo.pValues = values.data();

  QTZ_ATTEMPT_OPAL(OpalShaderInputInit(&m_inputSet, setInfo));
  return Quartz_Success;
}

QuartzResult Material::InitShaderFiles(const std::vector<std::string>& shaderPaths)
{
  char* fileBuffer;
  uint32_t size = 0;

  if (!m_shaders.size())
  {
    m_shaders.resize(2);
  }

  size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[0].c_str());
  QTZ_ATTEMPT(InitShader(size, fileBuffer, Opal_Shader_Vertex, &m_shaders[0]));
  free(fileBuffer);

  size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[1].c_str());
  QTZ_ATTEMPT(InitShader(size, fileBuffer, Opal_Shader_Fragment, &m_shaders[1]));
  free(fileBuffer);

  return Quartz_Success;
}

QuartzResult Material::InitShader(uint32_t size, const void* source, OpalShaderType type, OpalShader* outShader)
{
  OpalShaderInitInfo info;
  info.type = type;
  info.sourceSize = size;
  info.pSource = source;

  QTZ_ATTEMPT_OPAL(OpalShaderInit(outShader, info));
  return Quartz_Success;
}

QuartzResult Material::InitMaterial()
{
  const uint32_t layoutCount = 2;
  OpalShaderInputLayout layouts[layoutCount] = {
    Renderer::SceneLayout(),
    m_inputLayout
  };

  OpalShaderGroupInitInfo initInfo;
  initInfo.renderpass = m_renderpass;
  initInfo.subpassIndex = 0;
  initInfo.shaderInputLayoutCount = layoutCount;
  initInfo.pShaderInputLayouts = layouts;
  initInfo.shaderCount = m_shaders.size();
  initInfo.pShaders = m_shaders.data();
  initInfo.pushConstantSize = sizeof(Mat4);
  initInfo.flags = (OpalPipelineFlags)m_pipelineSettings;

  QTZ_ATTEMPT_OPAL(OpalShaderGroupInit(&m_group, initInfo));

  return Quartz_Success;
}

QuartzResult Material::Bind() const
{
  if (!m_isValid)
  {
    QTZ_ERROR("Attempting to use an invalid material");
    return Quartz_Failure;
  }

  OpalRenderBindShaderGroup(&m_group);
  OpalRenderBindShaderInput(Renderer::SceneSet(), 0);
  OpalRenderBindShaderInput(&m_inputSet, 1);

  return Quartz_Success;
}

Material::~Material()
{
  if (m_isBase)
  {
    if (m_isValid)
    {
      QTZ_ERROR("Material must be shut down manually");
    }
    else
    {
      Shutdown();
    }
  }
}

void Material::Shutdown()
{
  if (!m_isValid)
  {
    return;
  }
  OpalWaitIdle();

  if (m_isBase)
  {
    OpalShaderGroupShutdown(&m_group);
    for (uint32_t i = 0; i < m_shaders.size(); i++)
    {
      OpalShaderShutdown(&m_shaders[i]);
    }

    OpalShaderInputLayoutShutdown(&m_inputLayout);
  }

  OpalShaderInputShutdown(&m_inputSet);

  m_isValid = false;
}

QuartzResult Material::Reload()
{
  if (!m_isValid || !m_isBase)
  {
    QTZ_WARNING("Attempting to reload an invalid material");
    return Quartz_Success;
  }

  OpalWaitIdle();

  OpalShaderGroupShutdown(&m_group);
  if (m_shaderPaths.size() > 0)
  {
    QTZ_ATTEMPT(InitShaderFiles(m_shaderPaths));
  }
  QTZ_ATTEMPT(InitMaterial());

  return Quartz_Success;
}

QuartzResult Material::UpdateInputs()
{
  if (!m_isValid)
  {
    QTZ_WARNING("Attempting to update inputs on an invalid material");
    return Quartz_Failure;
  }

  OpalWaitIdle();
  OpalShaderInputShutdown(&m_inputSet);
  OpalShaderInputLayoutShutdown(&m_inputLayout);
  QTZ_ATTEMPT(InitInputs(m_inputs));
  return Quartz_Success;
}

QuartzResult Material::UpdateInputs(const std::vector<MaterialInputValue>& inputs)
{
  if (!m_isValid)
  {
    QTZ_WARNING("Attempting to update inputs on an invalid material");
    return Quartz_Failure;
  }

  for (uint32_t i = 0; i < inputs.size(); i++)
  {
    m_inputs[i].value = inputs[i];
  }

  QTZ_ATTEMPT(UpdateInputs());

  return Quartz_Success;
}

void Material::SetSingleInput(uint32_t index, MaterialInputValue input)
{
  if (!m_isValid)
  {
    QTZ_WARNING("Attempting to set an input on an invalid material");
    return;
  }

  m_inputs[index].value = input;
}

} // namespace Quartz
