#pragma once

#include "quartz/core.h"
#include "quartz/events/event.h"

namespace Quartz
{

class EventMouseMove : public Event
{
public:
  EventMouseMove(int newX, int newY) : m_newX(newX), m_newY(newY) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Mouse_Move);
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Input | EventCategory_Mouse);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Mouse Moved to : (" << m_newX << ", " << m_newY << ")";
    return stream.str();
  }

private:
  int m_newX;
  int m_newY;
}; // class EventMouseMove

class EventMouseScroll : public Event
{
public:
  EventMouseScroll(float amountX, float amountY) : m_amountX(amountX), m_amountY(amountY) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Mouse_Scroll);
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Input | EventCategory_Mouse);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Mouse scroll : (" << m_amountX << ", " << m_amountY << ")";
    return stream.str();
  }

private:
  float m_amountX;
  float m_amountY;
}; // class EventMouseScroll

class EventMouseButton : public Event
{
public:
  inline int GetButton() { return m_button; }
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Input | EventCategory_Mouse | EventCategory_Mouse_Button);

protected:
  EventMouseButton(int button) : m_button(button) {}
  int m_button;
}; // class EventMouseButton

class EventMouseButtonPress : public EventMouseButton
{
public:
  EventMouseButtonPress(int button) : EventMouseButton(button) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Mouse_Button_Press);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Mouse button press : (" << m_button << ")";
    return stream.str();
  }
}; // class EventMouseButtonPress

class EventMouseButtonRelease : public EventMouseButton
{
public:
  EventMouseButtonRelease(int button) : EventMouseButton(button) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Mouse_Button_Release);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Mouse button release : (" << m_button << ")";
    return stream.str();
  }
}; // class EventMouseButtonRelease

} // namespace Quartz
