#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>

namespace Quartz
{

class Buffer
{
  friend class Renderer;
  friend class Material;

public:
  Buffer() : m_isValid(false) {}
  Buffer(uint32_t size);

  QuartzResult Init(uint32_t size);

  void Shutdown();
  QuartzResult PushData(void* data);
  inline bool IsValid() const { return m_isValid; }

private:
  bool m_isValid = false;
  OpalBuffer m_opalBuffer;
};

} // namespace Quartz
