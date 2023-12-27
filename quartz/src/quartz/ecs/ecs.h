#pragma once

// Implementation of an Entity-Component-System archetecture
// Revolves around the idea of component "archetypes"
// 
// TODO : Rewrite without templates
// 
// TODO : VV Investigate if this is a good idea VV
// Data movement only occurs on calls to EcsWorld::Update()
// > This includes archetype creation and entity movement between archetypes
// > (Hopefully allows the system to be made somewhat thread-safe)
// > !!! Needs to be tested.

#include "quartz/core.h"
#include "quartz/containers/stepvector.h"

#include <unordered_map>
#include <memory>

namespace Quartz
{

typedef uint32_t ArchetypeId;
typedef uint32_t ComponentId;
typedef uint32_t Entity;

struct EcsArchetype
{
  ArchetypeId id;                           // Hash of constituent componentId's
  std::vector<Entity> owningEntities;       // Entity that "owns" each index
  std::vector<ComponentId> componentIds;    // For initializing subtractional edges

  // TODO : Profile interleaved component data vs separate comonent arrays
  Quartz::StepVector componentData;         // Memory for all components' data interleaved for each entity
  std::unordered_map<ComponentId, uint32_t> componentDataOffsets;

  std::unordered_map<ComponentId, EcsArchetype*> additionalEdges;    // Archetypes with 'ComponentId' added
  std::unordered_map<ComponentId, EcsArchetype*> subtractionalEdges; // Archetypes with 'ComponentId' removed
};

struct EcsRecord
{
  EcsArchetype* archetype;
  uint32_t index;
};

class EcsWorld
{
public:
  friend class EcsIterator;

  EcsWorld();
  ~EcsWorld();

  Entity CreateEntity();
  void DestroyEntity(Entity e);

  ComponentId GetComponentId(const char* name);
  void DefineComponent(ComponentId id, uint32_t size);
  void AddComponent(Entity e, ComponentId id);
  void RemoveComponent(Entity e, ComponentId id);
  bool EntityHasComponent(Entity e, ComponentId id);

  void PrintNextEdges();
  void PrintPrevEdges();

private:
  Entity m_nextEntity = 1; // Entity value 0 invalid
  ComponentId m_nextComponentId = 1; // ComponentId value 0 invalid
  std::unordered_map<ComponentId, uint32_t> m_componentSizes;
  std::unordered_map<ArchetypeId, EcsArchetype> m_archetypes;
  std::unordered_map<Entity, EcsRecord> m_records; // NOTE : Could probably just be an array depending upon how Entity is structured

  ArchetypeId GetArchetypeId(const std::vector<ComponentId>& components);
  bool ArchetypeHasComponent(EcsArchetype* arch, ComponentId id);
  EcsArchetype* CreateArchetype(const std::vector<ComponentId>& components);
  EcsArchetype* GetArchetypeNext(EcsArchetype* arch, ComponentId id);
  EcsArchetype* GetArchetypePrevious(EcsArchetype* arch, ComponentId id);
  void* GetComponentData(EcsRecord* record, ComponentId id);
};

class EcsIterator
{
public:
};

// TODO : Iterator that traverses archetypes for use by systems

} // namespace Quartz
