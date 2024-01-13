#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>
#include <peridot.h>

namespace Quartz
{

class Texture
{
  friend class Renderer;

public:
  void Shutdown();
  void* ForImgui();

private:
  OpalImage m_opalImage;
  OpalInputSet m_imguiSet;

  QuartzResult Init(const char* path, OpalInputLayout imguiLayout);
  //QuartzResult Init(const std::vector<Vec4>& pixels);
  //QuartzResult Init(const std::vector<Vec3>& pixels);
};

} // namespace Quartz
