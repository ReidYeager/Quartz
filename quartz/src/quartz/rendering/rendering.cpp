
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/rendering.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/filesystem/filesystem.h"

namespace Quartz
{

OpalInputLayout Renderer::m_sceneLayout;
OpalInputSet Renderer::m_sceneSet;

QuartzResult Renderer::Init(Window* window)
{
  m_qWindow = window;

  const uint32_t vertexFormatCount = 1;
  OpalFormat vertexFormats[vertexFormatCount] = {
    Opal_Format_RGB32, // Position
    //Opal_Format_RG32, // Uv
    //Opal_Format_RGB32, // Normal
  };

  OpalInitInfo opalInfo;
  opalInfo.debug = true;
  opalInfo.vertexStruct.count = vertexFormatCount;
  opalInfo.vertexStruct.pFormats = vertexFormats;

  OpalWindowInitInfo windowInfo;
  windowInfo.extents.width = window->Width();
  windowInfo.extents.height = window->Height();

  const WindowPlatformInfo* platformInfo = window->PlatformInfo();
#ifdef QTZ_PLATFORM_WIN32
  opalInfo.windowPlatformInfo.hinstance = platformInfo->hinstance;
  opalInfo.windowPlatformInfo.hwnd = platformInfo->hwnd;
  windowInfo.platformInfo.hinstance = platformInfo->hinstance;
  windowInfo.platformInfo.hwnd = platformInfo->hwnd;
#endif // QTZ_PLATFORM_WIN32

  QTZ_ATTEMPT_OPAL(OpalInit(opalInfo));
  QTZ_ATTEMPT_OPAL(OpalWindowInit(&m_window, windowInfo));
  OpalWindowGetBufferImage(m_window, &m_windowBufferImage);

  // ==============================
  // Depth image
  // ==============================

  OpalImageInitInfo depthImageInfo {};
  depthImageInfo.extent = m_window->extents;
  depthImageInfo.format = Opal_Format_D24_S8;
  depthImageInfo.sampleType = Opal_Sample_Bilinear;
  depthImageInfo.usage = Opal_Image_Usage_Depth;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&m_depthImage, depthImageInfo));

  // ==============================
  // Renderpass
  // ==============================

  const uint32_t attachmentCount = 2;
  OpalAttachmentInfo attachments[attachmentCount];
  // Window buffer image
  attachments[0].clearValue.color = OpalColorValue{ 0.1f, 0.8f, 0.8f, 1.0f };
  attachments[0].format = Opal_Format_BGR8;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].usage = Opal_Attachment_Usage_Presented;
  // Depth image
  attachments[1].clearValue.depthStencil = OpalDepthStencilValue{ 1, 0 };
  attachments[1].format = Opal_Format_D24_S8;
  attachments[1].loadOp = Opal_Attachment_Op_Clear;
  attachments[1].shouldStore = false;
  attachments[1].usage = Opal_Attachment_Usage_Depth;

  uint32_t zeroIndex = 0;
  OpalSubpassInfo subpass;
  subpass.depthAttachmentIndex = 1;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = &zeroIndex;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = nullptr;

  OpalRenderpassInitInfo renderpassInfo;
  renderpassInfo.dependencyCount = 0;
  renderpassInfo.pDependencies = nullptr;
  renderpassInfo.imageCount = attachmentCount;
  renderpassInfo.pAttachments = attachments;
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;

  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&m_renderpass, renderpassInfo));

  // ==============================
  // Framebuffer
  // ==============================

  OpalImage framebufferImages[attachmentCount] = { m_windowBufferImage, m_depthImage };

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = attachmentCount;
  framebufferInfo.pImages = framebufferImages;
  framebufferInfo.renderpass = m_renderpass;

  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_framebuffer, framebufferInfo));

  // ==============================
  // Scene input set
  // ==============================

  OpalBufferInitInfo sceneBufferInfo = {};
  sceneBufferInfo.size = sizeof(ScenePacket);
  sceneBufferInfo.usage = Opal_Buffer_Usage_Uniform;

  QTZ_ATTEMPT_OPAL(OpalBufferInit(&m_sceneBuffer, sceneBufferInfo));

  OpalInputType sceneInputTypes[1] = { Opal_Input_Type_Uniform_Buffer };

  OpalInputLayoutInitInfo sceneLayoutInfo = {};
  sceneLayoutInfo.count = 1;
  sceneLayoutInfo.pTypes = sceneInputTypes;

  QTZ_ATTEMPT_OPAL(OpalInputLayoutInit(&m_sceneLayout, sceneLayoutInfo));

  OpalMaterialInputValue sceneInputValues[1];
  sceneInputValues[0].buffer = m_sceneBuffer;

  OpalInputSetInitInfo sceneSetInfo = {};
  sceneSetInfo.layout = m_sceneLayout;
  sceneSetInfo.pInputValues = sceneInputValues;

  QTZ_ATTEMPT_OPAL(OpalInputSetInit(&m_sceneSet, sceneSetInfo));

  OpalInputInfo inputInfo[1] = {};
  inputInfo[0].index = 0;
  inputInfo[0].type = Opal_Input_Type_Uniform_Buffer;
  inputInfo[0].value.buffer = m_sceneBuffer;

  QTZ_ATTEMPT_OPAL(OpalInputSetUpdate(m_sceneSet, 1, inputInfo));

  return Quartz_Success;
}

