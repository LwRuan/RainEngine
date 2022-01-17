#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "mathtype.h"

namespace Rain {
struct Material {
  Vec3f Ka_ = Vec3f(0.2f, 0.2f, 0.2f);  // ambient color
  Vec3f Kd_ = Vec3f(0.8f, 0.8f, 0.8f);  // diffuse color
  Vec3f Ks_ = Vec3f(1.0f, 1.0f, 1.0f);  // specular color
  float d_ = 1.0f;                      // non-transparency
  float Ns_ = 0.0f;                     // shininess
};

class Object {
 public:
  uint64_t n_vert_ = 0;
  Vec3f* vertices_ = nullptr;
  Vec3f* normals_ = nullptr;
  uint64_t n_texc_ = 0;
  Vec2f* texcoords_ = nullptr;
  uint64_t n_ele_ = 0;
  uint32_t n_elevert_ = 3;  // 3 for triangle, 4 for tet
  uint32_t* indices_ = nullptr;
  uint64_t n_surfidx_;
  uint64_t n_face_;
  uint32_t* surface_indices_ = nullptr;
  Material material_;
  Mat4f transformation_;

  bool Init(const std::string& obj_file, const Mat3f& rot, const Vec3f& trans,
            float scale);
  void Destroy();
};

class Scene {
 public:
  std::vector<Object> objects_;
  void Init();
  void Destroy();
};
};  // namespace Rain