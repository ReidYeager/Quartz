
#pragma once

#include "quartz/defines.h"
#include "quartz/logging/logger.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/rendering.h"
#include "quartz/layers/layer.h"
#include "quartz/platform/input/input.h"

#include <vector>

namespace Quartz {

void Run();
void Quit();

// ============================================================
// Window
// ============================================================

uint32_t WindowGetWidth();
uint32_t WindowGetHeight();

// ============================================================
// Rendering
// ============================================================

Material CreateMaterial(const std::vector<const char*>& shaderPaths);
//Mesh CreateMesh(const char* path);
Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
void SubmitForRender(Renderable& renderable);

// ============================================================
// Layers
// ============================================================

void PushLayer(Layer* layer);
void PopLayer(Layer* layer);

// ============================================================
// Msc
// ============================================================
struct Time_T
{
  double totalRealTime;
  double deltaTime;
  double totalTimeDeltaSum;
  uint32_t frameCount;
};
extern Time_T time;

} // namespace Quartz

extern Quartz::Layer* GetGameLayer();

#define QUARTZ_GAME_LAYER(layer)                 \
Quartz::Layer* GetGameLayer()                    \
{                                                \
  static Quartz::Layer* gameLayer = new layer(); \
  return gameLayer;                              \
}

#define QUARTZ_ENTRY_POINT \
int main()                 \
{                          \
  Quartz::Run();           \
  return 0;                \
}
