
#pragma once

#include "quartz/defines.h"
#include "quartz/logging/logger.h"

#include "quartz/rendering/defines.h"
#include "quartz/rendering/material.h"
#include "quartz/rendering/mesh.h"
#include "quartz/rendering/rendering.h"
#include "quartz/layers/layer.h"

#include <vector>

namespace Quartz {

void Run(bool(*GameInit)(), bool(*GameUpdate)(float deltaTime), bool(*GameShutdown)());

// ============================================================
// Window
// ============================================================

uint32_t WindowGetWidth();
uint32_t WindowGetHeight();

// ============================================================
// Rendering
// ============================================================

Material CreateMaterial(const std::vector<const char*>& shaderPaths);
//Mesh CreateMesh(const char* path);
Mesh CreateMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
void SubmitForRender(Renderable& renderable);

// ============================================================
// Layers
// ============================================================

void PushLayer(Layer* layer);

} // namespace Quartz
