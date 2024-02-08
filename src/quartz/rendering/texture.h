#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/image.h"

#include <opal.h>

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
  Texture_Format_Depth,
};

enum TextureUsageFlagBits
{
  Texture_Usage_Shader_Input = 0x01,
  Texture_Usage_Framebuffer = 0x02
};
typedef uint32_t TextureUsageFlags;

class Texture : public Image
{
friend class Renderer;
friend class Material;
friend void SetHdri(Quartz::Texture& image);

public:
  TextureFormat format = Texture_Format_RGBA8;
  TextureUsageFlags usage = Texture_Usage_Shader_Input;
  TextureFilterMode filtering = Texture_Filter_Linear;
  TextureSampleMode sampleMode = Texture_Sample_Wrap;
  uint32_t mipLevels = 0;
  Vec2U extents = Vec2U{ 1, 1 };

public:
  QuartzResult Init();
  QuartzResult Init(const char* path);
  QuartzResult Init(const std::vector<Vec3>& pixels);
  QuartzResult Init(const std::vector<Vec4>& pixels);

  // TODO:
  QuartzResult Resize(Vec2U newExtents);

private:
  QuartzResult Init(OpalImage opalImage);
  QuartzResult Load8BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult Load32BitImage(const char* path, int32_t* outWidth, int32_t* outHeight, void** outPixels);
  QuartzResult InitOpalImage();
  QuartzResult FillImage(void* pixels);
};

} // namespace Quartz
