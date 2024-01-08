
#include <quartz.h>
#include <stdio.h>

#include <diamond.h>

class GameLayer : public Quartz::Layer
{
public:
  GameLayer() : Layer("Game layer") {}

  void OnAttach() override
  {
    // Resources
    // ============================================================

    m_mat = Quartz::CreateMaterial({
      "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.vert.spv",
      "D:/Dev/Quartz/quartz/res/shaders/compiled/blank.frag.spv"
      });

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
    m_mesh = Quartz::CreateMesh(verts, indices);

    // Camera
    // ============================================================

    float ratio = (float)Quartz::WindowGetWidth() / (float)Quartz::WindowGetHeight();
    Mat4 projMatrix = ProjectionPerspectiveExtended(ratio, 16.0f / 9.0f, 60.0f, 0.1f, 100.0f);
    camTransform.position.z = -2.0f;
    camTransform.rotation.y = 180.0f;
    camMatrix = Mat4MuliplyMat4(projMatrix, Mat4Invert(TransformToMat4(camTransform)));
    m_mat.PushData(&camMatrix);

    // Entities
    // ============================================================

    Quartz::Renderable baseRenderable{};
    baseRenderable.material = m_mat;
    baseRenderable.mesh = m_mesh;
    baseRenderable.transformMatrix = TransformToMat4(transformIdentity);

    Quartz::ComponentId renderableId = QuartzComponentId(Quartz::Renderable);
    Quartz::ComponentId transformId = QuartzComponentId(Transform);

    Vec3 positions[4] = {
      { 0.0f, 0.0f, 1.0f },
      { 2.0f, 1.0f, 2.0f },
      { 5.0f, 0.0f, 10.0f },
      { -2.0f, -4.0f, 6.0f }
    };

    m_entities.resize(4);
    for (uint32_t i = 0; i < 4; i++)
    {
      Transform* t = (Transform*)m_entities[i].Get(transformId);
      *t = transformIdentity;
      t->position = positions[i];

      Quartz::Renderable* r = (Quartz::Renderable*)m_entities[i].Get(renderableId);
      r->material = m_mat;
      r->mesh = m_mesh;
    }
  }

  void OnUpdate() override
  {
    if (Quartz::Input::OnButtonPress(Quartz::Key_Escape))
    {
      Quartz::Quit();
    }

    Quartz::ComponentId transformId = QuartzComponentId(Transform);

    Quartz::ObjectIterator iter({ transformId });
    if (iter.AtEnd())
      return;

    Transform* frontObject = (Transform*)iter.GetComponentValue(transformId);
    if (Quartz::Input::ButtonStatus(Quartz::Mouse_Left))
    {
      Vec2I mouseDelta = Quartz::Input::GetMouseDelta();
      frontObject->rotation.y -= mouseDelta.x;
      frontObject->rotation.x -= mouseDelta.y;
      frontObject->rotation.z += Quartz::Input::GetMouseScroll().y * 6.0f;
    }
    iter.NextElement();

    uint32_t i = 1;
    while(!iter.AtEnd())
    {
      Transform* r = (Transform*)iter.GetComponentValue(transformId);
      r->rotation.y += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->rotation.x += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->rotation.z += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->position.x = sin(Quartz::time.totalTimeDeltaSum / (i + 1)) * i * 3;
      i++;
      iter.NextElement();
    }
  }

  void OnDetach() override
  {
    m_mesh.Shutdown();
    m_mat.Shutdown();
  }

  void OnEvent(Quartz::Event& e) override
  {
    //QTZ_WARNING("Game handling event : {}", e.GetTypeName_Debug());
    e.NotifyHandled();

    if (e.GetType() == Quartz::Event_Window_Resize)
    {
      float ratio = (float)Quartz::WindowGetWidth() / (float)Quartz::WindowGetHeight();
      Mat4 projMatrix = ProjectionPerspectiveExtended(ratio, 16.0f / 9.0f, 60.0f, 0.1f, 100.0f);
      camMatrix = Mat4MuliplyMat4(projMatrix, Mat4Invert(TransformToMat4(camTransform)));
      m_mat.PushData(&camMatrix);
    }
  }

private:
  Transform camTransform = transformIdentity;
  Mat4 camMatrix = mat4Identity;

  std::vector<Quartz::Renderable*> renderables;
  std::vector<Quartz::Entity> m_entities;
  Quartz::Material m_mat {};
  Quartz::Mesh m_mesh {};
};

QUARTZ_GAME_LAYER(GameLayer)
QUARTZ_ENTRY_POINT
