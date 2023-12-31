
#include "quartz/defines.h"
#include "quartz.h"

#include "quartz/platform/platform.h"
#include "quartz/rendering/rendering.h"

#include <chrono>
#include <math.h>

namespace Quartz
{
uint32_t g_quartzAttemptDepth = 0;

void Run()
{
  // ============================================================
  // Init
  // ============================================================

  Logger::Init();

  // Create window
  Window* window = CreateWindow();

  // Init rendering api
  Renderer renderer;
  renderer.Init(window);

  Material mat = renderer.CreateMaterial({
    "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.vert.spv",
    "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.frag.spv"
    });

  float ratio = (float)window->Width() / (float)window->Height();
  Mat4 projMatrix = ProjectionPerspectiveExtended(ratio, 16.0f / 9.0f, 60.0f, 0.1f, 100.0f);

  Transform camTransform = transformIdentity;
  camTransform.position.z = -2.0f;
  camTransform.rotation.y = 180.0f;

  Mat4 camMatrix = Mat4MuliplyMat4(projMatrix, Mat4Invert(TransformToMat4(camTransform)));
  mat.PushData(&camMatrix);

  std::vector<Vertex> verts = {
    { .position = {  0.5f,  0.5f, -0.5f } }, // Right, Top,    Back   0
    { .position = {  0.5f, -0.5f, -0.5f } }, // Right, Bottom, Back   1
    { .position = {  0.5f,  0.5f,  0.5f } }, // Right, Top,    Front  2
    { .position = {  0.5f, -0.5f,  0.5f } }, // Right, Bottom, Front  3
    { .position = { -0.5f,  0.5f, -0.5f } }, // Left,  Top,    Back   4
    { .position = { -0.5f, -0.5f, -0.5f } }, // Left,  Bottom, Back   5
    { .position = { -0.5f,  0.5f,  0.5f } }, // Left,  Top,    Front  6
    { .position = { -0.5f, -0.5f,  0.5f } }, // Left,  Bottom, Front  7
  };
  std::vector<uint32_t> indices = {
    0, 4, 6, 0, 6, 2, // Top
    5, 1, 3, 5, 3, 7, // Bottom
    3, 2, 6, 3, 6, 7, // Front
    5, 4, 0, 5, 0, 1, // Back
    7, 6, 4, 7, 4, 5, // Left
    1, 0, 2, 1, 2, 3, // Right
  };
  Mesh mesh = renderer.CreateMesh(verts, indices);

  Renderable baseRenderable {};
  baseRenderable.material = mat;
  baseRenderable.mesh = mesh;
  baseRenderable.transform = transformIdentity;
  baseRenderable.transform.position.z = 1.0f;
  baseRenderable.transformMatrix = TransformToMat4(transformIdentity);

  std::vector<Renderable> renderables;
  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ 2.0f, 1.0f, 2.0f };
  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ 5.0f, 0.0f, 10.0f };
  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ -2.0f, -4.0f, 6.0f };
  renderables.push_back(baseRenderable);

  // ============================================================
  // Update
  // ============================================================

  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  double totalTime = 0.0f;

  while (!window->ShouldClose())
  {
    end = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001f;
    start = end;
    totalTime += deltaTime;
    if (deltaTime >= 1.0f)
      deltaTime = 0.016f; // Fake 60fps for stepping through

    for (uint32_t i = 0; i < renderables.size(); i++)
    {
      Renderable& r = renderables[i];
      r.transform.rotation.y += deltaTime * 30.0f * (i + 1);
      r.transform.rotation.x += deltaTime * 30.0f * (i + 1);
      r.transform.rotation.z += deltaTime * 30.0f * (i + 1);
      r.transform.position.x = sin(totalTime / (i + 1)) * i * 3;
      r.transformMatrix = TransformToMat4(r.transform);
    }

    window->PollEvents();
    renderer.Render(deltaTime, renderables);
  }

  // ============================================================
  // Shutdown
  // ============================================================

  mat.Shutdown();
  mesh.Shutdown();

  renderer.Shutdown();
  window->Shutdown();
}

} // namespace Quartz
