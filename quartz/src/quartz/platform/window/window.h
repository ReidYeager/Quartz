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
  virtual ~Window() {}

  static Window* InitNew(const WindowInitInfo& initInfo = WindowInitInfo());
  virtual QuartzResult Update() = 0;
  virtual void Shutdown() = 0;

  virtual void SetEventCallback(const QuartzEventCallbackFunction& callback) = 0;

  virtual inline bool IsValid() const = 0;
  virtual inline uint32_t Width() const = 0;
  virtual inline uint32_t Height() const = 0;
};

} // namespace Quartz