#include "scene.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace Rain {
bool Object::Init(const std::string& obj_file, const Mat3f& rot,
                  const Vec3f& trans, float scale) {
  transformation_ = Mat4f::Identity();
  transformation_.block<3, 3>(0, 0) = scale * rot;
  transformation_.block<3, 1>(0, 3) = trans;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              obj_file.c_str());
  if (!warn.empty()) spdlog::warn(warn);
  if (!err.empty()) spdlog::error(err);
  if (!ret) return ret;
  if (shapes.size() != 1) {
    spdlog::error("multiple shapes in one model");
    return false;
  }
  tinyobj::mesh_t& mesh = shapes[0].mesh;
  n_vert_ = attrib.vertices.size() / 3;
  vertices_ = new Vec3f[n_vert_];
  for (size_t i = 0; i < n_vert_; ++i) {
    vertices_[i] = Vec3f(attrib.vertices[3 * i], attrib.vertices[3 * i + 1],
                         attrib.vertices[3 * i + 2]);
    vertices_[i] = scale * rot * vertices_[i] + trans;
  }
  if (attrib.normals.size() > 0) {
    assert(attrib.normals.size() == n_vert_);
    normals_ = new Vec3f[n_vert_];
    for (size_t i = 0; i < n_vert_; ++i) {
      normals_[i] = Vec3f(attrib.normals[3 * i], attrib.normals[3 * i + 1],
                          attrib.normals[3 * i + 2]);
      normals_[i] = rot * normals_[i];
    }
  }
  if (attrib.texcoords.size() > 0) {
    n_texc_ = attrib.texcoords.size() / 2;
    texcoords_ = new Vec2f[n_texc_];
    for (size_t i = 0; i < n_texc_; ++i) {
      texcoords_[i] =
          Vec2f(attrib.texcoords[2 * i], attrib.texcoords[2 * i + 1]);
    }
  }
  n_elevert_ = mesh.num_face_vertices[0];
  n_ele_ = mesh.indices.size() / n_elevert_;
  indices_ = new uint32_t[mesh.indices.size()];
  size_t index_offset = 0;
  for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
    assert(mesh.num_face_vertices[f] == n_elevert_);
    for (size_t v = 0; v < n_elevert_; ++v) {
      tinyobj::index_t& idx = mesh.indices[index_offset + v];
      indices_[index_offset + v] = idx.vertex_index;
    }
    index_offset += n_elevert_;
  }
  if (n_elevert_ == 3) {
    n_surfidx_ = n_ele_ * 3;
    n_face_ = n_ele_;
    surface_indices_ = indices_;
  } else if (n_elevert_ == 4) {
    // TODO: build surface indices from tetmesh
  }

  if (attrib.normals.size() == 0) {
    // compute surface normal
    normals_ = new Vec3f[n_vert_];
    memset(normals_, 0, n_vert_ * sizeof(Vec3f));
    for (size_t f = 0; f < n_face_; ++f) {
      uint32_t v1 = surface_indices_[3 * f];
      uint32_t v2 = surface_indices_[3 * f + 1];
      uint32_t v3 = surface_indices_[3 * f + 2];
      Vec3f dx1 = vertices_[v2] - vertices_[v1];
      Vec3f dx2 = vertices_[v3] - vertices_[v1];
      Vec3f normal = (dx1.cross(dx2)).normalized();
      normals_[v1] += normal;
      normals_[v2] += normal;
      normals_[v3] += normal;
    }
    for (size_t i = 0; i < n_vert_; ++i) {
      normals_[i] = normals_[i].normalized();
    }
  }

  return true;
}

void Object::Destroy() {
  if (vertices_) delete[] vertices_;
  if (normals_) delete[] normals_;
  if (texcoords_) delete[] texcoords_;
  if (indices_) delete[] indices_;
  if ((n_elevert_ == 4) && surface_indices_) delete[] surface_indices_;
}

void Scene::Init() {
  Object bunny;
  bunny.Init("../assets/bunny/bunny.obj", Mat3f::Identity(), Vec3f::Zero(),
             5.0f);
  objects_.push_back(bunny);
}

void Scene::Destroy() {
  for (Object obj : objects_) {
    obj.Destroy();
  }
}
};  // namespace Rain