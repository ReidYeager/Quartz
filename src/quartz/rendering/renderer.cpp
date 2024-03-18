
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

OpalShaderInputLayout Renderer::m_sceneLayout;
OpalShaderInput Renderer::m_sceneSet;

void ImguiVkResultCheck(VkResult error) {}

void OpalMessageCallback(OpalMessageType type, const char* message)
{
  switch (type)
  {
  case Opal_Message_Info:
  {
    //QTZ_INFO("Opal : {}", message);
  } break;
  case Opal_Message_Error:
  {
    QTZ_ERROR("Opal : {}", message);
  } break;
  default: break;
  }
}

QuartzResult Renderer::Init(Window* window)
{
  m_qWindow = window;

  const WindowPlatformInfo platformInfo = window->PlatformInfo();

  const uint32_t vertexFormatCount = 4;
  OpalFormat vertexFormats[vertexFormatCount] = {
    Opal_Format_RGB32, // Position
    Opal_Format_RG32, // Uv
    Opal_Format_RGB32, // Normal
    Opal_Format_RGB32, // Tangent
  };

  OpalInitInfo opalInfo;
#ifdef QTZ_CONFIG_DEBUG
  opalInfo.useDebug = true;
#else
  opalInfo.useDebug = false;
#endif // QTZ_CONFIG_DEBUG
  opalInfo.api = Opal_Api_Vulkan;
  opalInfo.messageCallback = OpalMessageCallback;
  opalInfo.vertexLayout.elementCount = vertexFormatCount;
  opalInfo.vertexLayout.pElementFormats = vertexFormats;
#ifdef QTZ_PLATFORM_WIN32
  opalInfo.window.hinstance = platformInfo.hinstance;
  opalInfo.window.hwnd = platformInfo.hwnd;
#endif // QTZ_PLATFORM_*

  QTZ_ATTEMPT_OPAL(OpalInit(opalInfo));

  OpalWindowInitInfo windowInfo;
  windowInfo.width = window->Width();
  windowInfo.height = window->Height();
#ifdef QTZ_PLATFORM_WIN32
  windowInfo.platform.hinstance = platformInfo.hinstance;
  windowInfo.platform.hwnd = platformInfo.hwnd;
#endif // QTZ_PLATFORM_*

  QTZ_ATTEMPT_OPAL(OpalWindowInit(&m_window, windowInfo));

  // ==============================
  // Depth image
  // ==============================

  m_depthTexture.extents = Vec2U{ m_window.width, m_window.height };
  m_depthTexture.filtering = Quartz::Texture_Filter_Linear;
  m_depthTexture.usage = Quartz::Texture_Usage_Framebuffer;
  m_depthTexture.format = Quartz::Texture_Format_Depth;
  m_depthTexture.mipLevels = 1;
  QTZ_ATTEMPT(m_depthTexture.Init());

  // ==============================
  // Renderpass
  // ==============================

  const int attachmentCount = 2;
  OpalAttachmentInfo attachments[attachmentCount];
  // Presented image
  OpalAttachmentUsage presentedImageUsage = Opal_Attachment_Usage_Output_Presented;
  attachments[0].clearValue.color = OpalColorValue{ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[0].format = m_window.imageFormat;
  attachments[0].loadOp = Opal_Attachment_Load_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].pSubpassUsages = &presentedImageUsage;
  // Depth image
  OpalAttachmentUsage depthImageUsage = Opal_Attachment_Usage_Output;
  attachments[1].clearValue.depthStencil = OpalDepthStencilValue{ 1, 0 };
  attachments[1].format = Opal_Format_D24_S8;
  attachments[1].loadOp = Opal_Attachment_Load_Op_Clear;
  attachments[1].shouldStore = true;
  attachments[1].pSubpassUsages = &depthImageUsage;

  OpalRenderpassInitInfo renderpassInfo;
  renderpassInfo.attachmentCount = attachmentCount;
  renderpassInfo.pAttachments = attachments;
  renderpassInfo.subpassCount = 1;

  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&m_renderpass, renderpassInfo));

  // ==============================
  // Framebuffer
  // ==============================

  OpalImage* framebufferImages[2];
  framebufferImages[1] = &m_depthTexture.m_opalImage;

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 2;
  framebufferInfo.renderpass = m_renderpass;
  framebufferInfo.ppImages = &framebufferImages[0];

  m_framebuffers.resize(m_window.imageCount);
  for (int i = 0; i < m_window.imageCount; i++)
  {
    framebufferImages[0] = &m_window.pImages[i];
    QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_framebuffers[i], framebufferInfo));
  }

  // ==============================
  // Scene input set
  // ==============================

  OpalBufferInitInfo sceneBufferInfo = {};
  sceneBufferInfo.size = sizeof(Quartz::ScenePacket);
  sceneBufferInfo.usage = Opal_Buffer_Usage_Uniform;

  QTZ_ATTEMPT_OPAL(OpalBufferInit(&m_sceneBuffer, sceneBufferInfo));

  OpalStageFlags sceneInputUsageStages[] = {
    Opal_Stage_All_Graphics
  };
  OpalShaderInputType sceneInputTypes[] = {
    Opal_Shader_Input_Buffer
  };

  OpalShaderInputLayoutInitInfo sceneLayoutInfo = {};
  sceneLayoutInfo.count = 1;
  sceneLayoutInfo.pStages = sceneInputUsageStages;
  sceneLayoutInfo.pTypes = sceneInputTypes;

  QTZ_ATTEMPT_OPAL(OpalShaderInputLayoutInit(&m_sceneLayout, sceneLayoutInfo));

  OpalShaderInputValue sceneInputValues[] = {
    {.buffer = &m_sceneBuffer}
  };

  OpalShaderInputInitInfo sceneSetInfo = {};
  sceneSetInfo.layout = m_sceneLayout;
  sceneSetInfo.pValues = sceneInputValues;

  QTZ_ATTEMPT_OPAL(OpalShaderInputInit(&m_sceneSet, sceneSetInfo));

  InitImgui();

  return Quartz_Success;
}

