#pragma once

#include "quartz/core.h"
#include "quartz/events/event.h"

namespace Quartz
{

class EventWindowClose : public Event
{
public:
  EventWindowClose() {}
  QTZ_EVENT_DEFINE_TYPE(Event_Window_Close);
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Window);

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window closed";
    return stream.str();
  }
}; // class EventWindowClose

class EventWindowResize : public Event
{
public:
  EventWindowResize(uint32_t newWidth, uint32_t newHeight) : m_newWidth(newWidth), m_newHeight(newHeight) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Window_Resize);
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Window);

  inline uint32_t GetWidth() { return m_newWidth; }
  inline uint32_t GetHeight() { return m_newHeight; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window resized to : (" << m_newWidth << ", " << m_newHeight << ")";
    return stream.str();
  }

private:
  uint32_t m_newWidth;
  uint32_t m_newHeight;
}; // class EventWindowResize

class EventWindowMove : public Event
{
public:
  EventWindowMove(uint32_t newX, uint32_t newY) : m_newX(newX), m_newY(newY) {}
  QTZ_EVENT_DEFINE_TYPE(Event_Window_Move);
  QTZ_EVENT_DEFINE_CATEGORIES(EventCategory_Window);

  inline uint32_t GetX() { return m_newX; }
  inline uint32_t GetY() { return m_newY; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window moved to : (" << m_newX << ", " << m_newY << ")";
    return stream.str();
  }

private:
  uint32_t m_newX;
  uint32_t m_newY;
}; // class EventWindowMove

} // namespace Quartz
