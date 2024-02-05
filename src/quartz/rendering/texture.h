#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>

namespace Quartz
{

enum TextureFormat
{
  Texture_RGB8,
  Texture_RGBA8,
  Texture_RGB32,
  Texture_RGBA32,
};

class Image
{
  friend class Renderer;
  friend class Material;

protected:
  OpalImage m_opalImage = OPAL_NULL_HANDLE;
  OpalInputSet m_inputSet = OPAL_NULL_HANDLE;
  bool m_isValid = false;

public:
  virtual ~Image() = default;

  virtual void Shutdown()
  {
    if (!m_isValid)
    {
      return;
    }
    m_isValid = false;
    OpalImageShutdown(&m_opalImage);
    OpalInputSetShutdown(&m_inputSet);
  }

  inline bool IsValid() const
  {
    return m_isValid;
  }

  inline void* ForImgui() const
  {
    if (!m_isValid)
    {
      QTZ_ERROR("Attempting to use invalid image for imgui");
      return nullptr;
    }
    return (void*)&m_inputSet->vk.descriptorSet;
  }

private:
  inline OpalImage GetOpalImage() const { return m_opalImage; };
  inline OpalInputSet GetInputSet() const { return m_inputSet; }
};

class Texture : public Image
{
private:
  TextureFormat m_format;

public:
  QuartzResult Init(TextureFormat format, const char* path);
  QuartzResult Init(TextureFormat format, uint32_t width, uint32_t height, const std::vector<Vec3>& pixels);
  QuartzResult Init(TextureFormat format, uint32_t width, uint32_t height, const std::vector<Vec4>& pixels);

private:
  QuartzResult Load8BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult Load32BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult InitOpalImage(uint32_t width, uint32_t height, void* pixels);
};

} // namespace Quartz
