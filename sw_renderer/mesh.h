#pragma once

#include "sw_renderer/color.h"
#include "sw_renderer/texture.h"
#include "sw_renderer/types.h"

#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace rtw::sw_renderer
{

/// Index triplet for face vertex/texture/normal indices.
using Index = math::Vector3<std::uint32_t>;

/// A triangular face referencing mesh vertex, texture coordinate, and normal arrays.
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

  /// Material name for this face (key into Mesh::materials).
  std::string material;
};

/// Surface material properties (colors and texture names).
struct Material
{
  std::string name;
  std::string ambient_texture;   ///< Filename for ambient texture map.
  std::string diffuse_texture;   ///< Filename for diffuse texture map.
  std::string specular_texture;  ///< Filename for specular texture map.
  Color ambient{0xFF'FF'FF'FF};  ///< Ambient color (default: white).
  Color diffuse{0xFF'FF'FF'FF};  ///< Diffuse color (default: white).
  Color specular{0xFF'FF'FF'FF}; ///< Specular color (default: white).
};

/// A 3D triangle mesh with materials and textures.
struct Mesh
{
  /// @pre The material must exist (check with has_material() first).
  /// @param[in] name The material name to look up.
  /// @return Reference to the named material.
  const Material& material(const std::string& name) const
  {
    const auto it = materials.find(name);
    assert(it != materials.end());
    return it->second;
  }

  bool has_material(const std::string& name) const { return materials.find(name) != materials.end(); }

  /// @pre The texture must exist (check with has_texture() first).
  /// @param[in] name The texture name to look up.
  /// @return Reference to the named texture.
  const Texture& texture(const std::string& name) const
  {
    const auto it = textures.find(name);
    assert(it != textures.end());
    return it->second;
  }

  bool has_texture(const std::string& name) const { return textures.find(name) != textures.end(); }

  std::vector<Point3F> vertices;             ///< Vertex positions.
  std::vector<TexCoordF> tex_coords;         ///< Texture coordinates.
  std::vector<Vector3F> normals;             ///< Vertex normals.
  std::vector<Face> faces;                   ///< Triangle faces.
  std::map<std::string, Material> materials; ///< Named materials.
  std::map<std::string, Texture> textures;   ///< Named textures (loaded from files).
};

} // namespace rtw::sw_renderer
