#pragma once

#include "quartz/defines.h"
#include "quartz/platform/input/keys.h"

#include <stdint.h>
#include <peridot.h>

namespace Quartz
{
extern const ButtonCode PlatformKeyToQuartzInputCode[256];

class Input
{
public:
  static bool OnButtonPress(ButtonCode key)
  {
    uint32_t index = key >> 6; // button/64
    uint64_t bit = 1ll << (key % 64);

    return (curState.buttons[index] & bit) == bit && (prevState.buttons[index] & bit) == 0;
  }

  static bool OnButtonRelease(ButtonCode key)
  {
    uint32_t index = key >> 6; // button/64
    uint64_t bit = 1ll << (key % 64);

    return (curState.buttons[index] & bit) == 0 && (prevState.buttons[index] & bit) == bit;
  }

  static bool OnButtonChange(ButtonCode key)
  {
    uint32_t index = key >> 6; // button/64
    uint64_t bit = 1ll << (key % 64);

    return (curState.buttons[index] & bit) != (prevState.buttons[index] & bit);
  }

  static bool ButtonStatus(ButtonCode key)
  {
    uint32_t index = key >> 6; // button/64
    uint64_t bit = 1ll << (key % 64);

    return (curState.buttons[index] & bit) == bit;
  }

  static Vec2I GetMousePosition()
  {
    return curState.mousePos;
  }

  static Vec2I GetMouseDelta()
  {
    return Vec2ISubtractVec2I(prevState.mousePos, curState.mousePos);
  }

  static Vec2I GetMouseScroll()
  {
    return curState.mouseScroll;
  }

  void HandlePress(ButtonCode button)
  {
    uint32_t index = button >> 6; // button/64
    uint64_t bit = 1ll << (button % 64);

    curState.buttons[index] |= bit;
  }

  void HandleRelease(ButtonCode button)
  {
    uint32_t index = button >> 6; // button/64
    uint64_t bit = 1ll << (button % 64);

    curState.buttons[index] &= ~bit;
  }

  void HandleMouseMove(Vec2I newPosition)
  {
    curState.mousePos = newPosition;
  }

  void HandleMouseScroll(Vec2I amounts)
  {
    curState.mouseScroll.x += amounts.x;
    curState.mouseScroll.y += amounts.y;
  }

  void HandleMousePress(int buttonIndex)
  {
    HandlePress((ButtonCode)(buttonIndex + Mouse_0));
  }

  void HandleMouseRelease(int buttonIndex)
  {
    HandleRelease((ButtonCode)(buttonIndex + Mouse_0));
  }

  void UpdateState()
  {
    prevState = curState;
    curState.mouseScroll = { 0, 0 };
  }

  void PrintCurButtonState_Debug()
  {
    for (uint32_t i = 0; i < 128; i++)
    {
      uint32_t index = i >> 6; // i/64
      uint64_t bit = 1ll << (i % 64);
      printf("%u", (curState.buttons[index] & bit) != 0);

      if (i > 0 && (i % 8) == 0)
      {
        printf("-");
      }
    }
    printf("\n");
  }

private:
  static struct InputState
  {
    uint64_t buttons[2] = { 0, 0 }; // 128 bit flags
    Vec2I mousePos = { 0, 0 };
    Vec2I mouseScroll = { 0, 0 };
  } curState, prevState;
};

} // namespace Quartz
