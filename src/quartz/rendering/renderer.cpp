
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/renderer.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/filesystem/filesystem.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_vulkan.h>

namespace Quartz
{

OpalInputLayout Renderer::m_sceneLayout;
OpalInputSet Renderer::m_sceneSet;

void ImguiVkResultCheck(VkResult error) {}

void OpalMessageCallback(OpalMessageType type, const char* message)
{
  switch (type)
  {
  case Opal_Message_Info:
  {
    //QTZ_INFO("Opal : {}", message);
  } break;
  case Opal_Message_Debug:
  {
    QTZ_DEBUG("Opal : {}", message);
  } break;
  case Opal_Message_Warning:
  {
    QTZ_WARNING("Opal : {}", message);
  } break;
  case Opal_Message_Error:
  {
    QTZ_ERROR("Opal : {}", message);
  } break;
  case Opal_Message_Fatal:
  {
    QTZ_FATAL("Opal : {}", message);
  } break;
  default: break;
  }
}

QuartzResult Renderer::Init(Window* window)
{
  m_qWindow = window;

  const uint32_t vertexFormatCount = 4;
  OpalFormat vertexFormats[vertexFormatCount] = {
    Opal_Format_RGB32, // Position
    Opal_Format_RG32, // Uv
    Opal_Format_RGB32, // Normal
    Opal_Format_RGB32, // Tangent
  };

  OpalInitInfo opalInfo;
  opalInfo.debug = true;
  opalInfo.vertexStruct.count = vertexFormatCount;
  opalInfo.vertexStruct.pFormats = vertexFormats;
  opalInfo.messageCallback = OpalMessageCallback;

  OpalWindowInitInfo windowInfo;
  windowInfo.extents.width = window->Width();
  windowInfo.extents.height = window->Height();

  const WindowPlatformInfo platformInfo = window->PlatformInfo();
#ifdef QTZ_PLATFORM_WIN32
  opalInfo.windowPlatformInfo.hinstance = platformInfo.hinstance;
  opalInfo.windowPlatformInfo.hwnd = platformInfo.hwnd;
  windowInfo.platformInfo.hinstance = platformInfo.hinstance;
  windowInfo.platformInfo.hwnd = platformInfo.hwnd;
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
  attachments[0].clearValue.color = OpalColorValue{ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[0].format = Opal_Format_BGR8;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].usage = Opal_Attachment_Usage_Color;
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

  OpalBufferInitAlignedInfo sceneBufferInfo = {};
  sceneBufferInfo.usage = Opal_Buffer_Usage_Uniform;
  sceneBufferInfo.elementCount = packetElements.size();
  sceneBufferInfo.pElements = packetElements.data();

  QTZ_ATTEMPT_OPAL(OpalBufferInitAligned(&m_sceneBuffer, sceneBufferInfo));

  OpalInputAccessInfo sceneInputs[1] = {
    { Opal_Input_Type_Uniform_Buffer, Opal_Stage_All_Graphics }
  };

  OpalInputLayoutInitInfo sceneLayoutInfo = {};
  sceneLayoutInfo.count = 1;
  sceneLayoutInfo.pInputs = sceneInputs;

  QTZ_ATTEMPT_OPAL(OpalInputLayoutInit(&m_sceneLayout, sceneLayoutInfo));

  OpalInputValue sceneInputValues[1];
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

  InitImgui();

  return Quartz_Success;
}

QuartzResult Renderer::InitImgui()
{
  // Resources
  // ============================================================

  // Renderpass

  OpalAttachmentInfo attachment = {};
  attachment.clearValue.color = OpalColorValue{ 0.5f, 0.5f, 0.5f, 1.0f };
  attachment.format = Opal_Format_BGRA8;
  attachment.loadOp = Opal_Attachment_Op_Load;
  attachment.shouldStore = true;
  attachment.usage = Opal_Attachment_Usage_Presented;

  uint32_t colorIndex = 0;
  OpalSubpassInfo subpass = {};
  subpass.depthAttachmentIndex = OPAL_DEPTH_ATTACHMENT_NONE;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = &colorIndex;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = NULL;

  OpalRenderpassInitInfo renderpassInfo = {};
  renderpassInfo.dependencyCount = 0;
  renderpassInfo.pDependencies = NULL;
  renderpassInfo.imageCount = 1;
  renderpassInfo.pAttachments = &attachment;
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&m_imguiRenderpass, renderpassInfo));

  // Framebuffer

  OpalFramebufferInitInfo fbInfo = {};
  fbInfo.imageCount = 1;
  fbInfo.pImages = &m_window->renderBufferImage;
  fbInfo.renderpass = m_imguiRenderpass;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_imguiFramebuffer, fbInfo));

  // Input layout

  OpalInputAccessInfo input = { Opal_Input_Type_Samped_Image, Opal_Stage_Fragment };

  OpalInputLayoutInitInfo layoutInfo = {};
  layoutInfo.count = 1;
  layoutInfo.pInputs = &input;

  QTZ_ATTEMPT_OPAL(OpalInputLayoutInit(&m_imguiImageLayout, layoutInfo));

  // Imgui
  // ============================================================

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(m_window->platform.hwnd);

  ImGui_ImplVulkan_InitInfo imguiVulkanInfo = { 0 };
  imguiVulkanInfo.Allocator = NULL;
  imguiVulkanInfo.Instance = oState.vk.instance;
  imguiVulkanInfo.Device = oState.vk.device;
  imguiVulkanInfo.PhysicalDevice = oState.vk.gpu;
  imguiVulkanInfo.QueueFamily = oState.vk.gpuInfo.queueIndexGraphics;
  imguiVulkanInfo.Queue = oState.vk.queueGraphics;
  imguiVulkanInfo.PipelineCache = VK_NULL_HANDLE;
  imguiVulkanInfo.DescriptorPool = oState.vk.descriptorPool;
  imguiVulkanInfo.Subpass = 0;
  imguiVulkanInfo.MinImageCount = 2;
  imguiVulkanInfo.ImageCount = m_window->imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  ImGui_ImplVulkan_Init(&imguiVulkanInfo, m_imguiRenderpass->vk.renderpass);

  VkCommandBuffer cmd;
  OpalBeginSingleUseCommand(oState.vk.transientCommandPool, &cmd);
  ImGui_ImplVulkan_CreateFontsTexture();
  OpalEndSingleUseCommand(oState.vk.transientCommandPool, oState.vk.queueTransfer, cmd);

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

  return Quartz_Success;
}

