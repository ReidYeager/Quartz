#pragma once

#include "quartz/defines.h"
#include "quartz/rendering/defines.h"

#include <opal.h>

#include <vector>

namespace Quartz
{

class Mesh
{
friend class Renderer;

public:
  Mesh() : m_isValid(false) {}
  Mesh(const char* path);
  Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

  ~Mesh();

  QuartzResult Init(const char* path);
  QuartzResult Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
  void Shutdown();

  inline bool IsValid() const { return m_isValid; }

private:
  OpalMesh m_opalMesh;
  bool m_isValid;

  void Render() const;
};

} // namespace Quartz
