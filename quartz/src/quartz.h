
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

ComponentId _DefineComponent(const char* name, size_t size);
ComponentId _ComponentId(const char* name);
#define QuartzDefineComponent(type) Quartz::_DefineComponent(#type, sizeof(type))
#define QuartzComponentId(type) Quartz::_ComponentId(#type)

bool HasComponent(Diamond::Entity, ComponentId id);
void* AddComponent(Diamond::Entity, ComponentId id);
void* GetComponent(Diamond::Entity, ComponentId id);

class Entity
{
public:
  Entity();
  ~Entity();

  bool Has(ComponentId id) { return HasComponent(m_id, id); }
  void* Add(ComponentId id) { return AddComponent(m_id, id); }
  void* Get(ComponentId id) { return GetComponent(m_id, id); }

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
