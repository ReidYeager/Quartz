
#include "quartz/defines.h"
#include "quartz/layers/layer_stack.h"

namespace Quartz
{

void LayerStack::PushLayer(Layer* layer)
{
  m_underlayHead = m_layers.emplace(m_underlayHead, layer);
  layer->OnAttach();
}

void LayerStack::PopLayer(Layer* layer)
{
  auto iterator = std::find(m_layers.begin(), m_layers.end(), layer);
  if (iterator != m_layers.end())
  {
    layer->OnDetach();
    if (m_underlayHead != m_layers.begin())
      m_underlayHead--;
    m_layers.erase(iterator);
  }
}

void LayerStack::PushOverlay(Layer* overlay)
{
  m_layers.emplace_back(overlay);
  overlay->OnAttach();
}

void LayerStack::PopOverlay(Layer* overlay)
{
  auto iterator = std::find(m_layers.begin(), m_layers.end(), overlay);
  if (iterator != m_layers.end())
  {
    overlay->OnDetach();
    m_layers.erase(iterator);
  }
}

} // namespace Quartz
