#pragma once

#include "quartz/defines.h"

namespace Quartz
{

enum EventType
{
  Event_Window_Close,
  Event_Window_Resize,
  Event_Window_Move,
  Event_Window_Focus,

  Event_Key_Press,
  Event_Key_Release,

  Event_Mouse_Move,
  Event_Mouse_Scroll,
  Event_Mouse_Button_Press,
  Event_Mouse_Button_Release,
};

enum EventCategoryBits
{
  Event_Category_Unknown      = 0,
  Event_Category_Window       = (1 << 0),
  Event_Category_Input        = (1 << 1),
  Event_Category_Keyboard     = (1 << 2),
  Event_Category_Mouse        = (1 << 3),
  Event_Category_Mouse_Button = (1 << 4)
};
typedef uint16_t EventCategoryFlags;

#define QTZ_EVENT_DEFINE_CONSTS(type, categoryFlags)                                \
static Quartz::EventType GetTypeStatic() { return type; }                           \
virtual Quartz::EventType GetType() const override { return type; }                 \
virtual const char* GetTypeName_Debug() const override { return #type; }            \
virtual EventCategoryFlags GetCategories() const override { return categoryFlags; }

class Event
{
public:
  virtual EventType GetType() const = 0;
  virtual EventCategoryFlags GetCategories() const = 0;
  inline bool HasCategory(EventCategoryBits mask) const { return GetCategories() & mask; };

  virtual const char* GetTypeName_Debug() const = 0;
  virtual std::string ToString_Debug() const { return GetTypeName_Debug(); }

  inline bool GetHandled() const { return m_handled; }
  inline void NotifyHandled() { m_handled = true; }

protected:
  bool m_handled = false;
}; // class Event

// Not available with "SPDLOG_USE_STD_FORMAT" set ON in vendor cmake
//inline std::ostream& operator<<(std::ostream& os, const Event& e)
//{
//  return os << e.ToString_Debug();
//}

} // namespace Quartz

#include "quartz/events/event_window.h"
#include "quartz/events/event_input.h"
