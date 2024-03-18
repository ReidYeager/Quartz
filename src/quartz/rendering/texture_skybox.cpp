
#include "quartz/defines.h"
#include "quartz/core/core.h"
#include "quartz/rendering/texture.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/texture_skybox_shaders.inl"

namespace Quartz
{

QuartzResult TextureSkybox::Init(const char* path)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attemting to initialize a valid skybox");
    return Quartz_Success;
  }

  float* pixelData = nullptr;
  int32_t width = 0, height = 0;
  const char* err;

  if (LoadEXR(&pixelData, &width, &height, path, &err) || width <= 0 || height <= 0 || pixelData == nullptr)
  {
    QTZ_ERROR(
      "Failed to load 32-bit image \"{}\"\n"
      "    Width: {} -- Height: {} -- Channels: {}\n"
      "    Error: {}",
      path,
      width, height, 4,
      err);

    width = 0;
    return Quartz_Failure;
  }

  extents.width = width;
  extents.height = height;

  QTZ_ATTEMPT(CreateBase((void*)pixelData));
  QTZ_ATTEMPT(CreateIbl());

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult TextureSkybox::Init(const void* pixels)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attemting to initialize a valid skybox");
    return Quartz_Success;
  }

  QTZ_ATTEMPT(CreateBase(pixels));
  QTZ_ATTEMPT(CreateIbl());

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult TextureSkybox::InitFromDump(const char* path)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attemting to initialize a valid skybox");
    return Quartz_Success;
  }

  QTZ_ATTEMPT(m_baseImage.InitFromDump(path));

  extents = m_baseImage.extents;
  baseFormat = m_baseImage.format;

  QTZ_ATTEMPT(CreateIbl());

  m_isValid = true;
  return Quartz_Success;
}

// Shutdown
// ============================================================

void TextureSkybox::Shutdown()
{
  if (!m_isValid)
  {
    return;
  }

  m_baseImage.Shutdown();
  m_specularImage.Shutdown();
  m_diffuseImage.Shutdown();
  m_brdfImage.Shutdown();

  m_isValid = false;
}

// Ibl
// ============================================================

QuartzResult TextureSkybox::CreateIbl()
{
  // Mesh ==============================

  std::vector<Vertex> vertices(4);
  vertices[0].position = Vec3{ -1.0f, -1.0f, 1.0f }; // TL
  vertices[1].position = Vec3{  1.0f, -1.0f, 1.0f }; // TR
  vertices[2].position = Vec3{ -1.0f,  1.0f, 1.0f }; // BL
  vertices[3].position = Vec3{  1.0f,  1.0f, 1.0f }; // BR
  vertices[0].uv = Vec2{ 0.0f, 0.0f }; // TL
  vertices[1].uv = Vec2{ 1.0f, 0.0f }; // TR
  vertices[2].uv = Vec2{ 0.0f, 1.0f }; // BL
  vertices[3].uv = Vec2{ 1.0f, 1.0f }; // BR

  std::vector<uint32_t> indices = {
    0, 1, 2,
    2, 3, 1
  };

  Mesh screenQuadMesh;
  QTZ_ATTEMPT(screenQuadMesh.Init(vertices, indices));

  // Images ==============================

  QTZ_ATTEMPT(CreateDiffuse(screenQuadMesh));
  QTZ_ATTEMPT(CreateSpecular(screenQuadMesh));
  QTZ_ATTEMPT(CreateBrdf(screenQuadMesh));

  screenQuadMesh.Shutdown();
  return Quartz_Success;
}

QuartzResult TextureSkybox::CreateBase(const void* pixels)
{
  m_baseImage.mipLevels = 1;
  m_baseImage.extents = extents;
  m_baseImage.format = Texture_Format_RGBA32;
  m_baseImage.filtering = Texture_Filter_Linear;
  m_baseImage.sampleMode = Texture_Sample_Clamp;
  m_baseImage.usage = Texture_Usage_Shader_Input;

  QTZ_ATTEMPT(m_baseImage.Init(pixels));

  return Quartz_Success;
}

// Diffuse
// ============================================================

