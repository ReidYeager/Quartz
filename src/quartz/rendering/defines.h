#pragma once

#include "quartz/defines.h"

#include <opal.h>

namespace Quartz
{

#define QTZ_ATTEMPT_OPAL(fn, ...)                                                                    \
{                                                                                                    \
  Quartz::g_quartzAttemptDepth++;                                                                    \
  OpalResult result = fn;                                                                            \
  Quartz::g_quartzAttemptDepth--;                                                                    \
  if (result != Opal_Success)                                                                        \
  {                                                                                                  \
    QTZ_ERROR("Opal : {} : \"{}\"\n\t{}:{}", Quartz::g_quartzAttemptDepth, #fn, __FILE__, __LINE__); \
    {                                                                                                \
      __VA_ARGS__;                                                                                   \
    }                                                                                                \
    return Quartz_Failure_Vendor;                                                                    \
  }                                                                                                  \
}

struct Vertex
{
  Vec3 position;
  Vec2 uv;
  Vec3 normal;
  Vec3 tangent;

  bool operator==(const Vertex& other) const
  {
    return Vec3Compare(position, other.position)
      && Vec3Compare(normal, other.normal)
      && Vec2Compare(uv, other.uv)
      && Vec3Compare(tangent, other.tangent);
  }
};

struct LightDirectional
{
  Vec3 color;
  float intensity;
  Vec3 direction;
};
#define QTZ_LIGHT_DIRECTIONAL_MAX_COUNT 1

struct LightPoint
{
  Vec3 color;
  float intensity;
  Vec3 position;
};
#define QTZ_LIGHT_POINT_MAX_COUNT 4

struct LightSpot
{
  Vec3 color;
  float intensity;
  Vec3 position;
  Vec3 direction;

  float inner;
  float outer;
};
#define QTZ_LIGHT_SPOT_MAX_COUNT 2

struct Renderable
{
  class Mesh* mesh;
  class Material* material;
  Mat4 transformMatrix;
};

struct Camera
{
  float fov;
  float desiredRatio;
  float nearClip;
  float farClip;
  Mat4 projectionMatrix;
  Mat4 viewProjectionMatrix;
  Vec3 pos;
};

} // namespace Quartz