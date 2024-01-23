#pragma once

#include "quartz/defines.h"

namespace Quartz
{

class Application
{
public:
  virtual QuartzResult Init()                   = 0;
  virtual QuartzResult Update(double deltaTime) = 0;
  virtual void         Shutdown()               = 0;
  virtual void         RenderImgui()            = 0;
};

} // namespace Quartz