QuartzResult TextureSkybox::CreateDiffuse(const Mesh& screenQuadMesh)
{
  // Image creation ==============================

  OpalImageInitInfo imageInfo = {};
  imageInfo.height = 128;
  imageInfo.width = imageInfo.height * ((float)extents.width / (float)extents.height);
  imageInfo.mipCount = 1;
  // For :          Rendering to            | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Output | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RGBA32;
  imageInfo.filter = Opal_Image_Filter_Linear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage diffuseOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&diffuseOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  const uint32_t subpassCount = 1;
  OpalAttachmentUsage attachmentUses[subpassCount] = { Opal_Attachment_Usage_Output_Uniform };
  OpalAttachmentInfo attachment;
  attachment.clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachment.format = Opal_Format_RGBA32;
  attachment.loadOp = Opal_Attachment_Load_Op_Clear;
  attachment.shouldStore = true;
  attachment.pSubpassUsages = attachmentUses;

  OpalRenderpassInitInfo renderpassInfo = {};
  renderpassInfo.attachmentCount = attachmentCount;
  renderpassInfo.pAttachments = &attachment;
  renderpassInfo.subpassCount = subpassCount;

  OpalRenderpass diffuseRenderpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&diffuseRenderpass, renderpassInfo));

  // Framebuffer ==============================

  OpalImage* fbImages[] = { &diffuseOpalImage };
  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.ppImages = &fbImages[0];
  framebufferInfo.renderpass = diffuseRenderpass;

  OpalFramebuffer diffuseFramebuffer;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&diffuseFramebuffer, framebufferInfo));

  // Material ==============================

  Material diffuseMaterial;

  ShaderSourceInfo vertInfo, fragInfo;
  vertInfo.size = resSkyboxVertShaderByteCount;
  vertInfo.data = (void*)resSkyboxVertShaderBytes;
  fragInfo.size = resSkyboxDiffuseFragShaderByteCount;
  fragInfo.data = (void*)resSkyboxDiffuseFragShaderBytes;

  QTZ_ATTEMPT(diffuseMaterial.Init(
    vertInfo, fragInfo,
    {
      {.type = Quartz::Input_Texture, .value = { .texture = &m_baseImage } }
    },
    diffuseRenderpass,
    Quartz::Pipeline_Cull_None));

  // Render ==============================

  OpalWaitIdle();
  QTZ_ATTEMPT_OPAL(OpalRenderBegin());
  OpalRenderRenderpassBegin(&diffuseRenderpass, &diffuseFramebuffer);
  QTZ_ATTEMPT(diffuseMaterial.Bind());
  screenQuadMesh.Render();
  OpalRenderRenderpassEnd(&diffuseRenderpass);
  OpalSyncPack syncpack = {};
  QTZ_ATTEMPT_OPAL(OpalRenderEnd(syncpack));
  OpalWaitIdle();

  // Convert to texture ==============================

  m_diffuseImage.mipLevels = 1;
  m_diffuseImage.extents = Vec2U{ imageInfo.width, imageInfo.height };
  m_diffuseImage.format = Texture_Format_RGBA32;
  m_diffuseImage.usage = Texture_Usage_Shader_Input;
  m_diffuseImage.filtering = Texture_Filter_Linear;
  m_diffuseImage.sampleMode = Texture_Sample_Clamp;
  m_diffuseImage.Init(diffuseOpalImage);

  // Shutdown ==============================

  diffuseMaterial.Shutdown();
  OpalFramebufferShutdown(&diffuseFramebuffer);
  OpalRenderpassShutdown(&diffuseRenderpass);
  return Quartz_Success;
}

// Specular
// ============================================================

