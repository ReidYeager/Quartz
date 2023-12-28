#include "quartz/defines.h"
#include "quartz/render/render.h"
#include "quartz/platform/window/window.h"

#include "quartz/render/camera.h"

#include <opal.h>

namespace Quartz
{

Renderer* Renderer::m_instance = nullptr;

QuartzResult Renderer::Init(Window* window)
{
  if (m_instance != nullptr)
  {
    QTZ_LOG_CORE_WARNING("Attempting to init renderer multiple times");
    return Quartz_Success;
  }
  m_instance = this;

  const uint32_t vertexElementCount = 3;
  OpalFormat vertexElementTypes[vertexElementCount] = {
    Opal_Format_RGB32, // Position
    Opal_Format_RG32,  // Uv
    Opal_Format_RGB32  // Normal
  };

  OpalInitInfo opalInfo {};
  opalInfo.debug = true;
  opalInfo.vertexStruct.count = vertexElementCount;
  opalInfo.vertexStruct.pFormats = vertexElementTypes;

  OpalWindowInitInfo windowInfo {};
  windowInfo.extents = { window->Width(), window->Height() };

  const WindowPlatformInfo* platformInfo = window->GetPlatformInfo();
#ifdef QTZ_PLATFORM_WIN32
  opalInfo.windowPlatformInfo.hinstance = platformInfo->hinstance;
  windowInfo.platformInfo.hinstance = platformInfo->hinstance;
  opalInfo.windowPlatformInfo.hwnd = platformInfo->hwnd;
  windowInfo.platformInfo.hwnd = platformInfo->hwnd;
#endif

  QTZ_ATTEMPT_OPAL(OpalInit(opalInfo));

  QTZ_ATTEMPT_OPAL(OpalWindowInit(&m_window, windowInfo));
  OpalWindowGetBufferImage(m_window, &m_windowBufferImage);

  OpalAttachmentInfo attachments[1];
  attachments[0].clearValue.color = (OpalColorValue){ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[0].format = Opal_Format_BGRA8;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].usage = Opal_Attachment_Usage_Presented;

  uint32_t colorIndex = 0;
  OpalSubpassInfo subpass;
  subpass.depthAttachmentIndex = OPAL_DEPTH_ATTACHMENT_NONE;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = &colorIndex;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = nullptr;

  OpalRenderpassInitInfo renderpassInfo;
  renderpassInfo.dependencyCount = 0;
  renderpassInfo.pDependencies = nullptr;
  renderpassInfo.imageCount = 1;
  renderpassInfo.pAttachments = attachments;
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&m_renderpass, renderpassInfo));

  OpalImage images[1] = { m_windowBufferImage };
  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.pImages = images;
  framebufferInfo.renderpass = m_renderpass;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_framebuffer, framebufferInfo));



  return Quartz_Success;
}

void Renderer::Shutdown()
{
  OpalFramebufferShutdown(&m_framebuffer);
  OpalRenderpassShutdown(&m_renderpass);
  OpalWindowShutdown(&m_window);
  OpalShutdown();
}

QuartzResult Renderer::Resize(uint32_t width, uint32_t height)
{
  if (width == 0 || height == 0)
  {
    return Quartz_Success;
  }

  QTZ_ATTEMPT_OPAL(OpalWindowReinit(m_window));
  QTZ_ATTEMPT_OPAL(OpalFramebufferReinit(m_framebuffer));

  return Quartz_Success;
}

QuartzResult Renderer::StartFrame()
{
  QTZ_ATTEMPT_OPAL(OpalRenderBegin(m_window));
  return Quartz_Success;
}

QuartzResult Renderer::Render()
{
  OpalRenderBeginRenderpass(m_renderpass, m_framebuffer);
  OpalRenderEndRenderpass(m_renderpass);
  return Quartz_Success;
}

QuartzResult Renderer::EndFrame()
{
  QTZ_ATTEMPT_OPAL(OpalRenderEnd());

  return Quartz_Success;
}

} // namespace Quartz
