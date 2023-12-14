
#include "quartz.h"

#include <stdio.h>
#include <memory>

namespace Quartz
{

Application* Application::m_instance = NULL;

Application::Application()
{
  if (m_instance != NULL)
  {
    QTZ_LOG_CORE_ERROR("Application already exists\n");
    return;
  }

  m_instance = this;

  Quartz::Logger logger;
  logger.Init();

  QTZ_LOG_CORE_INFO("Application init complete");
}

} // namespace Quartz
