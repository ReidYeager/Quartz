#pragma once

#include "quartz/defines.h"
#include "quartz/layers/layer.h"

#include <vector>

namespace Quartz
{

class LayerStack
{
public:
  LayerStack() { m_underlayHead = m_layers.begin(); }
  ~LayerStack() {}

  void PushLayer(Layer* layer);
  void PopLayer(Layer* layer);
  void PushOverlay(Layer* overlay);
  void PopOverlay(Layer* overlay);

  inline std::vector<Layer*>::iterator BeginIterator() { return m_layers.begin(); }
  inline std::vector<Layer*>::iterator EndIterator() { return m_layers.end(); }

private:
  std::vector<Layer*> m_layers;
  std::vector<Layer*>::iterator m_underlayHead;
};

} // namespace Quartz
