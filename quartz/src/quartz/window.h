#pragma once

#include "quartz/core.h"
#include "quartz/events/event_all.h"

namespace Quartz
{

struct WindowInitInfo
{
  std::string title;
  uint32_t width, height;
  uint32_t xPos, yPos;

  WindowInitInfo(
    const std::string& title = "Quartz window",
    uint32_t width = 1280,
    uint32_t height = 720,
    uint32_t xPos = 0,
    uint32_t yPos = 0)
    : title(title), width(width), height(height), xPos(xPos), yPos(yPos) {}
};

class Window
{
public:
  using fnEventCallback = std::function<void(Event&)>;

  virtual ~Window() {}

  virtual bool GetIsValid() = 0;

  virtual QuartzResult Update() = 0;
  virtual void Close() = 0;

  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  virtual void SetEventCallback(const fnEventCallback& callback) = 0;

  static Window* Create(const WindowInitInfo& initInfo = WindowInitInfo());
};

} // namespace Quartz