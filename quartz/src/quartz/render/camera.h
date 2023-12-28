#pragma once

#include "quartz/defines.h"
#include "quartz/render/defines.h"

#include <opal.h>
#include <peridot.h>

namespace Quartz
{

class Camera
{
public:
  ~Camera() { Shutdown(); }

  QuartzResult Init(Vec2U resolution, float fovY, float nearClip = 0.1f, float farClip = 10.0f, float guaranteedRatio = 1.0f)
  {
    OpalImageInitInfo imageInfo;
    imageInfo.extent.width = resolution.width;
    imageInfo.extent.height = resolution.height;
    imageInfo.extent.depth = 1;
    imageInfo.format = Opal_Format_RGBA8;
    imageInfo.sampleType = Opal_Sample_Bilinear;
    imageInfo.usage = Opal_Image_Usage_Color;

    QTZ_ATTEMPT_OPAL(OpalImageInit(&m_targetImage, imageInfo));

    imageInfo.format = Opal_Format_D24_S8;
    imageInfo.usage = Opal_Image_Usage_Depth;
    QTZ_ATTEMPT_OPAL(OpalImageInit(&m_depthImage, imageInfo));

    m_fovY = fovY;
    m_resolution = resolution;
    m_ratio = (float)resolution.x / (float)resolution.y;
    m_nearClip = nearClip;
    m_farClip = farClip;
    m_guaranteedRatio = guaranteedRatio;
    UpdateProjection();

    return Quartz_Success;
  }

  void Shutdown()
  {
    if (m_targetImage == OPAL_NULL_HANDLE)
      return;

    OpalImageShutdown(&m_depthImage);
    OpalImageShutdown(&m_targetImage);
  }

  void SetAll(Vec2U resolution, float fovY, float nearClip = 0.1f, float farClip = 10.0f, float guaranteedRatio = 1.0f)
  {
    m_fovY = fovY;
    m_nearClip = nearClip;
    m_farClip = farClip;
    m_guaranteedRatio = guaranteedRatio;

    SetResolution(resolution); // Also updates projection
  }

  Vec2U Resolution() { return m_resolution; }
  void SetResolution(Vec2U resolution)
  {
    OpalExtent newResolution;
    newResolution.width = resolution.width;
    newResolution.height = resolution.height;
    newResolution.depth = 1;

    if (OpalImageResize(m_targetImage, newResolution))
    {
      QTZ_LOG_CORE_ERROR("Opal : Failed to resize camera target image");
      UpdateProjection();
      return;
    }
    if (OpalImageResize(m_depthImage, newResolution))
    {
      QTZ_LOG_CORE_ERROR("Opal : Failed to resize camera depth image");
      UpdateProjection();
      return;
    }

    // Only update resolution if the images were successfully resized
    m_resolution = resolution;
    m_ratio = (float)resolution.x / (float)resolution.y;

    UpdateProjection();
  }
  float ResolutionRatio() { return m_ratio; }

  float GuaranteedRatio() { return m_guaranteedRatio; }
  void SetGuaranteedRatio(float ratio)
  {
    m_guaranteedRatio = ratio;
    UpdateProjection();
  }

  float NearClip() { return m_nearClip; }
  float FarClip() { return m_farClip; }
  void SetNearClip(float clip)
  {
    m_nearClip = clip;
    UpdateProjection();
  }
  void SetFarClip(float clip)
  {
    m_farClip = clip;
    UpdateProjection();
  }

  float FovY() { return m_fovY; }
  void SetFovY(float fovY)
  { 
    m_fovY = fovY;
    UpdateProjection();
  }
  float FovX()
  {
    float fovY = PERI_DEGREES_TO_RADIANS(m_fovY);
    float ratio = (float)m_resolution.x / (float)m_resolution.y;
    return PERI_RADIANS_TO_DEGREES(2 * atan(tan(fovY / 2.0f) * ratio));
  }
  void SetFovX(float fovX)
  {
    fovX = PERI_DEGREES_TO_RADIANS(fovX);
    float ratio = (float)m_resolution.y / (float)m_resolution.x;
    m_fovY = PERI_RADIANS_TO_DEGREES(2 * atan(tan(fovX / 2.0f) * ratio));
    UpdateProjection();
  }

  Mat4 GetProjection() { return m_projectionMat; }

private:
  Vec2U m_resolution = { 256, 256 };
  float m_guaranteedRatio = 1.0f;
  float m_ratio = 1.0f;
  float m_nearClip = 0.1f, m_farClip = 10.0f;
  float m_fovY = 45.0f;
  Mat4 m_projectionMat = mat4Identity;

  void UpdateProjection()
  {
    m_projectionMat = ProjectionPerspectiveExtended(m_ratio, m_guaranteedRatio, m_fovY, m_nearClip, m_farClip);
  }

  OpalImage m_targetImage;
  OpalImage m_depthImage;

};

} // namespace Quartz