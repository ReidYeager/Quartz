
#include "quartz.h"
#include "quartz/defines.h"
#include "quartz/core/core.h"

#include "quartz/platform/window/window.h"

namespace Quartz
{

// Global variables
// ============================================================

CoreState g_coreState;
uint32_t g_quartzAttemptDepth;

// Quartz entrypoint
// ============================================================

void Run()
{
  QTZ_ATTEMPT_VOID(CoreInit());
  QTZ_ATTEMPT_VOID(CoreMainLoop());
  CoreShutdown();
}

// Declarations
// ============================================================

// Platform
QuartzResult InitWindow();
// Rendering
QuartzResult InitRenderer();
//void SetHdri(Texture& image); <-- Defined in quartz.h (Temporary function during Hdri convolution learning)
//QuartzResult ConvolveHdri();  <-- Defined in quartz.h (Temporary function during Hdri convolution learning)
//Texture& GetDiffuseHdri();    <-- Defined in quartz.h (Temporary function during Hdri convolution learning)
//Texture& GetSpecularHdri();   <-- Defined in quartz.h (Temporary function during Hdri convolution learning)
//Texture& GetSpecularBrdf();   <-- Defined in quartz.h (Temporary function during Hdri convolution learning)
// Engine
QuartzResult InitEcs();
void InitClocks();
QuartzResult InitLayers();

// Core
// ============================================================

QuartzResult CoreInit()
{
  Logger::Init();

  QTZ_ATTEMPT(InitWindow());
  QTZ_ATTEMPT(InitRenderer());
  // init resource pools

  QTZ_ATTEMPT(InitEcs());
  QTZ_ATTEMPT(InitLayers());

  QTZ_INFO("Core initialized");

  g_coreState.clientApp = GetClientApplication();
  QTZ_ATTEMPT(g_coreState.clientApp->Init());

  InitClocks();

  return Quartz_Success;
}

// Platform
// ============================================================

QuartzResult InitWindow()
{
  WindowInitInfo windowInfo = {};
  windowInfo.width = 1280;
  windowInfo.height = 720;
  windowInfo.posX = 600;
  windowInfo.posY = 300;
  windowInfo.title = "Test new window";
  windowInfo.eventCallbackFunction = EventCallback;

  QTZ_ATTEMPT(g_coreState.mainWindow.Init(windowInfo));

  return Quartz_Success;
}

// Rendering
// ============================================================

QuartzResult InitRenderer()
{
  QTZ_ATTEMPT(g_coreState.renderer.Init(&g_coreState.mainWindow));
  return Quartz_Success;
}

Quartz::Texture hdri;
Quartz::Texture dumpedDiffuse;
Quartz::Texture dumpedSpecular;
Quartz::Texture dumpedBrdf;

void SetHdri(Texture& image)
{
  hdri = image;
}

Texture& GetDiffuseHdri()
{
  return dumpedDiffuse;
}

Texture& GetSpecularHdri()
{
  return dumpedSpecular;
}

Texture& GetSpecularBrdf()
{
  return dumpedBrdf;
}

QuartzResult ConvolveHdri()
{
  if (!hdri.IsValid())
  {
    QTZ_ERROR("HDRI is not a valid texture");
    return Quartz_Failure;
  }

  OpalImage diffuseConvolutionImage;
  OpalImage specularConvolutionImage;
  OpalImage specularBrdfImage;
  OpalRenderpass convolutionRp;
  OpalRenderpass brdfRenderpass;
  const uint8_t specMipCount = 5;
  const uint32_t specMaxHeight = 256;
  const uint32_t brdfSize = 512;
  Buffer dummybuffer;
  dummybuffer.Init(1);

  struct SpecularMipRenderResources
  {
    Buffer buffer;
    Material mat;

    VkImageView view;
    VkFramebuffer framebuffer;
  };
  SpecularMipRenderResources specMipRes[specMipCount];

  // Image

  float extentsRatio = hdri.extents.width / hdri.extents.height;
  // Diffuse convolution
  OpalImageInitInfo diffuseImageInfo = {};
  diffuseImageInfo.extent.height = 64;
  diffuseImageInfo.extent.width = (uint32_t)((float)diffuseImageInfo.extent.height * extentsRatio);
  diffuseImageInfo.extent.depth = 1;
  diffuseImageInfo.filterType = Opal_Image_Filter_Bilinear;
  diffuseImageInfo.format = Opal_Format_RGBA32;
  diffuseImageInfo.mipLevels = 1;
  diffuseImageInfo.sampleMode = Opal_Image_Sample_Clamp;
  diffuseImageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&diffuseConvolutionImage, diffuseImageInfo));
  // Specular convolution
  OpalImageInitInfo specularImageInfo = {};
  specularImageInfo.extent.height = specMaxHeight;
  specularImageInfo.extent.width = (uint32_t)((float)specularImageInfo.extent.height * extentsRatio);
  specularImageInfo.extent.depth = 1;
  specularImageInfo.filterType = Opal_Image_Filter_Bilinear;
  specularImageInfo.format = Opal_Format_RGBA32;
  specularImageInfo.mipLevels = specMipCount;
  specularImageInfo.sampleMode = Opal_Image_Sample_Clamp;
  specularImageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&specularConvolutionImage, specularImageInfo));
  // Specular brdf
  OpalImageInitInfo brdfImageInfo = {};
  brdfImageInfo.extent.height = brdfSize;
  brdfImageInfo.extent.width = brdfSize;
  brdfImageInfo.extent.depth = 1;
  brdfImageInfo.filterType = Opal_Image_Filter_Bilinear;
  brdfImageInfo.format = Opal_Format_RG16;
  brdfImageInfo.mipLevels = 1;
  brdfImageInfo.sampleMode = Opal_Image_Sample_Clamp;
  brdfImageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&specularBrdfImage, brdfImageInfo));

  // Renderpass

  const uint32_t attachmentCount = 1;
  OpalAttachmentInfo attachments[1];
  attachments[0].clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachments[0].format = Opal_Format_RGBA32;
  attachments[0].loadOp = Opal_Attachment_Op_Clear;
  attachments[0].shouldStore = true;
  attachments[0].usage = Opal_Attachment_Usage_Color;

  uint32_t zeroIndex = 0;
  OpalSubpassInfo subpass;
  subpass.depthAttachmentIndex = OPAL_DEPTH_ATTACHMENT_NONE;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachmentIndices = &zeroIndex;
  subpass.inputAttachmentCount = 0;
  subpass.pInputColorAttachmentIndices = nullptr;

  OpalRenderpassInitInfo renderpassInfo = {};
  renderpassInfo.dependencyCount = 0;
  renderpassInfo.pDependencies = nullptr;
  renderpassInfo.imageCount = attachmentCount;
  renderpassInfo.pAttachments = attachments;
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;

  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&convolutionRp, renderpassInfo));

  attachments[0].format = Opal_Format_RG16;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&brdfRenderpass, renderpassInfo));

  // Framebuffer

  OpalFramebuffer convolutionFb;

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.pImages = &diffuseConvolutionImage;
  framebufferInfo.renderpass = convolutionRp;

  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&convolutionFb, framebufferInfo));

  OpalFramebuffer brdfFramebuffer;
  framebufferInfo.pImages = &specularBrdfImage;
  framebufferInfo.renderpass = brdfRenderpass;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&brdfFramebuffer, framebufferInfo));

  // Mesh

  std::vector<Vertex> vertices(4);
  vertices[0].position = Vec3{ -1.0f, -1.0f, 1.0f }; // TL
  vertices[0].uv = Vec2{ 0.0f, 0.0f };
  vertices[1].position = Vec3{  1.0f, -1.0f, 1.0f }; // TR
  vertices[1].uv = Vec2{ 1.0f, 0.0f };
  vertices[2].position = Vec3{ -1.0f,  1.0f, 1.0f }; // BL
  vertices[2].uv = Vec2{ 0.0f, 1.0f };
  vertices[3].position = Vec3{  1.0f,  1.0f, 1.0f }; // BR
  vertices[3].uv = Vec2{ 1.0f, 1.0f };

  std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 1
  };

  Mesh mesh;
  QTZ_ATTEMPT(mesh.Init(vertices, indices));

  // Material

  Material matDiffuse;
  matDiffuse.m_renderpass = convolutionRp;
  QTZ_ATTEMPT(matDiffuse.Init(
    {
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolution.vert.spv",
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolutionDiffuse.frag.spv"
    },
    {
      { .type = Quartz::Input_Texture, .value = { .texture = &hdri } }
    },
    Quartz::Pipeline_Cull_None));

  Material matSpecular;
  matSpecular.m_renderpass = convolutionRp;
  QTZ_ATTEMPT(matSpecular.Init(
    {
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolution.vert.spv",
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolutionSpecular.frag.spv"
    },
    {
      { .type = Quartz::Input_Texture, .value = { .texture = &hdri } },
      { .type = Quartz::Input_Buffer, .value = { .buffer = &dummybuffer } }
    },
    Quartz::Pipeline_Cull_None));

  Material matBrdf;
  matBrdf.m_renderpass = brdfRenderpass;
  QTZ_ATTEMPT(matBrdf.Init(
    {
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolution.vert.spv",
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolutionBrdf.frag.spv"
    },
    { },
    Quartz::Pipeline_Cull_None));

  // Specular mips

  for (uint32_t i = 0; i < specMipCount; i++)
  {
    // Image view

    VkImageViewCreateInfo viewCreateInfo = { 0 };
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.image = specularConvolutionImage->vk.image;
    viewCreateInfo.format = specularConvolutionImage->vk.format;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.baseMipLevel = i;

    if (vkCreateImageView(oState.vk.device, &viewCreateInfo, oState.vk.allocator, &specMipRes[i].view) != VK_SUCCESS)
    {
      QTZ_FAIL_LOG("Failed to create mip view {}", i);
      return Quartz_Failure;
    }

    // Framebuffer

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags = 0;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &specMipRes[i].view;
    framebufferInfo.height = specularImageInfo.extent.height * pow(0.5, i);
    framebufferInfo.width = specularImageInfo.extent.width * pow(0.5, i);
    framebufferInfo.layers = 1;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.renderPass = convolutionRp->vk.renderpass;
    VkResult result = vkCreateFramebuffer(oState.vk.device, &framebufferInfo, oState.vk.allocator, &specMipRes[i].framebuffer);
    if (result != VK_SUCCESS)
    {
      QTZ_FAIL_LOG("Failed to create mip framebuffer {}", i);
      return Quartz_Failure;
    }

    // Buffer

    float value = float(i) / float(specMipCount - 1);
    specMipRes[i].buffer.Init(sizeof(float));
    specMipRes[i].buffer.PushData(&value);

    // Material

    QTZ_ATTEMPT(specMipRes[i].mat.Init(
      matSpecular,
      {
        { .texture = &hdri },
        { .buffer = &specMipRes[i].buffer }
      }));
  }

  // Render

  QTZ_ATTEMPT_OPAL(OpalRenderBeginSingle());

  OpalRenderBeginRenderpass(convolutionRp, convolutionFb);
  QTZ_ATTEMPT(matDiffuse.Bind());
  mesh.Render();
  OpalRenderEndRenderpass(convolutionRp);

  VkRenderPassBeginInfo beginInfo = { 0 };
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

  beginInfo.renderPass = convolutionRp->vk.renderpass;
  beginInfo.clearValueCount = convolutionRp->imageCount;
  beginInfo.pClearValues = convolutionRp->vk.pClearValues;
  beginInfo.renderArea.offset = VkOffset2D{ 0, 0 };

  for (uint32_t i = 0; i < specMipCount; i++)
  {
    uint32_t mipHeight = specularImageInfo.extent.height * pow(0.5, i);
    uint32_t mipWidth = specularImageInfo.extent.width * pow(0.5, i);

    // Start renderpass for this framebuffer
    beginInfo.framebuffer = specMipRes[i].framebuffer;
    beginInfo.renderArea.extent = VkExtent2D{ mipWidth, mipHeight };
    vkCmdBeginRenderPass(oState.vk.currentCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind material
    QTZ_ATTEMPT(specMipRes[i].mat.Bind());

    VkViewport viewport = { 0, 0, 1, 1, 0, 1 };
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = mipWidth;
    viewport.height = mipHeight;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = { 0 };
    scissor.extent = VkExtent2D{ mipWidth, mipHeight };
    scissor.offset = VkOffset2D{ 0, 0 };

    vkCmdSetViewport(oState.vk.currentCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(oState.vk.currentCommandBuffer, 0, 1, &scissor);

    OpalRenderBindInputSet(Renderer::SceneSet(), 0);
    OpalRenderBindInputSet(specMipRes[i].mat.m_set, 1);

    mesh.Render();

    vkCmdEndRenderPass(oState.vk.currentCommandBuffer);
  }

  // Brdf
  beginInfo.renderPass = brdfRenderpass->vk.renderpass;
  beginInfo.framebuffer = brdfFramebuffer->vk.framebuffer;
  beginInfo.renderArea.extent = VkExtent2D{ brdfSize, brdfSize };
  vkCmdBeginRenderPass(oState.vk.currentCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

  QTZ_ATTEMPT(matBrdf.Bind());

  VkViewport viewport = { 0, 0, 1, 1, 0, 1 };
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = brdfSize;
  viewport.height = brdfSize;
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  VkRect2D scissor = { 0 };
  scissor.extent = VkExtent2D{ brdfSize, brdfSize };
  scissor.offset = VkOffset2D{ 0, 0 };

  vkCmdSetViewport(oState.vk.currentCommandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(oState.vk.currentCommandBuffer, 0, 1, &scissor);

  mesh.Render();

  vkCmdEndRenderPass(oState.vk.currentCommandBuffer);

  QTZ_ATTEMPT_OPAL(OpalRenderEndSingle());

  // Dump

  void* imageData;

  // Diffuse
  dumpedDiffuse.extents = Vec2U{ diffuseImageInfo.extent.width, diffuseImageInfo.extent.height };
  dumpedDiffuse.filtering = Texture_Filter_Linear;
  dumpedDiffuse.format = Texture_Format_RGBA32;
  dumpedDiffuse.mipLevels = 1;
  dumpedDiffuse.sampleMode = Texture_Sample_Clamp;
  dumpedDiffuse.usage = Texture_Usage_Shader_Input;

  uint32_t imageSize = OpalImageDumpData(diffuseConvolutionImage, &imageData);
  Vec4* pixelData = (Vec4*)imageData;
  std::vector<Vec4> pixelsDiffuse(pixelData, pixelData + (imageSize / sizeof(Vec4)));

  QTZ_ATTEMPT(dumpedDiffuse.Init(pixelsDiffuse));
  free(imageData);

  // Specular
  dumpedSpecular.extents = Vec2U{ specularImageInfo.extent.width, specularImageInfo.extent.height };
  dumpedSpecular.filtering = Texture_Filter_Linear;
  dumpedSpecular.format = Texture_Format_RGBA32;
  dumpedSpecular.mipLevels = specularImageInfo.mipLevels;
  dumpedSpecular.sampleMode = Texture_Sample_Clamp;
  dumpedSpecular.usage = Texture_Usage_Shader_Input;

  imageSize = OpalImageDumpData(specularConvolutionImage, &imageData);
  pixelData = (Vec4*)imageData;
  std::vector<Vec4> pixelsSpecular(pixelData, pixelData + (imageSize / sizeof(Vec4)));

  QTZ_ATTEMPT(dumpedSpecular.Init(pixelsSpecular));
  free(imageData);

  // Brdf
  dumpedBrdf.extents = Vec2U{ brdfSize, brdfSize };
  dumpedBrdf.filtering = Texture_Filter_Linear;
  dumpedBrdf.format = Texture_Format_RG16;
  dumpedBrdf.mipLevels = 1;
  dumpedBrdf.sampleMode = Texture_Sample_Clamp;
  dumpedBrdf.usage = Texture_Usage_Shader_Input;

  imageSize = OpalImageDumpData(specularBrdfImage, &imageData);
  QTZ_ATTEMPT(dumpedBrdf.Init(imageData));
  free(imageData);

  // Shutdown

  matDiffuse.Shutdown();
  matSpecular.Shutdown();
  matBrdf.Shutdown();
  mesh.Shutdown();
  dummybuffer.Shutdown();
  OpalRenderpassShutdown(&convolutionRp);
  OpalRenderpassShutdown(&brdfRenderpass);
  OpalFramebufferShutdown(&convolutionFb);
  OpalFramebufferShutdown(&brdfFramebuffer);
  OpalImageShutdown(&diffuseConvolutionImage);
  OpalImageShutdown(&specularConvolutionImage);
  OpalImageShutdown(&specularBrdfImage);

  for (uint32_t i = 0; i < specMipCount; i++)
  {
    specMipRes[i].mat.Shutdown();
    specMipRes[i].buffer.Shutdown();
    vkDestroyImageView(oState.vk.device, specMipRes[i].view, nullptr);
    vkDestroyFramebuffer(oState.vk.device, specMipRes[i].framebuffer, nullptr);
  }

  return Quartz_Success;
}

// Engine
// ============================================================

QuartzResult InitEcs()
{
  g_coreState.ecsIds.transform  = QuartzDefineComponent(Transform);
  g_coreState.ecsIds.renderable = QuartzDefineComponent(Renderable);
  g_coreState.ecsIds.camera     = QuartzDefineComponent(Camera);
  g_coreState.ecsIds.lightDir   = QuartzDefineComponent(LightDirectional);
  g_coreState.ecsIds.lightPoint = QuartzDefineComponent(LightPoint);
  g_coreState.ecsIds.lightSpot  = QuartzDefineComponent(LightSpot);

  return Quartz_Success;
}

void InitClocks()
{
  g_coreState.time.clocks.engineStart = std::chrono::high_resolution_clock::now();
  g_coreState.time.clocks.frameStart = std::chrono::high_resolution_clock::now();
  // frameEnd set at the start of the main loop
  g_coreState.time.frameIndex = ~0ll; // Will overflow to 0 at start of the first frame
}

QuartzResult InitLayers()
{
  // Init input layer
  // Init debug ui layer

  return Quartz_Success;
}

} // namespace Quartz
