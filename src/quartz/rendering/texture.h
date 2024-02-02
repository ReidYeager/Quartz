#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>

namespace Quartz
{

class Texture
{
  friend class Renderer;
  friend class Material;

public:
  Texture() : m_isValid(false) {}
  Texture(const char* path);

  QuartzResult Init(const char* path);
  QuartzResult Init(uint32_t width, uint32_t height, const std::vector<Vec3>& pixels);
  QuartzResult Init(uint32_t width, uint32_t height, const std::vector<Vec4>& pixels);
  QuartzResult Init(uint32_t width, uint32_t height, const std::vector<unsigned char>& pixels);

  void Shutdown();
  void* ForImgui();
  inline bool IsValid() const { return m_isValid; }

private:
  bool m_isValid = false;
  OpalImage m_opalImage;
  OpalInputSet m_imguiSet;
};

} // namespace Quartz
