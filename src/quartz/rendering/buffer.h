#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>
#include <peridot.h>

namespace Quartz
{

class Buffer
{
  friend class Renderer;
  friend class Material;

public:
  void Shutdown();
  QuartzResult PushData(void* data);

private:
  OpalBuffer m_opalBuffer;

  QuartzResult Init(uint32_t size);
};

} // namespace Quartz
