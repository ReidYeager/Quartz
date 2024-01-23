
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/buffer.h"

namespace Quartz
{

void Buffer::Shutdown()
{
  if (!m_isValid)
  {
    return;
  }

  m_isValid = false;
  OpalBufferShutdown(&m_opalBuffer);
}

Buffer::Buffer(uint32_t size) : m_isValid(false)
{
  QTZ_ATTEMPT_VOID(Init(size));
}

QuartzResult Buffer::Init(uint32_t size)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid buffer");
    return Quartz_Success;
  }

  OpalBufferInitInfo info = {};
  info.size = size;
  info.usage = Opal_Buffer_Usage_Uniform;

  QTZ_ATTEMPT_OPAL(OpalBufferInit(&m_opalBuffer, info));

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Buffer::PushData(void* data)
{
  if (!m_isValid)
  {
    QTZ_WARNING("Attempting to push data to an invalid bufffer");
    return Quartz_Failure;
  }

  QTZ_ATTEMPT_OPAL(OpalBufferPushData(m_opalBuffer, data));
  return Quartz_Success;
}

} // namespace Quartz
