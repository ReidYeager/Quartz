
#include "quartz/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/platform/filesystem/filesystem.h"
#include "quartz/rendering/renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Quartz
{

void Texture::Shutdown()
{
  OpalImageShutdown(&m_opalImage);
  OpalInputSetShutdown(&m_imguiSet);
}

void* Texture::ForImgui()
{
  return (void*)m_imguiSet->vk.descriptorSet;
}

QuartzResult Texture::Init(const char* path, OpalInputLayout imguiLayout)
{
  int32_t width, height, channels;
  stbi_uc* source = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

  if (width <= 0 || height <= 0 || source == nullptr)
  {
    QTZ_ERROR("Failed to load the image");
    return Quartz_Failure_Vendor;
  }

  OpalImageInitInfo info = {};
  info.extent = OpalExtent{ (uint32_t)width, (uint32_t)height, 1 };
  info.sampleType = Opal_Sample_Bilinear;
  info.usage = Opal_Image_Usage_Uniform;
  info.format = Opal_Format_RGBA8;

  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_opalImage, info));
  QTZ_ATTEMPT_OPAL(OpalImageFill(m_opalImage, source));

  stbi_image_free(source);

  OpalInputValue inValue = {};
  inValue.image = m_opalImage;

  OpalInputSetInitInfo setInfo = {};
  setInfo.layout = imguiLayout;
  setInfo.pInputValues = &inValue;

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_imguiSet, setInfo));

  OpalInputInfo imageInput = {};
  imageInput.type = Opal_Input_Type_Samped_Image;
  imageInput.value.image = m_opalImage;
  imageInput.index = 0;

  QTZ_ATTEMPT_OPAL(OpalInputSetUpdate(m_imguiSet, 1, &imageInput));

  return Quartz_Success;
}

} // namespace Quartz