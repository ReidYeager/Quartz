#pragma once

#include "quartz/defines.h"
#include "quartz/events/event.h"

namespace Quartz
{

class EventWindowClose : public Event
{
public:
  QTZ_EVENT_DEFINE_CONSTS(Event_Window_Close, Event_Category_Window)
  EventWindowClose() {}

  virtual std::string ToString_Debug() const override
  {
    return "Window closed";
  }
};

class EventWindowResize : public Event
{
public:
  QTZ_EVENT_DEFINE_CONSTS(Event_Window_Resize, Event_Category_Window)
  EventWindowResize(uint32_t newWidth, uint32_t newHeight) : m_width(newWidth), m_height(newHeight) {}

  inline uint32_t GetWidth() const { return m_width; }
  inline uint32_t GetHeight() const { return m_height; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window resized to (" << m_width << ", " << m_height << ")";
    return stream.str();
  }

private:
  uint32_t m_width, m_height;
};

class EventWindowMinimize : public Event
{
public:
  QTZ_EVENT_DEFINE_CONSTS(Event_Window_Minimize, Event_Category_Window)
    EventWindowMinimize(bool isMinimized) : m_isMinimized(isMinimized) {}

  inline bool GetIsMinimized() const { return m_isMinimized; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window " << (m_isMinimized ? "Minimized" : "Un-minimized");
    return stream.str();
  }

private:
  bool m_isMinimized;
};

class EventWindowMove : public Event
{
public:
  QTZ_EVENT_DEFINE_CONSTS(Event_Window_Move, Event_Category_Window)
  EventWindowMove(int32_t newX, int32_t newY) : m_xPos(newX), m_yPos(newY) {}

  inline int32_t GetXPos() const { return m_xPos; }
  inline int32_t GetYPos() const { return m_yPos; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window moved to (" << m_xPos << ", " << m_yPos << ")";
    return stream.str();
  }

private:
  int32_t m_xPos, m_yPos;
};

class EventWindowFocus : public Event
{
public:
  QTZ_EVENT_DEFINE_CONSTS(Event_Window_Focus, Event_Category_Window)
  EventWindowFocus(bool isFocused) : m_isFocused(isFocused) {}

  inline bool GetIsFocused() const { return m_isFocused; }

  virtual std::string ToString_Debug() const override
  {
    std::stringstream stream;
    stream << "Window " << ((m_isFocused) ? "focused" : "unfocused");
    return stream.str();
  }

private:
  bool m_isFocused;
};

} // namespace Quartz