QuartzResult Renderer::InitImgui()
{
  // Resources
  // ============================================================

  // Renderpass

  const int attachmentCount = 1;
  OpalAttachmentInfo attachments[attachmentCount];
  // Presented image
  OpalAttachmentUsage presentedImageUsage = Opal_Attachment_Usage_Output_Presented;
  attachments[0].clearValue.color = OpalColorValue{ 0.5f, 0.5f, 0.5f, 1.0f };
  attachments[0].format = m_window.imageFormat;
  attachments[0].loadOp = Opal_Attachment_Load_Op_Load;
  attachments[0].shouldStore = true;
  attachments[0].pSubpassUsages = &presentedImageUsage;

  OpalRenderpassInitInfo renderpassInfo;
  renderpassInfo.attachmentCount = attachmentCount;
  renderpassInfo.pAttachments = attachments;
  renderpassInfo.subpassCount = 1;

  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&m_imguiRenderpass, renderpassInfo));

  // Framebuffer

  OpalFramebufferInitInfo fbInfo;
  fbInfo.imageCount = attachmentCount;
  fbInfo.renderpass = m_imguiRenderpass;

  m_imguiFramebuffers.resize(m_window.imageCount);
  for (int i = 0; i < m_window.imageCount; i++)
  {
    OpalImage* imagePointer = &m_window.pImages[i];
    fbInfo.ppImages = &imagePointer;
    QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_imguiFramebuffers[i], fbInfo));
  }

  // Input layout

  OpalStageFlags imguiImageUsageFlags = Opal_Stage_Fragment;
  OpalShaderInputType imguiImageInputType = Opal_Shader_Input_Image;

  OpalShaderInputLayoutInitInfo layoutInfo;
  layoutInfo.count = 1;
  layoutInfo.pStages = &imguiImageUsageFlags;
  layoutInfo.pTypes = &imguiImageInputType;

  QTZ_ATTEMPT_OPAL(OpalShaderInputLayoutInit(&m_imguiImageLayout, layoutInfo));

  // Imgui
  // ============================================================

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
  //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(m_qWindow->PlatformInfo().hwnd);

  OpalState* oState = OpalGetState();

  ImGui_ImplVulkan_InitInfo imguiVulkanInfo = { 0 };
  imguiVulkanInfo.Allocator = NULL;
  imguiVulkanInfo.Instance = oState->api.vk.instance;
  imguiVulkanInfo.Device = oState->api.vk.device;
  imguiVulkanInfo.PhysicalDevice = oState->api.vk.gpu.device;
  imguiVulkanInfo.QueueFamily = oState->api.vk.gpu.queueIndexGraphicsCompute;
  imguiVulkanInfo.Queue = oState->api.vk.queueGraphics;
  imguiVulkanInfo.PipelineCache = VK_NULL_HANDLE;
  imguiVulkanInfo.DescriptorPool = oState->api.vk.descriptorPool;
  imguiVulkanInfo.Subpass = 0;
  imguiVulkanInfo.MinImageCount = 2;
  imguiVulkanInfo.ImageCount = m_window.imageCount;
  imguiVulkanInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  imguiVulkanInfo.CheckVkResultFn = ImguiVkResultCheck;
  imguiVulkanInfo.RenderPass = m_imguiRenderpass.api.vk.renderpass;
  ImGui_ImplVulkan_Init(&imguiVulkanInfo);

  //QTZ_ATTEMPT_OPAL(OpalRenderBegin());
  //ImGui_ImplVulkan_CreateFontsTexture();
  //QTZ_ATTEMPT_OPAL(OpalRenderEnd());

  return Quartz_Success;
}

