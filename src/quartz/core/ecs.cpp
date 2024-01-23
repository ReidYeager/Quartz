
#include "quartz/defines.h"
#include "quartz/core/core.h"
#include "quartz/core/ecs.h"

namespace Quartz
{

ComponentId __DefineComponent(const char* name, size_t size)
{
  return (ComponentId)g_coreState.ecsWorld.DefineComponent(name, size);
}

ComponentId __ComponentId(const char* name)
{
  return g_coreState.ecsWorld.GetComponentId(name);
}

bool __HasComponent(Diamond::Entity entity, ComponentId id)
{
  return g_coreState.ecsWorld.EntityHasComponent(entity, (Diamond::ComponentId)id);
}

void* __AddComponent(Diamond::Entity entity, ComponentId id)
{
  return g_coreState.ecsWorld.AddComponent(entity, (Diamond::ComponentId)id);
}

void* __GetComponent(Diamond::Entity entity, ComponentId id)
{
  return g_coreState.ecsWorld.GetComponent(entity, (Diamond::ComponentId)id);
}

void __RemoveComponent(Diamond::Entity entity, ComponentId id)
{
  g_coreState.ecsWorld.RemoveComponent(entity, (Diamond::ComponentId)id);
}

// ECS
// ============================================================

Entity::Entity()
{
  m_id = g_coreState.ecsWorld.CreateEntity();
  __AddComponent(m_id, QuartzComponentId(Transform));
}

Entity::~Entity()
{
  g_coreState.ecsWorld.DestroyEntity(m_id);
}

ObjectIterator::ObjectIterator(const std::vector<ComponentId>& componentIds)
{
  m_iterator = new Diamond::EcsIterator(&g_coreState.ecsWorld, componentIds);
}

} // namespace Quartz
