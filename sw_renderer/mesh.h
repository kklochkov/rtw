#pragma once

#include "sw_renderer/color.h"
#include "sw_renderer/tex_coord.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace rtw::sw_renderer
{

using Index = math::Vector3<std::uint32_t>;

struct Face
{
  /// Indices of vertices in the mesh.
  /// The winding order is counter-clockwise.
  Index vertex_indices;

  /// Indices of texture coordinates in the mesh.
  /// The winding order is counter-clockwise.
  std::optional<Index> texture_indices{std::nullopt};

  /// Indices of normals in the mesh.
  /// The winding order is counter-clockwise.
  std::optional<Index> normal_indices{std::nullopt};

  /// Material of the face.
  std::string material;
};

struct Material
{
  std::string name;
  std::string ambient_texture;
  std::string diffuse_texture;
  std::string specular_texture;
  Color ambient{0xFF'FF'FF'FF};
  Color diffuse{0xFF'FF'FF'FF};
  Color specular{0xFF'FF'FF'FF};
};

struct Mesh
{
  Material material(const std::string& name) const
  {
    const auto it = materials.find(name);
    return it != materials.end() ? it->second : Material{};
  }

  Texture texture(const std::string& name) const
  {
    const auto it = textures.find(name);
    return it != textures.end() ? it->second : Texture{};
  }

  std::vector<Point3F> vertices;
  std::vector<TexCoordF> tex_coords;
  std::vector<Vector3F> normals;
  std::vector<Face> faces;
  std::map<std::string, Material> materials;
  std::map<std::string, Texture> textures;
};

} // namespace rtw::sw_renderer
