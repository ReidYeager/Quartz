#pragma once

#include "quartz/defines.h"
#include "quartz/events/event.h"

namespace Quartz
{

// TODO : Move keycodes to its own file
enum Keycode
{
  A = 'a',
};

class EventKey : public Event
{
public:
  inline int GetKeycode() const { return m_keycode; }
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Input | EventCategory_Keyboard);

protected:
  EventKey(int keycode) : m_keycode(keycode) {}

  int m_keycode;
}; // class EventKey

class EventKeyPress : public EventKey
{
public:
  EventKeyPress(int keycode) : EventKey(keycode) {}

  QTZ_EVENT_DEFINE_TYPE(Event_Key_Press);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Event : Key pressed : " << m_keycode << " ('" << (char)m_keycode << "')";
    return stream.str();
  }
}; // class EventKeyPress

class EventKeyRelease : public EventKey
{
public:
  EventKeyRelease(int keycode) : EventKey(keycode) {}

  QTZ_EVENT_DEFINE_TYPE(Event_Key_Release);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Event : Key released : " << m_keycode << " ('" << (char)m_keycode << "')";
    return stream.str();
  }
}; // class EventKeyRelease

} // namespace Quartz
