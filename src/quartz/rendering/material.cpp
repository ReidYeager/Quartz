
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

QuartzResult Material::Init(const std::vector<std::string>& shaderPaths, const std::vector<MaterialInput>& inputs, QuartzPipelineSettingFlags pipelineSettings)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid material");
    return Quartz_Success;
  }
  m_isBase = true;
  m_pipelineSettings = pipelineSettings;

  QTZ_ATTEMPT(InitInputs(inputs));
  QTZ_ATTEMPT(InitShaders(shaderPaths));
  QTZ_ATTEMPT(InitMaterial());

  m_shaderPaths.resize(shaderPaths.size());
  for (uint32_t i = 0; i < shaderPaths.size(); i++)
  {
    m_shaderPaths[i] = std::string(shaderPaths[i]);
  }

  m_renderpass = g_coreState.renderer.GetRenderpass();
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
  m_layout = existingMaterial.m_layout;

  m_inputs = existingMaterial.m_inputs;
  for (uint32_t i = 0; i < inputs.size(); i++)
  {
    m_inputs[i].value = inputs[i];
  }

  QTZ_ATTEMPT(InitInputs(m_inputs));

  m_parent = &existingMaterial;
  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Material::InitInputs(const std::vector<MaterialInput>& inputs)
{
  std::vector<OpalInputAccessInfo> infos(inputs.size());
  std::vector<OpalInputValue> values(inputs.size());

  for (uint32_t i = 0; i < values.size(); i++)
  {
    infos[i].stages = Opal_Stage_All_Graphics;

    switch (inputs[i].type)
    {
    case Input_Texture:
    {
      if (!inputs[i].value.texture.IsValid())
      {
        QTZ_ERROR("Attempting to use an invalid texture as material input {}", i);
        return Quartz_Failure;
      }

      values[i].image = inputs[i].value.texture.m_opalImage;
      infos[i].type = Opal_Input_Type_Samped_Image;
    } break;
    case Input_Buffer:
    {
      if (!inputs[i].value.buffer.IsValid())
      {
        QTZ_ERROR("Attempting to use an invalid buffer as material input {}", i);
        return Quartz_Failure;
      }

      values[i].buffer = inputs[i].value.buffer.m_opalBuffer;
      infos[i].type = Opal_Input_Type_Uniform_Buffer;
    } break;
    default: return Quartz_Failure;
    }
  }

  if (m_isBase)
  {
    OpalInputLayoutInitInfo layoutInfo;
    layoutInfo.count = infos.size();
    layoutInfo.pInputs = infos.data();

    QTZ_ATTEMPT_OPAL(OpalInputLayoutInit(&m_layout, layoutInfo));
  }

  OpalInputSetInitInfo setInfo;
  setInfo.layout = m_layout;
  setInfo.pInputValues = values.data();

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_set, setInfo));
  return Quartz_Success;
}

QuartzResult Material::InitShaders(const std::vector<std::string>& shaderPaths)
{
  char* fileBuffer;

  if (!m_shaders.size())
  {
    m_shaders.resize(2);
  }

  OpalShaderInitInfo shaderInfo[2];
  shaderInfo[0].type = Opal_Shader_Vertex;
  shaderInfo[0].size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[0].c_str());
  shaderInfo[0].pSource = fileBuffer;
  QTZ_ATTEMPT_OPAL(OpalShaderInit(&m_shaders[0], shaderInfo[0]));
  free(fileBuffer);

  shaderInfo[1].type = Opal_Shader_Fragment;
  shaderInfo[1].size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[1].c_str());
  shaderInfo[1].pSource = fileBuffer;
  QTZ_ATTEMPT_OPAL(OpalShaderInit(&m_shaders[1], shaderInfo[1]));
  free(fileBuffer);

  return Quartz_Success;
}

QuartzResult Material::InitMaterial()
{
  const uint32_t layoutCount = 2;
  OpalInputLayout layouts[layoutCount] = {
    Renderer::SceneLayout(),
    m_layout
  };

  OpalMaterialInitInfo materialInfo;
  materialInfo.renderpass = g_coreState.renderer.GetRenderpass();
  materialInfo.subpassIndex = 0;
  materialInfo.inputLayoutCount = layoutCount;
  materialInfo.pInputLayouts = layouts;
  materialInfo.shaderCount = m_shaders.size();
  materialInfo.pShaders = m_shaders.data();
  materialInfo.pushConstantSize = sizeof(Mat4);

  materialInfo.pipelineSettings = (OpalPipelineSettingFlags)m_pipelineSettings;


  QTZ_ATTEMPT_OPAL(OpalMaterialInit(&m_material, materialInfo));

  return Quartz_Success;
}

QuartzResult Material::Bind() const
{
  if (!m_isValid)
  {
    QTZ_ERROR("Attempting to use an invalid material");
    return Quartz_Failure;
  }

  OpalMaterial mat = (m_isBase) ? m_material : m_parent->m_material;

  OpalRenderBindMaterial(mat);
  OpalRenderBindInputSet(Renderer::SceneSet(), 0);
  OpalRenderBindInputSet(m_set, 1);

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

  if (m_isBase)
  {
    OpalMaterialShutdown(&m_material);
    for (uint32_t i = 0; i < m_shaders.size(); i++)
    {
      OpalShaderShutdown(&m_shaders[i]);
    }

    OpalInputLayoutShutdown(&m_layout);
  }

  OpalInputSetShutdown(&m_set);

  m_isValid = false;
}

QuartzResult Material::Reload()
{
  if (!m_isValid)
  {
    QTZ_WARNING("Attempting to reload an invalid material");
    return Quartz_Success;
  }

  if (!m_isBase)
  {
    m_parent->Reload();
  }
  else
  {
    OpalWaitIdle();

    OpalMaterialShutdown(&m_material);
    QTZ_ATTEMPT(InitShaders(m_shaderPaths));
    QTZ_ATTEMPT(InitMaterial());
  }

  return Quartz_Success;
}


} // namespace Quartz
