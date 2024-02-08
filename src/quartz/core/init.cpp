
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
Quartz::Texture dumped;

void SetHdri(Quartz::Texture& image)
{
  hdri = image;
}

Quartz::Texture& GetConvolvedHdri()
{
  return dumped;
}

QuartzResult ConvolveHdri()
{
  if (!hdri.IsValid())
  {
    QTZ_ERROR("HDRI is not a valid texture");
    return Quartz_Failure;
  }

  OpalImage convolutionImage;
  OpalRenderpass convolutionRp;
  OpalFramebuffer convolutionFb;

  // Image

  OpalImageInitInfo imageInfo = {};
  float extentsRatio = hdri.extents.width / hdri.extents.height;
  imageInfo.extent.height = 256;
  imageInfo.extent.width = (uint32_t)((float)imageInfo.extent.height * extentsRatio);
  imageInfo.extent.depth = 1;
  imageInfo.filterType = Opal_Image_Filter_Bilinear;
  imageInfo.format = Opal_Format_RGBA32;
  imageInfo.mipLevels = 1;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;
  imageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Copy_Src;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&convolutionImage, imageInfo));

  // Renderpass

  const uint32_t attachmentCount = 1;
  OpalAttachmentInfo attachment = {};
  attachment.clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachment.format = Opal_Format_RGBA32;
  attachment.loadOp = Opal_Attachment_Op_Clear;
  attachment.shouldStore = true;
  attachment.usage = Opal_Attachment_Usage_Color;

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
  renderpassInfo.pAttachments = &attachment;
  renderpassInfo.subpassCount = 1;
  renderpassInfo.pSubpasses = &subpass;

  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&convolutionRp, renderpassInfo));

  // Framebuffer

  OpalImage framebufferImages[attachmentCount] = { convolutionImage };

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = attachmentCount;
  framebufferInfo.pImages = framebufferImages;
  framebufferInfo.renderpass = convolutionRp;

  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&convolutionFb, framebufferInfo));

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

  Material mat;
  mat.m_renderpass = convolutionRp;
  QTZ_ATTEMPT(mat.Init(
    {
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolution.vert.spv",
      "D:/Dev/QuartzSandbox/res/shaders/compiled/t_convolution.frag.spv"
    },
    {
      {.type = Quartz::Input_Texture, .value = {.texture = &hdri } }
    },
    Quartz::Pipeline_Cull_None));

  // Render

  QTZ_ATTEMPT_OPAL(OpalRenderBeginSingle());

  OpalRenderBeginRenderpass(convolutionRp, convolutionFb);
  QTZ_ATTEMPT(mat.Bind());
  mesh.Render();
  OpalRenderEndRenderpass(convolutionRp);

  QTZ_ATTEMPT_OPAL(OpalRenderEndSingle());

  // Dump

  void* imageData;
  uint32_t imageSize = OpalImageDumpData(convolutionImage, &imageData);

  dumped.extents = Vec2U{ imageInfo.extent.width, imageInfo.extent.height };
  dumped.filtering = Quartz::Texture_Filter_Linear;
  dumped.format = Quartz::Texture_Format_RGBA32;
  dumped.mipLevels = 1;
  dumped.sampleMode = Texture_Sample_Clamp;
  dumped.usage = Texture_Usage_Shader_Input;

  Vec4* pixelData = (Vec4*)imageData;
  std::vector<Vec4> pixels(pixelData, pixelData + (imageSize / sizeof(Vec4)));

  QTZ_ATTEMPT(dumped.Init(pixels));

  mat.Shutdown();
  mesh.Shutdown();
  OpalRenderpassShutdown(&convolutionRp);
  OpalFramebufferShutdown(&convolutionFb);
  OpalImageShutdown(&convolutionImage);

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
