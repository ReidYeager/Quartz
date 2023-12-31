#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>
#include <peridot.h>

#include <vector>

namespace Quartz
{

class Mesh
{
friend class Renderer;

public:
  void Shutdown();

private:
  OpalMesh m_opalMesh;

  QuartzResult Init(const char* path);
  QuartzResult Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

  void Render() const;
};

} // namespace Quartz
