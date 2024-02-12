
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
  imageInfo.extent.height = 64;
  imageInfo.extent.width = imageInfo.extent.height * ((float)extents.width / (float)extents.height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  // For :          Rendering to           | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RGBA32;
  imageInfo.filterType = Opal_Image_Filter_Bilinear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage diffuseOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&diffuseOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  OpalAttachmentInfo attachment;
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

  OpalRenderpass diffuseRenderpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&diffuseRenderpass, renderpassInfo));

  // Framebuffer ==============================

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.pImages = &diffuseOpalImage;
  framebufferInfo.renderpass = diffuseRenderpass;

  OpalFramebuffer diffuseFramebuffer;
  QTZ_ATTEMPT_OPAL(OpalFramebufferInit(&diffuseFramebuffer, framebufferInfo));

  // Material ==============================

  Material diffuseMaterial;
  diffuseMaterial.m_renderpass = diffuseRenderpass;

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
    Quartz::Pipeline_Cull_None));

  // Render ==============================

  QTZ_ATTEMPT_OPAL(OpalRenderBeginSingle());
  OpalRenderBeginRenderpass(diffuseRenderpass, diffuseFramebuffer);
  QTZ_ATTEMPT(diffuseMaterial.Bind());
  screenQuadMesh.Render();
  OpalRenderEndRenderpass(diffuseRenderpass);
  QTZ_ATTEMPT_OPAL(OpalRenderEndSingle());

  // Convert to texture ==============================

  QTZ_ATTEMPT_OPAL(OpalImageTransition(diffuseOpalImage, Opal_Image_Usage_Uniform));

  m_diffuseImage.mipLevels = 1;
  m_diffuseImage.extents = Vec2U{ imageInfo.extent.width, imageInfo.extent.height };
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
    VkImageView imageView;
    VkFramebuffer framebuffer;
  };
  SpecularMipInfo specularMips[levelCount];

  Buffer dummyBuffer;
  dummyBuffer.Init(1);

  // Image creation ==============================

  OpalImageInitInfo imageInfo = {};
  imageInfo.extent.height = imageInfo.extent.height = 512;
  imageInfo.extent.width = imageInfo.extent.height * ((float)extents.width / (float)extents.height);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = levelCount;
  // For :          Rendering to           | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RGBA32;
  imageInfo.filterType = Opal_Image_Filter_Bilinear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage specularOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&specularOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  OpalAttachmentInfo attachment;
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
    Quartz::Pipeline_Cull_None));

  // Mips ==============================

  for (uint32_t i = 0; i < levelCount; i++)
  {
    VkResult vkResult;
    SpecularMipInfo& m = specularMips[i];

    // Image view =====

    VkImageViewCreateInfo viewCreateInfo = { 0 };
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.image = specularOpalImage->vk.image;
    viewCreateInfo.format = specularOpalImage->vk.format;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.baseMipLevel = i;
    vkResult = vkCreateImageView(oState.vk.device, &viewCreateInfo, oState.vk.allocator, &m.imageView);

    if (vkResult != VK_SUCCESS)
    {
      QTZ_ERROR("Failed to initialize the image view for specular mip {}", i);
      return Quartz_Failure_Vendor;
    }

    // Framebuffer =====

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.flags = 0;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &m.imageView;
    framebufferInfo.height = imageInfo.extent.height * pow(0.5, i);
    framebufferInfo.width  = imageInfo.extent.width  * pow(0.5, i);
    framebufferInfo.layers = 1;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.renderPass = specularRenderpass->vk.renderpass;
    vkResult = vkCreateFramebuffer(oState.vk.device, &framebufferInfo, oState.vk.allocator, &m.framebuffer);

    if (vkResult != VK_SUCCESS)
    {
      QTZ_ERROR("Failed to initialize the framebuffer for specular mip {}", i);
      return Quartz_Failure_Vendor;
    }

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

  QTZ_ATTEMPT_OPAL(OpalRenderBeginSingle());
  for (uint32_t i = 0; i < levelCount; i++)
  {
    SpecularMipInfo& m = specularMips[i];

    uint32_t mipHeight = imageInfo.extent.height * pow(0.5, i);
    uint32_t mipWidth  = imageInfo.extent.width  * pow(0.5, i);

    // Begin Renderpass =====

    VkRenderPassBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = specularRenderpass->vk.renderpass;
    beginInfo.clearValueCount = specularRenderpass->imageCount;
    beginInfo.pClearValues = specularRenderpass->vk.pClearValues;
    beginInfo.renderArea.offset = VkOffset2D{ 0, 0 };
    beginInfo.framebuffer = m.framebuffer;
    beginInfo.renderArea.extent = VkExtent2D{ mipWidth, mipHeight };
    vkCmdBeginRenderPass(oState.vk.currentCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Material =====

    QTZ_ATTEMPT(m.material.Bind());
    OpalRenderSetViewportDimensions(mipWidth, mipHeight);

    OpalRenderBindInputSet(Renderer::SceneSet(), 0);
    OpalRenderBindInputSet(m.material.m_set, 1);

    // Mesh =====

    screenQuadMesh.Render();

    // End renderpass =====

    vkCmdEndRenderPass(oState.vk.currentCommandBuffer);
  }
  QTZ_ATTEMPT_OPAL(OpalRenderEndSingle());

  // Convert to texture ==============================

  QTZ_ATTEMPT_OPAL(OpalImageTransition(specularOpalImage, Opal_Image_Usage_Uniform));

  m_specularImage.mipLevels = levelCount;
  m_specularImage.extents = Vec2U{ imageInfo.extent.width, imageInfo.extent.height };
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
    vkDestroyFramebuffer(oState.vk.device, m.framebuffer, oState.vk.allocator);
    vkDestroyImageView(oState.vk.device, m.imageView, oState.vk.allocator);
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
  imageInfo.extent.height = m_brdfSize;
  imageInfo.extent.width = m_brdfSize;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  // For :          Rendering to           | Use in pbr shaders
  imageInfo.usage = Opal_Image_Usage_Color | Opal_Image_Usage_Uniform;
  imageInfo.format = Opal_Format_RG16;
  imageInfo.filterType = Opal_Image_Filter_Bilinear;
  imageInfo.sampleMode = Opal_Image_Sample_Clamp;

  OpalImage brdfOpalImage;
  QTZ_ATTEMPT_OPAL(OpalImageInit(&brdfOpalImage, imageInfo));

  // Renderpass ==============================

  const uint32_t attachmentCount = 1;
  OpalAttachmentInfo attachment;
  attachment.clearValue.color = OpalColorValue{ 1.0f, 0.0f, 1.0f, 1.0f };
  attachment.format = Opal_Format_RG16;
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

  OpalRenderpass brdfRenderpass;
  QTZ_ATTEMPT_OPAL(OpalRenderpassInit(&brdfRenderpass, renderpassInfo));

  // Framebuffer ==============================

  OpalFramebufferInitInfo framebufferInfo;
  framebufferInfo.imageCount = 1;
  framebufferInfo.pImages = &brdfOpalImage;
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
    Quartz::Pipeline_Cull_None));

  // Render ==============================

  QTZ_ATTEMPT_OPAL(OpalRenderBeginSingle());
  OpalRenderBeginRenderpass(brdfRenderpass, brdfFramebuffer);
  QTZ_ATTEMPT(brdfMaterial.Bind());
  screenQuadMesh.Render();
  OpalRenderEndRenderpass(brdfRenderpass);
  QTZ_ATTEMPT_OPAL(OpalRenderEndSingle());

  // Convert to texture ==============================

  QTZ_ATTEMPT_OPAL(OpalImageTransition(brdfOpalImage, Opal_Image_Usage_Uniform));

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