QuartzResult Renderer::StartFrame()
{
  OpalResult result = OpalRenderToWindowBegin(&m_window);

  if (result != Opal_Success)
  {
    if (result == Opal_Failure_Window_Minimized)
      return Quartz_Success;
    else
      return Quartz_Failure;
  }

  return Quartz_Success;
}

QuartzResult Renderer::EndFrame()
{
  QTZ_ATTEMPT_OPAL(OpalRenderToWindowEnd(&m_window));
  imageIndex = (imageIndex + 1) % m_framebuffers.size();
  return Quartz_Success;
}

void Renderer::StartSceneRender()
{
  OpalRenderRenderpassBegin(&m_renderpass, &m_framebuffers[imageIndex]);
}

void Renderer::EndSceneRender()
{
  OpalRenderRenderpassEnd(&m_renderpass);
}

void Renderer::StartImguiRender()
{
  OpalRenderRenderpassBegin(&m_imguiRenderpass, &m_imguiFramebuffers[imageIndex]);
}

void Renderer::EndImguiRender()
{
  OpalRenderRenderpassEnd(&m_imguiRenderpass);
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
  //ImGui_ImplVulkan_DestroyFontsTexture();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplWin32_Shutdown();

  OpalBufferShutdown(&m_sceneBuffer);
  OpalShaderInputLayoutShutdown(&m_sceneLayout);
  OpalShaderInputShutdown(&m_sceneSet);

  for (int i = 0; i < m_framebuffers.size(); i++)
  {
    OpalFramebufferShutdown(&m_imguiFramebuffers[i]);
    OpalFramebufferShutdown(&m_framebuffers[i]);
  }

  OpalShaderInputLayoutShutdown(&m_imguiImageLayout);
  OpalRenderpassShutdown(&m_imguiRenderpass);

  OpalRenderpassShutdown(&m_renderpass);

  m_depthTexture.Shutdown();
  OpalWindowShutdown(&m_window);
  OpalShutdown();
}

QuartzResult Renderer::PushSceneData(ScenePacket* sceneInfo)
{
  QTZ_ATTEMPT_OPAL(OpalBufferPushData(&m_sceneBuffer, (void*)sceneInfo));
  OpalWaitIdle(); // TODO : Remove on Opal synchronization rework
  return Quartz_Success;
}

QuartzResult Renderer::Resize(uint32_t width, uint32_t height)
{
  OpalWindowShutdown(&m_window);
  OpalWindowInitInfo windowInfo;
  windowInfo.height = height;
  windowInfo.width = width;
  windowInfo.platform.hwnd = m_qWindow->PlatformInfo().hwnd;
  windowInfo.platform.hinstance = m_qWindow->PlatformInfo().hinstance;
  QTZ_ATTEMPT_OPAL(OpalWindowInit(&m_window, windowInfo));

  QTZ_ATTEMPT(m_depthTexture.Resize(Vec2U{ m_window.width, m_window.height }));

  OpalImage* framebufferImages[2];
  framebufferImages[1] = &m_depthTexture.m_opalImage;

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.ppImages = framebufferImages;

  m_framebuffers.resize(m_window.imageCount);
  m_imguiFramebuffers.resize(m_window.imageCount);
  for (int i = 0; i < m_window.imageCount; i++)
  {
    framebufferImages[0] = &m_window.pImages[i];

    OpalFramebufferShutdown(&m_framebuffers[i]);
    OpalFramebufferShutdown(&m_imguiFramebuffers[i]);

    framebufferInfo.renderpass = m_renderpass;
    framebufferInfo.imageCount = 2;
    QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_framebuffers[i], framebufferInfo));
    framebufferInfo.renderpass = m_imguiRenderpass;
    framebufferInfo.imageCount = 1;
    QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m_imguiFramebuffers[i], framebufferInfo));
  }

  return Quartz_Success;
}

} // namespace Quartz
