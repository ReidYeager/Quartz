
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

// Loaded
// ============================================================

QuartzResult Texture::Init(TextureFormat format, const char* path)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  if (format == Texture_RGB32 || format == Texture_RGB8)
  {
    QTZ_ERROR("RGBxx textures not supported for uniforms");
    return Quartz_Failure;
  }

  m_format = format;

  int32_t width, height;
  void* pixels;

  if (m_format == Texture_RGB8 || m_format == Texture_RGBA8)
  {
    QTZ_ATTEMPT(Load8BitImage(path, &width, &height, &pixels));
  }
  else
  {
    QTZ_ATTEMPT(Load32BitImage(path, &width, &height, &pixels));
  }

  QTZ_ATTEMPT(InitOpalImage((uint32_t)width, (uint32_t)height, pixels));

  free(pixels);

  return Quartz_Success;
}

QuartzResult Texture::Load8BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels)
{
  int width = 0, height = 0;
  int fileChannelCount;
  int requestedChannelCount = (m_format == Texture_RGB8) ? STBI_rgb : STBI_rgb_alpha;
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
  if (m_format == Texture_RGB32)
  {
    QTZ_ERROR("RGB32 texture loading not supported by tinyexr");
    return Quartz_Failure;
  }

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

QuartzResult Texture::Init(TextureFormat format, uint32_t width, uint32_t height, const std::vector<Vec3>& pixels)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  if (format == Texture_RGB32 || format == Texture_RGB8)
  {
    QTZ_ERROR("RGBxx textures not supported for uniforms");
    return Quartz_Failure;
  }

  m_format = format;

  void* pixelData;
  std::vector<Vec4> pixelsRgba32;
  std::vector<unsigned char> pixels8Bit;

  switch (m_format)
  {
  case Quartz::Texture_RGB8:
  {
    uint32_t count = width * height * 3;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < width * height; i++, channelIndex += 3)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_RGBA8:
  {
    uint32_t count = width * height * 4;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < width * height; i++, channelIndex += 4)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 3] = (uint8_t)(255);
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_RGB32:
  {
    pixelsRgba32.resize(pixels.size());

    for (uint32_t i = 0; i < width * height; i++)
    {
      pixelsRgba32[i] = Vec4{ pixels[i].x, pixels[i].y, pixels[i].z, 0.0f };
    }

    pixelData = (void*)pixelsRgba32.data();
  } break;
  case Quartz::Texture_RGBA32:
  {
    pixelData = (void*)pixels.data();
  } break;
  }

  QTZ_ATTEMPT(InitOpalImage(width, height, pixelData));

  return Quartz_Success;
}

QuartzResult Texture::Init(TextureFormat format, uint32_t width, uint32_t height, const std::vector<Vec4>& pixels)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to intialize a valid texture");
    return Quartz_Success;
  }

  if (format == Texture_RGB32 || format == Texture_RGB8)
  {
    QTZ_ERROR("RGBxx textures not supported for uniforms");
    return Quartz_Failure;
  }

  m_format = format;

  void* pixelData;
  std::vector<Vec3> pixelsRgb32;
  std::vector<unsigned char> pixels8Bit;

  switch (m_format)
  {
  case Quartz::Texture_RGB8:
  {
    uint32_t count = width * height * 3;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < width * height; i++, channelIndex += 3)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_RGBA8:
  {
    uint32_t count = width * height * 4;
    pixels8Bit.resize(count);

    for (uint32_t i = 0, channelIndex = 0; i < width * height; i++, channelIndex += 4)
    {
      pixels8Bit[channelIndex    ] = (uint8_t)(255 * PeriClamp(pixels[i].x, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 1] = (uint8_t)(255 * PeriClamp(pixels[i].y, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 2] = (uint8_t)(255 * PeriClamp(pixels[i].z, 0.0f, 1.0f));
      pixels8Bit[channelIndex + 3] = (uint8_t)(255 * PeriClamp(pixels[i].w, 0.0f, 1.0f));
    }

    pixelData = (void*)pixels8Bit.data();
  } break;
  case Quartz::Texture_RGB32:
  {
    pixelsRgb32.resize(pixels.size());

    for (uint32_t i = 0; i < width * height; i++)
    {
      pixelsRgb32[i] = Vec3{ pixels[i].x, pixels[i].y, pixels[i].z };
    }

    pixelData = (void*)pixelsRgb32.data();
  } break;
  case Quartz::Texture_RGBA32:
  {
    pixelData = (void*)pixels.data();
  } break;
  }

  QTZ_ATTEMPT(InitOpalImage(width, height, pixelData));

  return Quartz_Success;
}

// Opal
// ============================================================

QuartzResult Texture::InitOpalImage(uint32_t width, uint32_t height, void* pixels)
{
  OpalImageInitInfo info = {};
  info.extent = OpalExtent{ width, height, 1 };
  info.sampleType = Opal_Sample_Bilinear;
  info.usage = Opal_Image_Usage_Uniform;
  info.mipLevels = (uint32_t)floor(log2((double)PeriMax(width, height)));

  switch (m_format)
  {
  case Quartz::Texture_RGB8:   info.format = Opal_Format_RGB8;   break;
  case Quartz::Texture_RGBA8:  info.format = Opal_Format_RGBA8;  break;
  case Quartz::Texture_RGB32:  info.format = Opal_Format_RGB32;  break;
  case Quartz::Texture_RGBA32: info.format = Opal_Format_RGBA32; break;
  }

  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_opalImage, info));
  QTZ_ATTEMPT_OPAL(OpalImageFill(m_opalImage, pixels));

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

  m_isValid = true;
  return Quartz_Success;
}

} // namespace Quartz