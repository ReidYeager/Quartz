#pragma once

#include "quartz/core.h"

#include <string>
#include <sstream>

namespace Quartz
{

// TODO : Buffer events into a queue and execute all events at the end/start of the current/next frame

enum EventType
{
  Event_Unknown = 0,

  //Event_Application_Update_Start,
  //Event_Application_Update_End,

  Event_Window_Close,
  Event_Window_Resize,
  Event_Window_Move,
  Event_Window_Focused,

  Event_Key_Press,
  Event_Key_Release,

  Event_Mouse_Move,
  Event_Mouse_Scroll,
  Event_Mouse_Button_Press,
  Event_Mouse_Button_Release
};

enum EventCategoryBits
{
  EventCategory_Unknown      = 0,
  EventCategory_Window       = (1 << 0),
  EventCategory_Input        = (1 << 1),
  EventCategory_Keyboard     = (1 << 2),
  EventCategory_Mouse        = (1 << 3),
  EventCategory_Mouse_Button = (1 << 4)
};
typedef uint32_t EventCategoryFlags;

#define QTZ_EVENT_DEFINE_TYPE(type)                                 \
static EventType GetTypeStatic() { return Quartz::type; }           \
virtual EventType GetType() const override { return Quartz::type; } \
virtual const char* GetName_Debug() const override { return #type; }

#define QTZ_EVENT_DEFINE_CATEGORIES(categories) \
virtual EventCategoryFlags GetCategories() const override { return categories; }

class Event
{
  friend class EventDispatcher;

public:
  virtual EventType GetType() const = 0;
  virtual EventCategoryFlags GetCategories() const = 0;
  virtual const char* GetName_Debug() const = 0;
  virtual std::string ToString_Debug() const { return GetName_Debug(); }

  inline bool HasCategory(EventCategoryBits category) { return GetCategories() & category; }

protected:
  bool m_handled = false;
}; // class Event

class EventDispatcher
{
public:
  EventDispatcher(Event& event) : m_event(event) {}

  template<typename T>
  bool Dispatch(std::function<bool(T&)> function)
  {
    if (m_event.GetType() == T::GetTypeStatic())
    {
      m_event.m_handled = function(*(T*)&m_event);
      return true;
    }
    return false;
  }

private:
  Event& m_event;
}; // class EventDispatcher

inline std::ostream& operator<<(std::ostream& os, const Event& e)
{
  return os << e.ToString_Debug();
}

} // namespace Quartz
