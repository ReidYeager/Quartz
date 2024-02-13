#pragma once

#include "quartz/defines.h"
#include <diamond.h>

namespace Quartz
{

// Types
// ============================================================

typedef Diamond::ComponentId ComponentId;

// Declarations
// ============================================================

ComponentId __DefineComponent(const char* name, size_t size);
ComponentId __ComponentId(const char* name);
#define QuartzDefineComponent(type) Quartz::__DefineComponent(typeid(type).name(), sizeof(type))
#define QuartzComponentId(type) Quartz::__ComponentId(typeid(type).name())

bool __HasComponent(Diamond::Entity entity, ComponentId id);
void* __AddComponent(Diamond::Entity entity, ComponentId id);
void* __GetComponent(Diamond::Entity entity, ComponentId id);
void __RemoveComponent(Diamond::Entity entity, ComponentId id);

// ECS
// ============================================================

class Entity
{
public:
  Entity();
  ~Entity();

  inline bool IsEnabled() const;
  void Enable();
  void Disable();

  template<typename T>
  inline bool Has() { return __HasComponent(m_id, Quartz::__ComponentId(typeid(T).name())); }
  template<typename T>
  inline void Remove() { __RemoveComponent(m_id, Quartz::__ComponentId(typeid(T).name())); }
  template<typename T>
  inline T* Add() { return (T*)__AddComponent(m_id, Quartz::__ComponentId(typeid(T).name())); }
  template<typename T>
  inline T* Get() { return (T*)__GetComponent(m_id, Quartz::__ComponentId(typeid(T).name())); }

private:
  Diamond::Entity m_id;
  bool m_isEnabled = true;
};

class ObjectIterator
{
public:
  ObjectIterator() : m_isValid(false) {};
  ObjectIterator(const std::vector<ComponentId>& componentIds);
  QuartzResult Init(const std::vector<ComponentId>& componentIds);

  void NextElement()
  {
    m_iterator.StepNextElement();
  }

  bool AtEnd()
  {
    return m_iterator.AtEnd();
  }

  template<typename T>
  T* Get()
  {
    return (T*)m_iterator.GetComponent(Quartz::__ComponentId(typeid(T).name()));
  }

  template<typename T>
  bool Has()
  {
    return m_iterator.HasComponent(Quartz::__ComponentId(typeid(T).name()));
  }

private:
  bool m_isValid = false;
  Diamond::EcsIterator m_iterator;
};

} // namespace Quartz
