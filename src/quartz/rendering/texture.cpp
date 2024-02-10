
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

QuartzResult Texture::Init()
{
  QTZ_ATTEMPT(InitOpalImage());

  return Quartz_Success;
}

// Loaded
// ============================================================

QuartzResult Texture::Init(const char* path)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  int32_t width, height;
  void* pixels;

  if (format == Texture_Format_RGBA8)
  {
    QTZ_ATTEMPT(Load8BitImage(path, &width, &height, &pixels));
  }
  else
  {
    QTZ_ATTEMPT(Load32BitImage(path, &width, &height, &pixels));
  }

  extents = Vec2U{ (uint32_t)width, (uint32_t)height };
  QTZ_ATTEMPT(InitOpalImage());
  QTZ_ATTEMPT(FillImage(pixels));

  free(pixels);

  return Quartz_Success;
}

QuartzResult Texture::Load8BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels)
{
  int width = 0, height = 0;
  int fileChannelCount;
  int requestedChannelCount = STBI_rgb_alpha;
  stbi_uc* source = stbi_load(path, &width, &height, &fileChannelCount, requestedChannelCount);

  if (width <= 0 || height <= 0 || source == nullptr)
  {
    if (source != nullptr)
    {
      stbi_image_free(source);
    }

    QTZ_ERROR("Failed to load 8-bit image \"{}\"\n    Width: {} -- Height: {} -- Channels: {}", path, width, height, requestedChannelCount);
    return Quartz_Failure_Vendor;
  }

  *outWidth = width;
  *outHeight = height;
  *outPixels = source;
  return Quartz_Success;
}

QuartzResult Texture::Load32BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels)
{
  float* pixelData = nullptr;
  int width = 0, height = 0;
  const char* err;

  if (LoadEXR(&pixelData, &width, &height, path, &err) || width <= 0 || height <= 0 || pixelData == nullptr)
  {
    QTZ_ERROR("Failed to load 32-bit image \"{}\"\n    Width: {} -- Height: {} -- Channels: {}\n    Error: {}", path, width, height, 4, err);
    width = 0;
    return Quartz_Failure;
  }

  *outWidth = width;
  *outHeight = height;
  *outPixels = pixelData;
  return Quartz_Success;
}

// Assembled
// ============================================================

QuartzResult Texture::Init(const void* pixels)
{
  QTZ_ATTEMPT(InitOpalImage());
  QTZ_ATTEMPT(FillImage(pixels));

  return Quartz_Success;
}

QuartzResult Texture::Init(const std::vector<Vec3>& pixels)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  void* pixelData;
  std::vector<Vec4> pixelsRgba32;
  std::vector<unsigned char> pixels8Bit;

  switch (format)
  {
  case Quartz::Texture_Format_RGBA8:
  {
    uint32_t count = extents.width * extents.height * 4;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < extents.width * extents.height; i++, channelIndex += 4)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 3] = (uint8_t)(255);
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_Format_RGBA32:
  {
    pixelsRgba32.resize(pixels.size());

    for (uint32_t i = 0; i < extents.width * extents.height; i++)
    {
      pixelsRgba32[i] = Vec4{ pixels[i].x, pixels[i].y, pixels[i].z, 0.0f };
    }

    pixelData = (void*)pixelsRgba32.data();
  } break;
  }

  QTZ_ATTEMPT(InitOpalImage());
  QTZ_ATTEMPT(FillImage(pixelData));

  return Quartz_Success;
}

QuartzResult Texture::Init(const std::vector<Vec4>& pixels)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  void* pixelData;
  std::vector<Vec3> pixelsRgb32;
  std::vector<unsigned char> pixels8Bit;

  switch (format)
  {
  case Quartz::Texture_Format_RGBA8:
  {
    uint32_t count = extents.width * extents.height * 4;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < extents.width * extents.height; i++, channelIndex += 4)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 3] = (uint8_t)(255 * PeriClamp(pixels[i].w, 0.0f, 1.0f));
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_Format_RGBA32:
  {
    pixelData = (void*)pixels.data();
  } break;
  }

  QTZ_ATTEMPT(InitOpalImage());
  QTZ_ATTEMPT(FillImage(pixelData));

  return Quartz_Success;
}

// Opal
// ============================================================

QuartzResult Texture::Init(OpalImage opalImage)
{
  m_opalImage = opalImage;

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Texture::InitOpalImage()
{
  OpalImageInitInfo info = {};
  info.extent = OpalExtent{ extents.width, extents.height, 1 };

  if (mipLevels == 0)
  {
    mipLevels = (uint32_t)floor(log2((double)PeriMax(extents.width, extents.height)));
  }
  info.mipLevels = mipLevels;

  info.usage |= ((usage & Texture_Usage_Shader_Input) != 0) * Opal_Image_Usage_Uniform;
  info.usage |= ((usage & Texture_Usage_Framebuffer) != 0) * (format == Texture_Format_Depth ? Opal_Image_Usage_Depth : Opal_Image_Usage_Color);

  switch (format)
  {
  case Quartz::Texture_Format_RG16:   info.format = Opal_Format_RG16;   break;
  case Quartz::Texture_Format_RGBA8:  info.format = Opal_Format_RGBA8;  break;
  case Quartz::Texture_Format_RGBA32: info.format = Opal_Format_RGBA32; break;
  case Quartz::Texture_Format_Depth:  info.format = Opal_Format_D24_S8; break;
  }

  switch (filtering)
  {
  case Quartz::Texture_Filter_Linear: info.filterType = Opal_Image_Filter_Bilinear; break;
  case Quartz::Texture_Filter_Nearest: info.filterType = Opal_Image_Filter_Point; break;
  }

  switch (sampleMode)
  {
  case Quartz::Texture_Sample_Wrap: info.sampleMode = Opal_Image_Sample_Wrap; break;
  case Quartz::Texture_Sample_Clamp: info.sampleMode = Opal_Image_Sample_Clamp; break;
  case Quartz::Texture_Sample_Reflect: info.sampleMode = Opal_Image_Sample_Reflect; break;
  }

  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_opalImage, info));

  if (usage & Texture_Usage_Shader_Input)
  {
    OpalInputValue inValue = {};
    inValue.image = m_opalImage;

    OpalInputSetInitInfo setInfo = {};
    setInfo.layout = g_coreState.renderer.GetSingleImageLayout();
    setInfo.pInputValues = &inValue;

    QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_inputSet, setInfo));

    OpalInputInfo imageInput = {};
    imageInput.type = Opal_Input_Type_Samped_Image;
    imageInput.value.image = m_opalImage;
    imageInput.index = 0;

    QTZ_ATTEMPT_OPAL(OpalInputSetUpdate(m_inputSet, 1, &imageInput));
  }

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Texture::FillImage(const void* pixels)
{
  if (usage & Texture_Usage_Framebuffer)
  {
    QTZ_ERROR("Can not manually fill a framebuffer texture");
    return Quartz_Failure;
  }

  QTZ_ATTEMPT_OPAL(OpalImageFill(m_opalImage, pixels));

  return Quartz_Success;
}

// Other
// ============================================================

QuartzResult Texture::Resize(Vec2U newExtents)
{
  if (!m_isValid)
  {
    QTZ_ERROR("Attempting to resize an invalid texture");
    return Quartz_Failure;
  }

  OpalExtent e = OpalExtent{ newExtents.width, newExtents.height, 1 };
  QTZ_ATTEMPT_OPAL(OpalImageResize(m_opalImage, e));

  extents = newExtents;

  return Quartz_Success;
}

} // namespace Quartz