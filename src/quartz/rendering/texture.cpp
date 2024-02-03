
#include "quartz/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/platform/filesystem/filesystem.h"
#include "quartz/rendering/renderer.h"
#include "quartz/core/core.h"

#define STBI_SUPPORT_ZLIB
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

namespace Quartz
{

Texture::Texture(const char* path, bool isExr)
{
  if (!isExr)
  {
    QTZ_ATTEMPT_VOID(Init(path));
  }
  else
  {
    QTZ_ATTEMPT_VOID(InitExr(path));
  }
}

void Texture::Shutdown()
{
  if (!m_isValid)
  {
    return;
  }

  m_isValid = false;
  OpalImageShutdown(&m_opalImage);
  OpalInputSetShutdown(&m_imguiSet);
}

void* Texture::ForImgui()
{
  if (!m_isValid)
  {
    QTZ_ERROR("Attempting to use invalid texture for imgui");
    return nullptr;
  }

  return (void*)m_imguiSet->vk.descriptorSet;
}

QuartzResult Texture::Init(const char* path)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  int32_t width, height, channels;
  stbi_uc* source = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

  if (width <= 0 || height <= 0 || source == nullptr)
  {
    QTZ_ERROR("Failed to load the image (\"{}\")", path);
    return Quartz_Failure_Vendor;
  }

  std::vector<unsigned char> pixels(source, source + (width * height * 4));
  QTZ_ATTEMPT(Init((uint32_t)width, (uint32_t)height, pixels));

  stbi_image_free(source);

  return Quartz_Success;
}

QuartzResult Texture::Init(uint32_t width, uint32_t height, const std::vector<Vec3>& pixels)
{
  if (width * height != pixels.size())
  {
    QTZ_ERROR("Initializing texture with incorrect number of pixels ({} should be {})", pixels.size(), width * height);
    return Quartz_Failure;
  }

  std::vector<unsigned char> rgba8(width * height * 4);

  for (uint32_t i = 0; i < pixels.size(); i++)
  {
    uint32_t index = i * 4;

    rgba8[index + 0] = PeriClamp((uint8_t)(255 * pixels[i].r), 0, 255);
    rgba8[index + 1] = PeriClamp((uint8_t)(255 * pixels[i].g), 0, 255);
    rgba8[index + 2] = PeriClamp((uint8_t)(255 * pixels[i].b), 0, 255);
    rgba8[index + 3] = PeriClamp((uint8_t)(255)              , 0, 255);
  }

  QTZ_ATTEMPT(Init(width, height, rgba8));
  return Quartz_Success;
}

QuartzResult Texture::Init(uint32_t width, uint32_t height, const std::vector<Vec4>& pixels)
{
  if (width * height != pixels.size())
  {
    QTZ_ERROR("Initializing texture with incorrect number of pixels ({} should be {})", pixels.size(), width * height);
    return Quartz_Failure;
  }

  std::vector<unsigned char> rgba8(width * height * 4);

  for (uint32_t i = 0; i < pixels.size(); i++)
  {
    uint32_t index = i * 4;

    rgba8[index + 0] = PeriClamp((uint8_t)(255 * pixels[i].r), 0, 255);
    rgba8[index + 1] = PeriClamp((uint8_t)(255 * pixels[i].g), 0, 255);
    rgba8[index + 2] = PeriClamp((uint8_t)(255 * pixels[i].b), 0, 255);
    rgba8[index + 3] = PeriClamp((uint8_t)(255 * pixels[i].a), 0, 255);
  }

  QTZ_ATTEMPT(Init(width, height, rgba8));
  return Quartz_Success;
}

QuartzResult Texture::Init(uint32_t width, uint32_t height, const std::vector<unsigned char>& pixels)
{
  OpalImageInitInfo info = {};
  info.extent = OpalExtent{ (uint32_t)width, (uint32_t)height, 1 };
  info.sampleType = Opal_Sample_Bilinear;
  info.usage = Opal_Image_Usage_Uniform;
  info.format = Opal_Format_RGBA8;

  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_opalImage, info));
  QTZ_ATTEMPT_OPAL(OpalImageFill(m_opalImage, (void*)pixels.data()));

  OpalInputValue inValue = {};
  inValue.image = m_opalImage;

  OpalInputSetInitInfo setInfo = {};
  setInfo.layout = g_coreState.renderer.GetSingleImageLayout();
  setInfo.pInputValues = &inValue;

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_imguiSet, setInfo));

  OpalInputInfo imageInput = {};
  imageInput.type = Opal_Input_Type_Samped_Image;
  imageInput.value.image = m_opalImage;
  imageInput.index = 0;

  QTZ_ATTEMPT_OPAL(OpalInputSetUpdate(m_imguiSet, 1, &imageInput));

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Texture::InitExr(const char* path)
{
  int width, height;
  float* source = nullptr;
  const char* err = nullptr;

  if (LoadEXR(&source, &width, &height, path, &err))
  {
    QTZ_ERROR("Failed to load .exr file (\"{}\")\n    Error : \"{}\"", path, err);
    return Quartz_Failure;
  }

  std::vector<float> pixels(source, source + (width * height * 4));
  QTZ_ATTEMPT(InitHdr((uint32_t)width, (uint32_t)height, pixels));

  free(source);

  return Quartz_Success;
}

QuartzResult Texture::InitHdr(uint32_t width, uint32_t height, const std::vector<float>& pixels)
{
  OpalImageInitInfo info = {};
  info.extent = OpalExtent{ (uint32_t)width, (uint32_t)height, 1 };
  info.sampleType = Opal_Sample_Bilinear;
  info.usage = Opal_Image_Usage_Uniform;
  info.format = Opal_Format_RGBA32;

  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_opalImage, info));
  QTZ_ATTEMPT_OPAL(OpalImageFill(m_opalImage, (void*)pixels.data()));

  OpalInputValue inValue = {};
  inValue.image = m_opalImage;

  OpalInputSetInitInfo setInfo = {};
  setInfo.layout = g_coreState.renderer.GetSingleImageLayout();
  setInfo.pInputValues = &inValue;

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_imguiSet, setInfo));

  OpalInputInfo imageInput = {};
  imageInput.type = Opal_Input_Type_Samped_Image;
  imageInput.value.image = m_opalImage;
  imageInput.index = 0;

  QTZ_ATTEMPT_OPAL(OpalInputSetUpdate(m_imguiSet, 1, &imageInput));

  m_isValid = true;
  return Quartz_Success;
}

} // namespace Quartz