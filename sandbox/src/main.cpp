
#include <quartz.h>
#include <stdio.h>

class GameLayer : public Quartz::Layer
{
public:
  GameLayer() : Layer("Game layer") {}

  void OnUpdate() override
  {
    QTZ_INFO("Game layer update");
  }

  void OnEvent(Quartz::Event& e) override
  {
    QTZ_WARNING("Game handling event : {}", e.GetTypeName_Debug());
    e.NotifyHandled();
  }
};

std::vector<Quartz::Renderable> renderables;
Quartz::Material mat;
Quartz::Mesh mesh;

bool Init()
{
  Quartz::PushLayer(new GameLayer());
  


  mat = Quartz::CreateMaterial({
      "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.vert.spv",
      "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.frag.spv"
    });

  float ratio = (float)Quartz::WindowGetWidth() / (float)Quartz::WindowGetHeight();
  Mat4 projMatrix = ProjectionPerspectiveExtended(ratio, 16.0f / 9.0f, 60.0f, 0.1f, 100.0f);

  Transform camTransform = transformIdentity;
  camTransform.position.z = -2.0f;
  camTransform.rotation.y = 180.0f;

  Mat4 camMatrix = Mat4MuliplyMat4(projMatrix, Mat4Invert(TransformToMat4(camTransform)));
  mat.PushData(&camMatrix);

  std::vector<Quartz::Vertex> verts = {
    {.position = {  0.5f,  0.5f, -0.5f } }, // Right, Top,    Back   0
    {.position = {  0.5f, -0.5f, -0.5f } }, // Right, Bottom, Back   1
    {.position = {  0.5f,  0.5f,  0.5f } }, // Right, Top,    Front  2
    {.position = {  0.5f, -0.5f,  0.5f } }, // Right, Bottom, Front  3
    {.position = { -0.5f,  0.5f, -0.5f } }, // Left,  Top,    Back   4
    {.position = { -0.5f, -0.5f, -0.5f } }, // Left,  Bottom, Back   5
    {.position = { -0.5f,  0.5f,  0.5f } }, // Left,  Top,    Front  6
    {.position = { -0.5f, -0.5f,  0.5f } }, // Left,  Bottom, Front  7
  };
  std::vector<uint32_t> indices = {
    0, 4, 6, 0, 6, 2, // Top
    5, 1, 3, 5, 3, 7, // Bottom
    3, 2, 6, 3, 6, 7, // Front
    5, 4, 0, 5, 0, 1, // Back
    7, 6, 4, 7, 4, 5, // Left
    1, 0, 2, 1, 2, 3, // Right
  };
  mesh = Quartz::CreateMesh(verts, indices);

  Quartz::Renderable baseRenderable{};
  baseRenderable.material = mat;
  baseRenderable.mesh = mesh;
  baseRenderable.transform = transformIdentity;
  baseRenderable.transform.position.z = 1.0f;
  baseRenderable.transformMatrix = TransformToMat4(transformIdentity);

  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ 2.0f, 1.0f, 2.0f };
  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ 5.0f, 0.0f, 10.0f };
  renderables.push_back(baseRenderable);
  baseRenderable.transform.position = Vec3{ -2.0f, -4.0f, 6.0f };
  renderables.push_back(baseRenderable);

  return true;
}

float totalTime = 0.0f;
bool Update(float deltaTime)
{
  totalTime += deltaTime;

  for (uint32_t i = 0; i < renderables.size(); i++)
  {
    Quartz::Renderable& r = renderables[i];
    r.transform.rotation.y += deltaTime * 30.0f * (i + 1);
    r.transform.rotation.x += deltaTime * 30.0f * (i + 1);
    r.transform.rotation.z += deltaTime * 30.0f * (i + 1);
    r.transform.position.x = sin(totalTime / (i + 1)) * i * 3;
    r.transformMatrix = TransformToMat4(r.transform);

    Quartz::SubmitForRender(r);
  }

  return true;
}

bool Shutdown()
{
  mesh.Shutdown(); 
  mat.Shutdown();
  return true;
}

int main()
{
  Quartz::Run(Init, Update, Shutdown);

  return 0;
}
