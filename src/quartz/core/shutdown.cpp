
#include "quartz/core/core.h"

namespace Quartz
{

void CoreShutdown()
{
  QTZ_DEBUG(
    "Average frame time : {} ms : {} frames",
    (g_coreState.time.deltaSum / g_coreState.time.frameIndex) * 1000,
    g_coreState.time.frameIndex);

  g_coreState.clientApp->Shutdown();

  for (auto iterator = g_coreState.layerStack.EndIterator(); iterator != g_coreState.layerStack.BeginIterator();)
  {
    iterator--;
    (*iterator)->OnDetach();
  }

  g_coreState.renderer.Shutdown();
  g_coreState.mainWindow.Shutdown();
}

} // namespace Quartz
