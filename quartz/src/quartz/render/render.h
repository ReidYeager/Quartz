#pragma once

#include "quartz/defines.h"
#include "quartz/platform/window/window.h"

#include "quartz/render/defines.h"
#include "quartz/render/camera.h"

namespace Quartz
{

class Renderer
{
public:
  static Renderer* Get() { return m_instance; }
  QuartzResult Init(Window* window);
  void Shutdown();

  QuartzResult Resize(uint32_t width, uint32_t height);

  QuartzResult StartFrame();
  QuartzResult Render();
  QuartzResult EndFrame();

private:
  static Renderer* m_instance;
  OpalWindow m_window;
  OpalImage m_windowBufferImage;
  OpalRenderpass m_renderpass;
  OpalFramebuffer m_framebuffer;

};

} // namespace Quartz
