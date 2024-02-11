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
friend class TextureSkybox;
friend QuartzResult ConvolveHdri();

public:
  Mesh() : m_isValid(false) {}
  Mesh(const char* path);
  Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

  ~Mesh();

  QuartzResult Init(const char* path);
  QuartzResult Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
  QuartzResult InitFromDump(const char* path);
  void Shutdown();

  void Dump(const char* path);

  inline bool IsValid() const { return m_isValid; }

private:
  OpalMesh m_opalMesh;
  bool m_isValid;

  std::vector<uint32_t> m_indices;
  std::vector<Vertex> m_verticies;

  void Render() const;
};

} // namespace Quartz
