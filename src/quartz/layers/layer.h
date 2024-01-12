#pragma once

#include "quartz/defines.h"
#include "quartz/events/event.h"

namespace Quartz
{

class Layer
{
public:
  Layer(const char* name = "DefaultLayerName") : m_name(name) {}
  virtual ~Layer() {}
  inline const std::string& GetName_Debug() { return m_name; }

  virtual void OnAttach() {}
  virtual void OnDetach() {}
  virtual void OnUpdate() {}
  virtual void OnUpdateImgui() {}
  virtual void OnEvent(Event& e) {}

protected:
  std::string m_name;
};

} // namespace Quartz
