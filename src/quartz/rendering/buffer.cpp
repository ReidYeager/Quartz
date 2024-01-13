
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/buffer.h"

namespace Quartz
{

void Buffer::Shutdown()
{
  OpalBufferShutdown(&m_opalBuffer);
}

QuartzResult Buffer::Init(uint32_t size)
{
  OpalBufferInitInfo info = {};
  info.size = size;
  info.usage = Opal_Buffer_Usage_Uniform;

  QTZ_ATTEMPT_OPAL(OpalBufferInit(&m_opalBuffer, info));
  return Quartz_Success;
}

QuartzResult Buffer::PushData(void* data)
{
  QTZ_ATTEMPT_OPAL(OpalBufferPushData(m_opalBuffer, data));
  return Quartz_Success;
}

} // namespace Quartz