QuartzResult TextureSkybox::CreateSpecular(const Mesh& screenQuadMesh)
{
  // Number of mip/roughness levels to create and render
  const uint32_t levelCount = 5;

  struct SpecularMipInfo
  {
    Buffer buffer;
    Material material;
    OpalImage mipImage;
    OpalFramebuffer framebuffer;
  };
  SpecularMipInfo specularMips[levelCount];

  Buffer dummyBuffer;
  dummyBuffer.Init(1);

  // Image creation ==============================

  OpalImageInitInfo imageInfo = {};
  imageInfo.height = 512;
  imageInfo.width = imageInfo.height * ((float)extents.width / (float)extents.height);
  imageInfo.mipCount = levelCount;
  // For :          Rendering to            | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Output | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RGBA32;
  imageInfo.filter = Opal_Image_Filter_Linear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage specularOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&specularOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  const uint32_t subpassCount = 1;
  OpalAttachmentUsage attachmentUses[subpassCount] = { Opal_Attachment_Usage_Output_Uniform };
  OpalAttachmentInfo attachment;
  attachment.clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachment.format = Opal_Format_RGBA32;
  attachment.loadOp = Opal_Attachment_Load_Op_Clear;
  attachment.shouldStore = true;
  attachment.pSubpassUsages = attachmentUses;

  OpalRenderpassInitInfo renderpassInfo = {};
  renderpassInfo.attachmentCount = attachmentCount;
  renderpassInfo.pAttachments = &attachment;
  renderpassInfo.subpassCount = subpassCount;

  OpalRenderpass specularRenderpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&specularRenderpass, renderpassInfo));

  // Material (base) ==============================

  Material specularMaterial;
  specularMaterial.m_renderpass = specularRenderpass;

  ShaderSourceInfo vertInfo, fragInfo;
  vertInfo.size = resSkyboxVertShaderByteCount;
  vertInfo.data = (void*)resSkyboxVertShaderBytes;
  fragInfo.size = resSkyboxSpecularFragShaderByteCount;
  fragInfo.data = (void*)resSkyboxSpecularFragShaderBytes;

  QTZ_ATTEMPT(specularMaterial.Init(
    vertInfo, fragInfo,
    {
      { .type = Quartz::Input_Texture, .value = { .texture = &m_baseImage } },
      { .type = Quartz::Input_Buffer , .value = { .buffer  = &dummyBuffer } }
    },
    specularRenderpass,
    Quartz::Pipeline_Cull_None));

  // Mips ==============================

  for (uint32_t i = 0; i < levelCount; i++)
  {
    VkResult vkResult;
    SpecularMipInfo& m = specularMips[i];

    QTZ_ATTEMPT_OPAL(OpalImageGetMipAsImage(&specularOpalImage, &m.mipImage, i));

    // Framebuffer =====

    OpalImage* fbImages[] = { &m.mipImage };
    OpalFramebufferInitInfo fbInfo;
    fbInfo.imageCount = 1;
    fbInfo.ppImages = &fbImages[0];
    fbInfo.renderpass = specularRenderpass;

    QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&m.framebuffer, fbInfo));

    // Buffer =====

    float value = float(i) / float(levelCount - 1);
    m.buffer.Init(sizeof(float));
    m.buffer.PushData(&value);

    // Material =====

    QTZ_ATTEMPT(m.material.Init(
      specularMaterial,
      {
        { .texture = &m_baseImage },
        { .buffer  = &m.buffer    }
      }));
  }

  // Render ==============================

  OpalWaitIdle();
  QTZ_ATTEMPT_OPAL(OpalRenderBegin());

  for (uint32_t i = 0; i < levelCount; i++)
  {
    SpecularMipInfo& m = specularMips[i];

    uint32_t mipHeight = imageInfo.height * pow(0.5, i);
    uint32_t mipWidth  = imageInfo.width  * pow(0.5, i);

    // Begin Renderpass =====

    OpalRenderRenderpassBegin(&specularRenderpass, &m.framebuffer);

    // Material =====

    QTZ_ATTEMPT(m.material.Bind());
    OpalRenderSetViewportDimensions(mipWidth, mipHeight);

    OpalRenderBindShaderInput(Renderer::SceneSet(), 0);
    OpalRenderBindShaderInput(&m.material.m_inputSet, 1);

    // Mesh =====

    screenQuadMesh.Render();

    // End renderpass =====

    OpalRenderRenderpassEnd(&specularRenderpass);
  }
  OpalSyncPack syncpack = {};
  QTZ_ATTEMPT_OPAL(OpalRenderEnd(syncpack));
  OpalWaitIdle();

  // Convert to texture ==============================

  m_specularImage.mipLevels = levelCount;
  m_specularImage.extents = Vec2U{ imageInfo.width, imageInfo.height };
  m_specularImage.format = Texture_Format_RGBA32;
  m_specularImage.usage = Texture_Usage_Shader_Input;
  m_specularImage.filtering = Texture_Filter_Linear;
  m_specularImage.sampleMode = Texture_Sample_Clamp;
  m_specularImage.Init(specularOpalImage);

  // Shutdown ==============================

  for (uint32_t i = 0; i < levelCount; i++)
  {
    SpecularMipInfo& m = specularMips[i];

    m.material.Shutdown();
    m.buffer.Shutdown();
    OpalFramebufferShutdown(&m.framebuffer);
    OpalImageShutdown(&m.mipImage);
  }

  specularMaterial.Shutdown();
  dummyBuffer.Shutdown();
  OpalRenderpassShutdown(&specularRenderpass);
  return Quartz_Success;
}

