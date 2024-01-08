
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
    Quartz::Camera* camStruct = m_camera.Add<Quartz::Camera>();
    Transform* camTransform = m_camera.Get<Transform>();
    *camTransform = transformIdentity;
    camTransform->position.z = -1.0f;
    camTransform->rotation.y = 180.0f;

    camStruct->desiredRatio = 16.0f / 9.0f;
    camStruct->farClip = 100.0f;
    camStruct->nearClip = 0.1f;
    camStruct->fov = 65.0f;

    float screenRatio = (float)Quartz::WindowGetWidth() / (float)Quartz::WindowGetHeight();
    camStruct->projectionMatrix = ProjectionPerspectiveExtended(
      screenRatio,
      camStruct->desiredRatio,
      camStruct->fov,
      camStruct->nearClip,
      camStruct->farClip);

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
      Transform* t = m_entities[i].Get<Transform>();
      *t = transformIdentity;
      t->position = positions[i];

      Quartz::Renderable* r = m_entities[i].Add<Quartz::Renderable>();
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

    if (Quartz::Input::ButtonStatus(Quartz::Mouse_Right))
    {
      Transform* t = m_camera.Get<Transform>();
      Vec2I mouseMove = Quartz::Input::GetMouseDelta();
      t->rotation.x += mouseMove.y;
      t->rotation.y += mouseMove.x;

      Quaternion rotQ = QuaternionFromEuler(t->rotation);
      Vec3 forward = QuaternionMultiplyVec3(rotQ, Vec3{0.0f, 0.0f, -1.0f});
      Vec3 right = QuaternionMultiplyVec3(rotQ, Vec3{1.0f, 0.0f, 0.0f});

      forward = Vec3MultiplyFloat(forward, Quartz::Input::ButtonStatus(Quartz::Key_W) - Quartz::Input::ButtonStatus(Quartz::Key_S));
      right = Vec3MultiplyFloat(right, Quartz::Input::ButtonStatus(Quartz::Key_D) - Quartz::Input::ButtonStatus(Quartz::Key_A));
      Vec3 up = Vec3MultiplyFloat(Vec3{0.0f, 1.0f, 0.0f}, Quartz::Input::ButtonStatus(Quartz::Key_E) - Quartz::Input::ButtonStatus(Quartz::Key_Q));

      Vec3 movement = Vec3Normalize(Vec3AddVec3(Vec3AddVec3(forward, right), up));
      movement = Vec3MultiplyFloat(movement, Quartz::time.deltaTime * (1 + (3 * Quartz::Input::ButtonStatus(Quartz::Key_Shift_L))));

      t->position = Vec3AddVec3(movement, t->position);
      //QTZ_INFO("Cam pos ({}, {}, {})", t->position.x, t->position.y, t->position.z);
    }

    for (uint32_t i = 0; i < 4; i++)
    {
      Transform* r = m_entities[i].Get<Transform>();
      r->rotation.y += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->rotation.x += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->rotation.z += Quartz::time.deltaTime * 30.0f * (i + 1);
      r->position.x = sin(Quartz::time.totalTimeDeltaSum / (i + 1)) * i * 3;
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
      Quartz::Camera* camStruct = m_camera.Get<Quartz::Camera>();
      Transform* camTransform = m_camera.Get<Transform>();

      float screenRatio = (float)Quartz::WindowGetWidth() / (float)Quartz::WindowGetHeight();
      camStruct->projectionMatrix = ProjectionPerspectiveExtended(
        screenRatio,
        camStruct->desiredRatio,
        camStruct->fov,
        camStruct->nearClip,
        camStruct->farClip);
    }
  }

private:
  std::vector<Quartz::Renderable*> renderables;
  Quartz::Entity m_camera;
  std::vector<Quartz::Entity> m_entities;
  Quartz::Material m_mat {};
  Quartz::Mesh m_mesh {};
};

QUARTZ_GAME_LAYER(GameLayer)
QUARTZ_ENTRY_POINT
