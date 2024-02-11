#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/mesh.h"

#include <opal.h>

#define STBI_SUPPORT_ZLIB
#include <stb_image.h>

#include <stb_image_write.h>

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#include <tinyexr.h>

namespace Quartz
{

enum TextureSampleMode
{
  Texture_Sample_Wrap,
  Texture_Sample_Clamp,
  Texture_Sample_Reflect
};

enum TextureFilterMode
{
  Texture_Filter_Linear,
  Texture_Filter_Nearest
};

enum TextureFormat
{
  Texture_Format_RGBA8,
  Texture_Format_RGBA32,
  Texture_Format_RG16,
  Texture_Format_Depth,
};

enum TextureUsageFlagBits
{
  Texture_Usage_Shader_Input = 0x01,
  Texture_Usage_Framebuffer = 0x02
};
typedef uint32_t TextureUsageFlags;

class Texture
{
friend class Renderer;
friend class Material;
friend void SetHdri(Quartz::Texture& image);
friend class TextureSkybox;

  // Variables
  // ============================================================

public:
  TextureFormat     format     = Texture_Format_RGBA8;
  TextureUsageFlags usage      = 0;
  TextureFilterMode filtering  = Texture_Filter_Linear;
  TextureSampleMode sampleMode = Texture_Sample_Wrap;
  uint32_t          mipLevels  = 0;
  Vec2U             extents    = Vec2U{ 1, 1 };

private:
  OpalImage    m_opalImage = OPAL_NULL_HANDLE;
  OpalInputSet m_inputSet  = OPAL_NULL_HANDLE;
  bool         m_isValid   = false;

  // Functions
  // ============================================================

public:
  QuartzResult Init();
  QuartzResult Init(const char* path);
  QuartzResult Init(const void* pixels);
  QuartzResult Init(const std::vector<Vec3>& pixels);
  QuartzResult Init(const std::vector<Vec4>& pixels);

  // TODO:
  QuartzResult Resize(Vec2U newExtents);

  void Shutdown()
  {
    if (!m_isValid)
    {
      return;
    }
    m_isValid = false;

    if (m_opalImage != OPAL_NULL_HANDLE)
      OpalImageShutdown(&m_opalImage);
    if (m_inputSet != OPAL_NULL_HANDLE)
      OpalInputSetShutdown(&m_inputSet);
  }

  inline bool IsValid() const
  {
    return m_isValid;
  }

  inline void* ForImgui() const
  {
    if (!m_isValid || m_inputSet == OPAL_NULL_HANDLE)
    {
      QTZ_ERROR("Attempting to use invalid image for imgui");
      return nullptr;
    }
    return (void*)&m_inputSet->vk.descriptorSet;
  }

private:
  QuartzResult Init(OpalImage opalImage);
  QuartzResult Load8BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult Load32BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult InitOpalImage();
  QuartzResult FillImage(const void* pixels);
};

class TextureSkybox
{
  // Variables
  // ============================================================

public:
  TextureFormat     baseFormat = Texture_Format_RGBA32;
  Vec2U             extents = Vec2U{ 1, 1 };

private:
  Texture      m_baseImage;
  Texture      m_diffuseImage;
  Texture      m_specularImage;
  Texture      m_brdfImage;
  bool         m_isValid  = false;

  static const uint32_t m_brdfSize = 512;

  // Functions
  // ============================================================

public:
  QuartzResult Init(const char* path);
  QuartzResult Init(const void* pixels);
  void Shutdown();

  const inline Texture& GetBase()     const { return m_baseImage;     }
  const inline Texture& GetDiffuse()  const { return m_diffuseImage;  }
  const inline Texture& GetSpecular() const { return m_specularImage; }
  const inline Texture& GetBrdf()     const { return m_brdfImage;     }

  inline bool IsValid() const
  {
    return m_isValid;
  }

  inline void* ForImgui() const
  {
    if (!m_isValid || !m_baseImage.m_isValid)
    {
      QTZ_ERROR("Attempting to use invalid image for imgui");
      return nullptr;
    }
    return m_baseImage.ForImgui();
  }

private:
  QuartzResult CreateBase(const void* pixels);
  QuartzResult CreateIbl();

  QuartzResult CreateDiffuse(const Mesh& screenQuadMesh);
  QuartzResult CreateSpecular(const Mesh& screenQuadMesh);
  QuartzResult CreateBrdf(const Mesh& screenQuadMesh);

};

} // namespace Quartz
