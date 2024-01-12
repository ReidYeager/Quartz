
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

size_t HashInt(int32_t value)
{
  value += (0x9e3779b9 * value) >> 3;
  return value;
}

// Used to map vertices into an unordered array during mesh building
namespace std {
template<> struct hash<Quartz::Vertex>
{
  size_t operator()(Quartz::Vertex const& vertex) const
  {
    // Keep all bits, don't round
    Vec3I* iPos = (Vec3I*)&vertex.position;
    //Vec3I* iNor = (Vec3I*)&vertex.normal;
    //Vec2I* iUv = (Vec2I*)&vertex.uv;

    size_t posHash = 0, normHash = 0, uvHash = 0;

    posHash ^= HashInt(iPos->x);
    posHash ^= HashInt(iPos->y);
    posHash ^= HashInt(iPos->z);

    //normHash ^= HashInt(iNor->x);
    //normHash ^= HashInt(iNor->y);
    //normHash ^= HashInt(iNor->z);

    //uvHash ^= HashInt(iUv->x);
    //uvHash ^= HashInt(iUv->y);

    return posHash ^ normHash ^ uvHash;
  }
};
}

namespace Quartz
{

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

QuartzResult LoadTinyObjMesh(const char* path, tinyobj::attrib_t* attrib, std::vector<tinyobj::shape_t>* shapes)
{
  std::string loadWarnings, loadErrors;
  std::vector<tinyobj::material_t> materials;

  if (!tinyobj::LoadObj(attrib, shapes, &materials, &loadWarnings, &loadErrors, path))
  {
    QTZ_ERROR("Failed to load mesh\"{}\"\n\tTinyobj warning : {}\n\tTinyobj error : {}", path, loadWarnings.c_str(), loadErrors.c_str());
    return Quartz_Failure_Vendor;
  }

  return Quartz_Success;
}

QuartzResult Mesh::Init(const char* path)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  QTZ_ATTEMPT(LoadTinyObjMesh(path, &attrib, &shapes));

  // Assemble mesh =====
  std::unordered_map<Vertex, uint32_t> vertMap = {};
  Vertex vert{};
  std::vector<uint32_t> indices;
  std::vector<Vertex> verticies;
  for (const auto& shape : shapes)
  {
    for (const auto& indexSet : shape.mesh.indices)
    {
      vert.position = Vec3{
        attrib.vertices[3 * indexSet.vertex_index + 0],
        attrib.vertices[3 * indexSet.vertex_index + 1],
        attrib.vertices[3 * indexSet.vertex_index + 2]
      };

      //vert.uv = Vec2{
      //  attrib.texcoords[2 * indexSet.texcoord_index + 0],
      //  1.0f - attrib.texcoords[2 * indexSet.texcoord_index + 1],
      //};

      //vert.normal = Vec3{
      //  attrib.normals[3 * indexSet.normal_index + 0],
      //  attrib.normals[3 * indexSet.normal_index + 1],
      //  attrib.normals[3 * indexSet.normal_index + 2]
      //};

      if (vertMap.count(vert) == 0)
      {
        vertMap[vert] = (uint32_t)verticies.size();
        verticies.push_back(vert);
      }
      indices.push_back(vertMap[vert]);
    }
  }

  Init(verticies, indices);

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
