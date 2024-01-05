
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

class ObjectIterator
{
public:
  ObjectIterator(Diamond::EcsWorld* world, const std::vector<const char*> componentNames) : m_world(world)
  {
    std::vector<Diamond::ComponentId> ids(componentNames.size());
    for (uint32_t i = 0; i < componentNames.size(); i++)
    {
      ids[i] = world->GetComponentId(componentNames[i]);
    }

    m_iterator = new Diamond::EcsIterator(world, ids);
  }
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

  void* GetComponentValue(const char* comonentName)
  {
    Diamond::ComponentId id = m_world->GetComponentId(comonentName);
    return m_iterator->GetComponent(id);
  }

private:
  Diamond::EcsWorld* m_world;
  Diamond::EcsIterator* m_iterator;
};

Renderable* CreateObject();
void DestroyObject(Renderable* object);
ObjectIterator CreateIterator(const std::vector<const char*>& componentNames);

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
