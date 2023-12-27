
#include "quartz/core.h"
#include "quartz/ecs/ecs.h"

namespace Quartz
{

EcsWorld::EcsWorld()
{
  CreateArchetype({});
}

EcsWorld::~EcsWorld()
{
  
}

ComponentId EcsWorld::GetComponentId(const char* name)
{
  uint32_t hash = 0;
  for (uint32_t i = 0; name[i] != '\0'; i++)
  {
    hash = 31 * hash + name[i];
  }

  return hash;
}

ArchetypeId EcsWorld::GetArchetypeId(const std::vector<ComponentId>& components)
{
  // TODO : Sort the component list

  uint32_t hash = 0;
  for (uint32_t i = 0; i < components.size(); i++)
  {
    hash = 31 * hash + components[i];
  }

  return hash;
}

bool EcsWorld::EntityHasComponent(Entity e, ComponentId id)
{
  EcsRecord& record = m_records[e];
  return ArchetypeHasComponent(record.archetype, id);
}

bool EcsWorld::ArchetypeHasComponent(EcsArchetype* arch, ComponentId id)
{
  for (uint32_t i = 0; i < arch->componentIds.size(); i++)
  {
    if (arch->componentIds[i] == id)
      return true;
  }
  return false;
}

// ==========================================================================================
// Entity
// ==========================================================================================

Entity EcsWorld::CreateEntity()
{
  EcsRecord* record = &m_records[m_nextEntity];
  record->archetype = &m_archetypes[0];
  record->index = 0;

  return m_nextEntity++;
}

void EcsWorld::DestroyEntity(Entity e)
{
  
}

// ==========================================================================================
// Component
// ==========================================================================================

void EcsWorld::DefineComponent(ComponentId id, uint32_t size)
{
  m_componentSizes[id] = size;
}

void EcsWorld::AddComponent(Entity e, ComponentId id)
{
  EcsRecord* record = &m_records[e];
  EcsArchetype* src = record->archetype;
  EcsArchetype* dst = GetArchetypeNext(record->archetype, id);
  uint32_t srcIndex = record->index;

  record->archetype = dst;
  record->index = dst->componentData.elementCount();

  if (src->componentData.elementCount() == 0)
    return;

  // Copy data to next archetype
  void* srcData = src->componentData[srcIndex];
  void* dstData = dst->componentData.PushBack();
  uint32_t srcOffset = 0, dstOffset = 0;
  uint32_t curSize = 0;
  for (uint32_t i = 0; i < dst->componentIds.size(); i++)
  {
    curSize = m_componentSizes[dst->componentIds[i]];

    if (src->componentIds[i] != dst->componentIds[i])
    {
      memset(((char*)dstData) + dstOffset, 0, curSize);
      dstOffset += curSize;
      continue;
    }

    memcpy(((char*)dstData) + dstOffset, ((char*)srcData) + srcOffset, curSize);

    srcOffset += curSize;
    dstOffset += curSize;
  }

  dst->owningEntities.push_back(e);

  // Remove entity from src archetype
  void* srcBack = src->componentData[src->componentData.elementCount() - 1];
  memcpy(srcData, srcBack, src->componentData.elementSize());
  src->componentData.PopBack();

  src->owningEntities[srcIndex] = src->owningEntities[src->owningEntities.size() - 1];
  src->owningEntities.pop_back();
}

void EcsWorld::RemoveComponent(Entity e, ComponentId id)
{
  EcsRecord* record = &m_records[e];
  EcsArchetype* src = record->archetype;
  EcsArchetype* dst = GetArchetypePrevious(record->archetype, id);
  uint32_t srcIndex = record->index;

  record->archetype = dst;
  record->index = dst->componentData.elementCount() - 1;

  if (dst->componentData.elementCount() == 0)
    return;

  // Copy data to previous archetype
  void* srcData = src->componentData[srcIndex];
  void* dstData = dst->componentData.PushBack();
  uint32_t srcOffset = 0, dstOffset = 0;
  uint32_t curSize = 0;
  for (uint32_t i = 0; i < src->componentIds.size(); i++)
  {
    curSize = m_componentSizes[src->componentIds[i]];

    if (src->componentIds[i] != dst->componentIds[i])
    {
      srcOffset += curSize;
      continue;
    }

    memcpy(((char*)dstData) + dstOffset, ((char*)srcData) + srcOffset, curSize);

    srcOffset += curSize;
    dstOffset += curSize;
  }

  dst->owningEntities.push_back(e);

  // Remove entity from src archetype
  void* srcBack = src->componentData[src->componentData.elementCount() - 1];
  memcpy(srcData, srcBack, src->componentData.elementSize());
  src->componentData.PopBack();

  src->owningEntities[srcIndex] = src->owningEntities[src->owningEntities.size() - 1];
  src->owningEntities.pop_back();
}

// ==========================================================================================
// Archetype
// ==========================================================================================

EcsArchetype* EcsWorld::CreateArchetype(const std::vector<ComponentId>& components)
{
  ArchetypeId id = GetArchetypeId(components);

  if (m_archetypes.contains(id))
  {
    return &m_archetypes[id];
  }

  EcsArchetype* newArchetype = &m_archetypes[id];

  newArchetype->id = id;
  newArchetype->componentIds = components;
  newArchetype->owningEntities = {};
  newArchetype->additionalEdges = {};
  newArchetype->subtractionalEdges = {};

  uint32_t sizeSum = 0;
  for (uint32_t i = 0; i < components.size(); i++)
  {
    newArchetype->componentDataOffsets[components[i]] = sizeSum;
    sizeSum += m_componentSizes[components[i]];
  }
  newArchetype->componentData.Init(sizeSum);

  // Connect edges to other archetypes -- prevents any nodes from remaining unconnected
  // TODO : This may cause some hitching issues later. Profile it.
  for (uint32_t i = 0; i < components.size(); i++)
  {
    ComponentId removedId = components[i];
    EcsArchetype* prevArch = GetArchetypePrevious(newArchetype, removedId);
    newArchetype->subtractionalEdges[removedId] = prevArch;
    prevArch->additionalEdges[removedId] = newArchetype;
  }

  return newArchetype;
}

EcsArchetype* EcsWorld::GetArchetypeNext(EcsArchetype* arch, ComponentId id)
{
  if (!arch->additionalEdges.contains(id))
  {
    std::vector<ComponentId> nextIds(arch->componentIds);
    nextIds.push_back(id);

    // Keep sorted
    for (int32_t i = nextIds.size() - 2; i >= 0; i--)
    {
      if (nextIds[i] > id)
      {
        nextIds[i + 1] = nextIds[i];
        nextIds[i] = id;
      }
      else
      {
        break;
      }
    }

    arch->additionalEdges[id] = CreateArchetype(nextIds);
  }

  return arch->additionalEdges[id];
}

EcsArchetype* EcsWorld::GetArchetypePrevious(EcsArchetype* arch, ComponentId id)
{
  if (arch->subtractionalEdges.contains(id))
  {
    return arch->subtractionalEdges[id];
  }

  if (arch->componentIds.size() == 0)
    return nullptr;

  std::vector<ComponentId> prevIds(arch->componentIds);

  ComponentId prevId = prevIds[prevIds.size() - 1];
  if (prevId != id)
  {
    for (int32_t i = prevIds.size() - 2; i >= 0; i--)
    {
      if (prevIds[i] == id)
      {
        prevIds[i] = prevId;
        break;
      }

      ComponentId tmp = prevIds[i];
      prevIds[i] = prevId;
      prevId = tmp;
    }
  }
  prevIds.pop_back();

  EcsArchetype* newArch = CreateArchetype(prevIds);
  arch->subtractionalEdges[id] = newArch;
  return newArch;
}

void* EcsWorld::GetComponentData(EcsRecord* record, ComponentId id)
{
  char* data = (char*)record->archetype->componentData[record->index];
  return (data + record->archetype->componentDataOffsets[id]);
}

// ==========================================================================================
// Debug prints
// ==========================================================================================

void EcsWorld::PrintNextEdges()
{
  for (std::pair<ArchetypeId, EcsArchetype> ap : m_archetypes)
  {
    EcsArchetype& arch = ap.second;

    QTZ_LOG_CORE_DEBUG("Arch ({}) : {}", arch.id, arch.additionalEdges.size());
    for (std::pair<ComponentId, EcsArchetype*> edge : arch.additionalEdges)
    {
      QTZ_LOG_CORE_DEBUG(">> {}", edge.second->id);
    }
  }
}

void EcsWorld::PrintPrevEdges()
{
  for (std::pair<ArchetypeId, EcsArchetype> ap : m_archetypes)
  {
    EcsArchetype& arch = ap.second;

    QTZ_LOG_CORE_DEBUG("Arch ({}) : {}", arch.id, arch.subtractionalEdges.size());
    for (std::pair<ComponentId, EcsArchetype*> edge : arch.subtractionalEdges)
    {
      QTZ_LOG_CORE_DEBUG(">> {}", edge.second->id);
    }
  }
}

} // namespace Quartz
