
#include "quartz/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/platform/filesystem/filesystem.h"
#include "quartz/rendering/rendering.h"

namespace Quartz
{
QuartzResult Material::Init(OpalRenderpass renderpass, const std::vector<const char*>& shaderPaths)
{
  QTZ_ATTEMPT(InitInputLayout());
  QTZ_ATTEMPT(InitBuffer());
  QTZ_ATTEMPT(InitInputSet());
  QTZ_ATTEMPT(InitShaders(shaderPaths));
  QTZ_ATTEMPT(InitMaterial(renderpass));

  m_valid = true;
  return Quartz_Success;
}

QuartzResult Material::InitInputLayout()
{
  OpalInputAccessInfo inputs[1] = {
      { Opal_Input_Type_Uniform_Buffer, Opal_Stage_All_Graphics }
  };

  OpalInputLayoutInitInfo layoutInfo;
  layoutInfo.count = 1;
  layoutInfo.pInputs = inputs;

  QTZ_ATTEMPT_OPAL(OpalInputLayoutInit(&m_layout, layoutInfo));

  return Quartz_Success;
}

QuartzResult Material::InitBuffer()
{
  OpalBufferInitInfo bufferInfo;
  bufferInfo.size = sizeof(Mat4);
  bufferInfo.usage = Opal_Buffer_Usage_Uniform;

  QTZ_ATTEMPT_OPAL(OpalBufferInit(&m_buffer, bufferInfo));
  return Quartz_Success;
}

QuartzResult Material::InitInputSet()
{
  OpalInputValue values[1];
  values[0].buffer = m_buffer;

  OpalInputSetInitInfo setInfo;
  setInfo.layout = m_layout;
  setInfo.pInputValues = values;

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_set, setInfo));
  return Quartz_Success;
}

QuartzResult Material::InitShaders(const std::vector<const char*>& shaderPaths)
{
  char* fileBuffer;

  OpalShaderInitInfo shaderInfo[2];
  shaderInfo[0].type = Opal_Shader_Vertex;
  shaderInfo[0].size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[0]);
  shaderInfo[0].pSource = fileBuffer;
  QTZ_ATTEMPT_OPAL(OpalShaderInit(&m_shaders[0], shaderInfo[0]));
  free(fileBuffer);

  shaderInfo[1].type = Opal_Shader_Fragment;
  shaderInfo[1].size = PlatformLoadFile((void**)&fileBuffer, shaderPaths[1]);
  shaderInfo[1].pSource = fileBuffer;
  QTZ_ATTEMPT_OPAL(OpalShaderInit(&m_shaders[1], shaderInfo[1]));
  free(fileBuffer);

  return Quartz_Success;
}

QuartzResult Material::InitMaterial(OpalRenderpass renderpass)
{
  const uint32_t layoutCount = 2;
  OpalInputLayout layouts[layoutCount] = {
    Renderer::SceneLayout(),
    m_layout
  };

  OpalMaterialInitInfo materialInfo;
  materialInfo.renderpass = renderpass;
  materialInfo.subpassIndex = 0;
  materialInfo.inputLayoutCount = layoutCount;
  materialInfo.pInputLayouts = layouts;
  materialInfo.shaderCount = 2;
  materialInfo.pShaders = m_shaders;
  materialInfo.pushConstantSize = sizeof(Mat4);

  QTZ_ATTEMPT_OPAL(OpalMaterialInit(&m_material, materialInfo));

  OpalShaderShutdown(&m_shaders[0]);
  OpalShaderShutdown(&m_shaders[1]);
  OpalInputLayoutShutdown(&m_layout);

  return Quartz_Success;
}

QuartzResult Material::PushData(void* data)
{
  QTZ_ATTEMPT_OPAL(OpalBufferPushData(m_buffer, data));
  return Quartz_Success;
}

QuartzResult Material::Bind() const
{
  OpalRenderBindMaterial(m_material);
  OpalRenderBindInputSet(Renderer::SceneSet(), 0);
  OpalRenderBindInputSet(m_set, 1);
  return Quartz_Success;
}

void Material::Shutdown()
{
  OpalMaterialShutdown(&m_material);
  OpalInputSetShutdown(&m_set);
  OpalBufferShutdown(&m_buffer);

  m_valid = false;
}


} // namespace Quartz
