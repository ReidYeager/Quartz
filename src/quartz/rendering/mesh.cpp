
#include "quartz/defines.h"
#include "quartz/rendering/defines.h"
#include "quartz/rendering/mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>
#include <stdio.h>

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
    Vec3I* iNor = (Vec3I*)&vertex.normal;
    Vec3I* iTan = (Vec3I*)&vertex.tangent;
    Vec2I* iUv = (Vec2I*)&vertex.uv;

    size_t posHash = 0, normHash = 0, tanHash = 0, uvHash = 0;

    posHash ^= HashInt(iPos->x);
    posHash ^= HashInt(iPos->y);
    posHash ^= HashInt(iPos->z);

    normHash ^= HashInt(iNor->x);
    normHash ^= HashInt(iNor->y);
    normHash ^= HashInt(iNor->z);

    tanHash ^= HashInt(iTan->x);
    tanHash ^= HashInt(iTan->y);
    tanHash ^= HashInt(iTan->z);

    uvHash ^= HashInt(iUv->x);
    uvHash ^= HashInt(iUv->y);

    return posHash ^ normHash ^ uvHash;
  }
};
}

namespace Quartz
{

Mesh::Mesh(const char* path) : m_isValid(false)
{
  QTZ_ATTEMPT_VOID(Init(path));
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : m_isValid(false)
{
  QTZ_ATTEMPT_VOID(Init(vertices, indices));
}

Mesh::~Mesh()
{
  if (m_isValid)
  {
    QTZ_ERROR("Mesh must be shut down manually");
  }
}

QuartzResult Mesh::Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid mesh");
    return Quartz_Success;
  }

  OpalMeshInitInfo meshInfo {};
  meshInfo.vertexCount = vertices.size();
  meshInfo.pVertices = vertices.data();
  meshInfo.indexCount = indices.size();
  meshInfo.pIndices = indices.data();

  QTZ_ATTEMPT_OPAL(OpalMeshInit(&m_opalMesh, meshInfo));

  m_verticies = std::vector<Vertex>(vertices);
  m_indices = std::vector<uint32_t>(indices);

  m_isValid = true;
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
  if (m_isValid)
  {
    QTZ_WARNING("Attempting to initialize a valid mesh");
    return Quartz_Success;
  }

  // TODO : Replace with custom model loader for better control
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

      vert.uv = Vec2{
        attrib.texcoords[2 * indexSet.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * indexSet.texcoord_index + 1],
      };

      vert.normal = Vec3{
        attrib.normals[3 * indexSet.normal_index + 0],
        attrib.normals[3 * indexSet.normal_index + 1],
        attrib.normals[3 * indexSet.normal_index + 2]
      };

      vert.tangent = Vec3{ 0.0f, 0.0f, 0.0f };

      if (vertMap.count(vert) == 0)
      {
        vertMap[vert] = (uint32_t)verticies.size();
        verticies.push_back(vert);
      }
      indices.push_back(vertMap[vert]);
    }
  }

  std::vector<uint32_t> vertexTangentCounts(verticies.size());
  for (uint32_t i = 0; i < indices.size(); i += 3)
  {
    Vertex* vert1 = &verticies[indices[i + 0]];
    Vertex* vert2 = &verticies[indices[i + 1]];
    Vertex* vert3 = &verticies[indices[i + 2]];

    Vec3 e1 = vert2->position - vert1->position;
    Vec3 e2 = vert3->position - vert1->position;

    Vec2 uv1 = vert2->uv - vert1->uv;
    Vec2 uv2 = vert3->uv - vert1->uv;

    float r = 1.0f / (uv1.x * uv2.y - uv2.x * uv1.y);

    Vec3 tangent = Vec3{ (uv2.y * e1.x - uv1.y * e2.x) * r, (uv2.y * e1.y - uv1.y * e2.y) * r, (uv2.y * e1.z - uv1.y * e2.z) * r };

    if (tangent.x != tangent.x || tangent.y != tangent.y || tangent.z != tangent.z)
    {
      // Fix divide by zero issues (If all UV's share an axis)
      tangent = Vec3{0.0f, 0.0f, 0.0f};
    }

    vert1->tangent = vert1->tangent + tangent;
    vert2->tangent = vert2->tangent + tangent;
    vert3->tangent = vert3->tangent + tangent;
  }

  for (uint32_t i = 0; i < verticies.size(); i++)
  {
    Vertex* v = &verticies[i];

    if (v->tangent == Vec3{ 0.0f, 0.0f, 0.0f })
    {
      // TODO : Find a proper fix for invalid tangents
      v->tangent = v->normal;
    }

    // Calculate orthogonal tangent
    v->tangent = (v->tangent - (v->normal * Dot(v->normal, v->tangent))).Normal();
  }

  QTZ_ATTEMPT(Init(verticies, indices));

  m_isValid = true;
  return Quartz_Success;
}

QuartzResult Mesh::InitFromDump(const char* path)
{
  FILE* inFile;
  int err = fopen_s(&inFile, path, "rb");
  if (err)
  {
    QTZ_ERROR("Failed to open a mesh dump file (\"%s\")", path);
    return Quartz_Failure;
  }

  std::vector<Vertex> verts;
  std::vector<uint32_t> indices;
  size_t vertCount;
  size_t indexCount;

  fread(&vertCount, sizeof(size_t), 1, inFile);
  verts.resize(vertCount);
  fread(verts.data(), sizeof(Vertex), vertCount, inFile);
  fread(&indexCount, sizeof(size_t), 1, inFile);
  indices.resize(indexCount);
  fread(indices.data(), sizeof(uint32_t), indexCount, inFile);

  fclose(inFile);

  QTZ_ATTEMPT(Init(verts, indices));

  return Quartz_Success;
}

void Mesh::Shutdown()
{
  if (!m_isValid)
  {
    return;
  }

  m_isValid = false;
  OpalMeshShutdown(&m_opalMesh);
}

void Mesh::Dump(const char* path)
{
  FILE* outFile;
  int err = fopen_s(&outFile, path, "wb");
  if (err)
  {
    QTZ_ERROR("Failed to open a file to dump the mesh (\"%s\")", path);
    return;
  }

  size_t vertCount = m_verticies.size();
  size_t indexCount = m_indices.size();

  fwrite(&vertCount, sizeof(size_t), 1, outFile);
  fwrite(m_verticies.data(), sizeof(Vertex), m_verticies.size(), outFile);
  fwrite(&indexCount, sizeof(size_t), 1, outFile);
  fwrite(m_indices.data(), sizeof(uint32_t), m_indices.size(), outFile);

  fclose(outFile);
}

void Mesh::Render() const
{
  if (!m_isValid)
  {
    QTZ_ERROR("Attempting to render invalid mesh");
    return;
  }

  OpalRenderMesh(m_opalMesh);
}

} // namespace Quartz
