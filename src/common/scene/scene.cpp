#include "scene.h"

namespace Rain {
void Object::Init() {}

void Object::Destroy() {
  if (vertices_) delete[] vertices_;
  if (normals_) delete[] normals_;
  if (texcoords_) delete[] texcoords_;
  if (indices_) delete[] indices_;
  if ((n_elevert_ == 4) && surface_indices_) delete[] surface_indices_;
}

void Scene::Init() {
  Object test_triangle;
  test_triangle.n_vert_ = 4;
  test_triangle.vertices_ = new Vec3f[4]{{-0.5f, 0.0f, -0.5f},
                                         {-0.5f, 0.0f, 0.5f},
                                         {0.5f, 0.0f, 0.5f},
                                         {0.5f, 0.0f, -0.5f}};
  test_triangle.normals_ = new Vec3f[4]{{0.0f, 1.0f, 0.0f},
                                        {0.0f, 1.0f, 0.0f},
                                        {0.0f, 1.0f, 0.0f},
                                        {0.0f, 1.0f, 0.0f}};
  test_triangle.n_ele_ = 2;
  test_triangle.n_elevert_ = 3;
  test_triangle.indices_ = new uint32_t[6]{0, 1, 2, 2, 3, 0};
  test_triangle.n_surfidx_ = 6;
  test_triangle.surface_indices_ = test_triangle.indices_;
  test_triangle.transformation_ = Mat4f::Identity();
  objects_.push_back(test_triangle);
}

void Scene::Destroy() {
  for (Object obj : objects_) {
    obj.Destroy();
  }
}
};  // namespace Rain