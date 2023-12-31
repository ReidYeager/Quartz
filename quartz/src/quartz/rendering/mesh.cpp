#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/mesh.h"

namespace Quartz
{

QuartzResult Mesh::Init(const char* path)
{
  QTZ_ERROR("Loading a mesh from file not yet supported");
  return Quartz_Failure;
}

QuartzResult Mesh::Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  OpalMeshInitInfo meshInfo {};
  meshInfo.vertexCount = vertices.size();
  meshInfo.pVertices = vertices.data();
  meshInfo.indexCount = indices.size();
  meshInfo.pIndices = indices.data();

  QTZ_ATTEMPT_OPAL(OpalMeshInit(&m_opalMesh, meshInfo));
  return Quartz_Success;
}

void Mesh::Shutdown()
{
  OpalMeshShutdown(&m_opalMesh);
}

void Mesh::Render() const
{
  OpalRenderMesh(m_opalMesh);
}

} // namespace Quartz