QuartzResult Renderer::EndFrame()
{
  QTZ_ATTEMPT_OPAL(OpalRenderEnd());
  return Quartz_Success;
}

void Renderer::StartSceneRender()
{
  OpalRenderBeginRenderpass(m_renderpass, m_framebuffer);
}

void Renderer::EndSceneRender()
{
  OpalRenderEndRenderpass(m_renderpass);
}

void Renderer::StartImguiRender()
{
  OpalRenderBeginRenderpass(m_imguiRenderpass, m_imguiFramebuffer);
}

void Renderer::EndImguiRender()
{
  OpalRenderEndRenderpass(m_imguiRenderpass);
}

QuartzResult Renderer::Render(Renderable* renderable)
{
  renderable->material->Bind();
  OpalRenderSetPushConstant((void*)&renderable->transformMatrix);
  renderable->mesh->Render();

  return Quartz_Success;
}

void Renderer::Shutdown()
{
  OpalWaitIdle();
  
  ImGui_ImplVulkan_Shutdown();
  OpalInputLayoutShutdown(&m_imguiImageLayout);
  OpalFramebufferShutdown(&m_imguiFramebuffer);
  OpalRenderpassShutdown(&m_imguiRenderpass);

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
  QTZ_ATTEMPT_OPAL(OpalBufferAlignAndPushData(m_sceneBuffer, packetElements.size(), packetElements.data(), (void*)sceneInfo));
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
  QTZ_ATTEMPT_OPAL(OpalFramebufferReinit(m_imguiFramebuffer));

  return Quartz_Success;
}

} // namespace Quartz
