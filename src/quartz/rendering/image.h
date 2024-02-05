#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

namespace Quartz
{

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
};

} // namespace Quartz
