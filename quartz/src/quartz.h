
#pragma once

#include "quartz/defines.h"
#include "quartz/logging/logger.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/rendering.h"
#include "quartz/layers/layer.h"
#include "quartz/platform/input/input.h"

#include <diamond.h>

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
Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

// ============================================================
// Layers
// ============================================================

void PushLayer(Layer* layer);
void PopLayer(Layer* layer);

// ============================================================
// Objects
// ============================================================

typedef Diamond::ComponentId ComponentId;

class Entity
{
public:
  Entity();
  ~Entity();

private:
  Diamond::Entity m_id;
};

class ObjectIterator
{
public:
  ObjectIterator(const std::vector<ComponentId>& componentIds);

  ~ObjectIterator()
  {
    delete m_iterator;
  }

  void NextElement()
  {
    m_iterator->StepNextElement();
  }

  bool AtEnd()
  {
    return m_iterator->AtEnd();
  }

  void* GetComponentValue(ComponentId id);

private:
  Diamond::EcsIterator* m_iterator;
};

ComponentId _DefineComponent(const char* name, size_t size);
ComponentId _ComponentId(const char* name);
#define QuartzDefineComponent(type) Quartz::_DefineComponent(#type, sizeof(type))
#define QuartzComponentId(type) Quartz::_ComponentId(#type)

Renderable* CreateObject();
void DestroyObject(Renderable* object);

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