QuartzResult Renderer::StartFrame()
{
  OpalResult result = OpalRenderBegin(m_window);
  if (result != Opal_Success)
  {
    if (result == Opal_Window_Minimized)
      return Quartz_Success;
    else
      return Quartz_Failure;
  }

  OpalRenderBeginRenderpass(m_renderpass, m_framebuffer);
  return Quartz_Success;
}

QuartzResult Renderer::EndFrame()
{
  OpalRenderEndRenderpass(m_renderpass);
  QTZ_ATTEMPT_OPAL(OpalRenderEnd());
  return Quartz_Success;
}

QuartzResult Renderer::Render(Renderable* renderable)
{
  renderable->material.Bind();
  OpalRenderSetPushConstant((void*)&renderable->transformMatrix);
  renderable->mesh.Render();

  return Quartz_Success;
}

void Renderer::Shutdown()
{
  OpalWaitIdle();

  OpalBufferShutdown(&m_sceneBuffer);
  OpalInputLayoutShutdown(&m_sceneLayout);
  OpalInputSetShutdown(&m_sceneSet);

  OpalFramebufferShutdown(&m_framebuffer);
  OpalRenderpassShutdown(&m_renderpass);
  OpalImageShutdown(&m_depthImage);
  OpalWindowShutdown(&m_window);
  OpalShutdown();
}

QuartzResult Renderer::PushSceneData(ScenePacket* sceneInfo)
{
  QTZ_ATTEMPT_OPAL(OpalBufferPushData(m_sceneBuffer, (void*)sceneInfo));
  return Quartz_Success;
}

QuartzResult Renderer::Resize(uint32_t width, uint32_t height)
{
  OpalResult result = OpalWindowReinit(m_window);
  if (result != Opal_Success)
  {
    if (result == Opal_Window_Minimized)
      return Quartz_Success;
    else
      return Quartz_Failure;
  }
  QTZ_ATTEMPT_OPAL(OpalImageResize(m_depthImage, m_window->extents));
  QTZ_ATTEMPT_OPAL(OpalFramebufferReinit(m_framebuffer));

  return Quartz_Success;
}

Material Renderer::CreateMaterial(const std::vector<const char*>& shaderPaths)
{
  Material newMaterial;
  newMaterial.Init(m_renderpass, shaderPaths);
  return newMaterial;
}

Mesh Renderer::CreateMesh(const char* path)
{
  Mesh m;
  

  return m;
}

Mesh Renderer::CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  Mesh newMesh;
  if (newMesh.Init(vertices, indices) != Quartz_Success)
  {
    QTZ_ATTEMPT_FAIL_LOG("Failed to build mesh");
  }

  return newMesh;
}

} // namespace Quartz