// Brdf
// ============================================================

QuartzResult TextureSkybox::CreateBrdf(const Mesh& screenQuadMesh)
{
  // Image creation ==============================

  OpalImageInitInfo imageInfo = {};
  imageInfo.height = m_brdfSize;
  imageInfo.width = m_brdfSize;
  imageInfo.mipCount = 1;
  // For :          Rendering to            | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Output | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RG16;
  imageInfo.filter = Opal_Image_Filter_Linear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage brdfOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&brdfOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  const uint32_t subpassCount = 1;
  OpalAttachmentUsage attachmentUses[subpassCount] = { Opal_Attachment_Usage_Output_Uniform };
  OpalAttachmentInfo attachment;
  attachment.clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachment.format = Opal_Format_RG16;
  attachment.loadOp = Opal_Attachment_Load_Op_Clear;
  attachment.shouldStore = true;
  attachment.pSubpassUsages = attachmentUses;

  OpalRenderpassInitInfo renderpassInfo = {};
  renderpassInfo.attachmentCount = attachmentCount;
  renderpassInfo.pAttachments = &attachment;
  renderpassInfo.subpassCount = subpassCount;

  OpalRenderpass brdfRenderpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&brdfRenderpass, renderpassInfo));

  // Framebuffer ==============================

  OpalImage* fbImages[] = { &brdfOpalImage };
  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.ppImages = &fbImages[0];
  framebufferInfo.renderpass = brdfRenderpass;

  OpalFramebuffer brdfFramebuffer;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&brdfFramebuffer, framebufferInfo));

  // Material ==============================

  Material brdfMaterial;
  brdfMaterial.m_renderpass = brdfRenderpass;

  ShaderSourceInfo vertInfo, fragInfo;
  vertInfo.size = resSkyboxVertShaderByteCount;
  vertInfo.data = (void*)resSkyboxVertShaderBytes;
  fragInfo.size = resSkyboxBrdfFragShaderByteCount;
  fragInfo.data = (void*)resSkyboxBrdfFragShaderBytes;

  QTZ_ATTEMPT(brdfMaterial.Init(
    vertInfo, fragInfo,
    { },
    brdfRenderpass,
    Quartz::Pipeline_Cull_None));

  // Render ==============================

  OpalWaitIdle();
  QTZ_ATTEMPT_OPAL(OpalRenderBegin());
  OpalRenderRenderpassBegin(&brdfRenderpass, &brdfFramebuffer);
  QTZ_ATTEMPT(brdfMaterial.Bind());
  screenQuadMesh.Render();
  OpalRenderRenderpassEnd(&brdfRenderpass);
  OpalSyncPack syncpack = {};
  QTZ_ATTEMPT_OPAL(OpalRenderEnd(syncpack));
  OpalWaitIdle();

  // Convert to texture ==============================

  m_brdfImage.mipLevels = 1;
  m_brdfImage.extents = Vec2U{ m_brdfSize, m_brdfSize };
  m_brdfImage.format = Texture_Format_RG16;
  m_brdfImage.usage = Texture_Usage_Shader_Input;
  m_brdfImage.filtering = Texture_Filter_Linear;
  m_brdfImage.sampleMode = Texture_Sample_Clamp;
  m_brdfImage.Init(brdfOpalImage);

  // Shutdown ==============================

  brdfMaterial.Shutdown();
  OpalFramebufferShutdown(&brdfFramebuffer);
  OpalRenderpassShutdown(&brdfRenderpass);
  return Quartz_Success;
}

} // namespace Quartz
