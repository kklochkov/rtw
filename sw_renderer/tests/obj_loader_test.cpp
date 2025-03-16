#include "sw_renderer/obj_loader.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{

rtw::sw_renderer::Face make_face(std::initializer_list<std::uint32_t> vertex_indices,
                                 std::initializer_list<std::uint32_t> texture_indices,
                                 std::initializer_list<std::uint32_t> normal_indices, const std::string& material = {})
{
  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  rtw::sw_renderer::Face face;
  auto index =
      rtw::sw_renderer::Index{vertex_indices.begin()[0U], vertex_indices.begin()[1U], vertex_indices.begin()[2U]}
      - rtw::sw_renderer::Index{1U, 1U, 1U};
  face.vertex_indices = index;
  if (!std::empty(texture_indices))
  {
    index =
        rtw::sw_renderer::Index{texture_indices.begin()[0U], texture_indices.begin()[1U], texture_indices.begin()[2U]}
        - rtw::sw_renderer::Index{1U, 1U, 1U};
    face.texture_indices = std::make_optional<rtw::sw_renderer::Index>(index);
  }
  if (!std::empty(normal_indices))
  {
    index = rtw::sw_renderer::Index{normal_indices.begin()[0U], normal_indices.begin()[1U], normal_indices.begin()[2U]}
          - rtw::sw_renderer::Index{1U, 1U, 1U};
    face.normal_indices = std::make_optional<rtw::sw_renderer::Index>(index);
  }
  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  face.material = material;
  return face;
}

} // namespace

TEST(ObjLoader, load_obj_from_empty_stream)
{
  std::istringstream stream;
  const auto result = rtw::sw_renderer::load_obj(stream);
  EXPECT_TRUE(result.mesh.vertices.empty());
  EXPECT_TRUE(result.mesh.faces.empty());
  EXPECT_TRUE(result.mesh.materials.empty());
}

TEST(ObjLoader, load_obj_from_stream)
{
  // Data from https://en.m.wikipedia.org/wiki/Wavefront_.obj_file#File_format
  std::istringstream stream(R"(
# List of geometric vertices, with (x, y, z, [w]) coordinates, w is optional and defaults to 1.0.
v 0.123 0.234 0.345 1.0
# List of texture coordinates, in (u, [v, w]) coordinates, these will vary between 0 and 1. v, w are optional and default to 0.
vt 0.500 1 [0]
# List of vertex normals in (x,y,z) form; normals might not be unit vectors.
vn 0.707 0.000 0.707
# Polygonal face element (see below)
f 1 2 3
f 3/1 4/2 5/3
f 6/4/1 3/5/3 7/6/5
f 7//1 8//2 9//3
)");

  const auto result = rtw::sw_renderer::load_obj(stream);
  const auto& mesh = result.mesh;
  ASSERT_EQ(mesh.vertices.size(), 1U);
  ASSERT_EQ(mesh.faces.size(), 4U);
  ASSERT_EQ(mesh.materials.size(), 0U);

  const rtw::math::Point3F expected_vertices[] = {
      rtw::math::Point3F{0.123F, 0.234F, 0.345F},
  };
  EXPECT_THAT(mesh.vertices, testing::ElementsAreArray(expected_vertices));

  const rtw::sw_renderer::Face expected_faces[] = {
      make_face({1U, 2U, 3U}, {}, {}),
      make_face({3U, 4U, 5U}, {1U, 2U, 3U}, {}),
      make_face({6U, 3U, 7U}, {4U, 5U, 6U}, {1U, 3U, 5U}),
      make_face({7U, 8U, 9U}, {}, {1U, 2U, 3U}),
  };
  for (std::size_t i = 0U; i < mesh.faces.size(); ++i)
  {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    ASSERT_EQ(mesh.faces[i].vertex_indices, expected_faces[i].vertex_indices);
    ASSERT_EQ(mesh.faces[i].texture_indices, expected_faces[i].texture_indices);
    ASSERT_EQ(mesh.faces[i].normal_indices, expected_faces[i].normal_indices);
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
  }
}

TEST(ObjLoader, load_mtl_from_empty_stream)
{
  std::istringstream stream;
  rtw::sw_renderer::Mesh mesh;
  rtw::sw_renderer::load_mtl(stream, mesh);
  ASSERT_TRUE(mesh.materials.empty());
}

TEST(ObjLoader, load_mtl_from_stream)
{
  // Data from https://en.m.wikipedia.org/wiki/Wavefront_.obj_file#Texture_maps
  std::istringstream stream(R"(
newmtl Textured
Ka 0.200 0.400 0.800
Kd 1.000 0.400 0.200
Ks 0.200 0.400 0.800
d 1.0
illum 2
map_Ka ambient.png
map_Kd diffuse.png
map_Ks specular.png)");

  const rtw::sw_renderer::Material expected_material = {"Textured",
                                                        "ambient.png",
                                                        "diffuse.png",
                                                        "specular.png",
                                                        {0.2F, 0.4F, 0.8F, 1.0F},
                                                        {1.0F, 0.4F, 0.2F, 1.0F},
                                                        {0.2F, 0.4F, 0.8F, 1.0F}};

  rtw::sw_renderer::Mesh mesh;
  rtw::sw_renderer::load_mtl(stream, mesh);

  ASSERT_EQ(mesh.materials.size(), 1U);
  ASSERT_EQ(mesh.materials.count("Textured"), 1U);
  ASSERT_EQ(mesh.textures.size(), 3U);

  const auto& material = mesh.materials.at("Textured");
  ASSERT_EQ(material.name, expected_material.name);
  ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
  ASSERT_EQ(material.diffuse.rgba, expected_material.diffuse.rgba);
  ASSERT_EQ(material.specular.rgba, expected_material.specular.rgba);
  ASSERT_EQ(material.ambient_texture, expected_material.ambient_texture);
  ASSERT_EQ(material.diffuse_texture, expected_material.diffuse_texture);
  ASSERT_EQ(material.specular_texture, expected_material.specular_texture);
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST(ObjLoader, load_obj_from_file)
{
  const auto mesh = rtw::sw_renderer::load_obj("sw_renderer/resources/cube.obj");
  ASSERT_TRUE(mesh.has_value());

  if (!mesh.has_value())
  {
    return;
  }

  const auto& vertices = mesh->vertices;
  ASSERT_EQ(vertices.size(), 8U);

  const rtw::math::Point3F expected_vertices[] = {
      rtw::math::Point3F{1.0F, 1.0F, -1.0F},  rtw::math::Point3F{1.0F, -1.0F, -1.0F},
      rtw::math::Point3F{1.0F, 1.0F, 1.0F},   rtw::math::Point3F{1.0F, -1.0F, 1.0F},
      rtw::math::Point3F{-1.0F, 1.0F, -1.0F}, rtw::math::Point3F{-1.0F, -1.0F, -1.0F},
      rtw::math::Point3F{-1.0F, 1.0F, 1.0F},  rtw::math::Point3F{-1.0F, -1.0F, 1.0F},
  };
  EXPECT_THAT(vertices, testing::ElementsAreArray(expected_vertices));

  const auto& faces = mesh->faces;
  ASSERT_EQ(faces.size(), 12);

  // clang-format off
  const rtw::sw_renderer::Face expected_faces[] = {
      make_face({5U, 3U, 1U}, { 1U,  2U,  3U}, {1U, 1U, 1U}, "White"),
      make_face({5U, 7U, 3U}, { 1U,  4U,  2U}, {1U, 1U, 1U}, "White"),
      make_face({2U, 8U, 6U}, { 5U,  6U,  7U}, {2U, 2U, 2U}, "Yellow"),
      make_face({2U, 4U, 8U}, { 5U,  8U,  6U}, {2U, 2U, 2U}, "Yellow"),
      make_face({3U, 8U, 4U}, { 2U,  9U,  8U}, {3U, 3U, 3U}, "Green"),
      make_face({3U, 7U, 8U}, { 2U, 10U,  9U}, {3U, 3U, 3U}, "Green"),
      make_face({5U, 2U, 6U}, {11U,  5U, 12U}, {4U, 4U, 4U}, "Blue"),
      make_face({5U, 1U, 2U}, {11U,  3U,  5U}, {4U, 4U, 4U}, "Blue"),
      make_face({1U, 4U, 2U}, { 3U,  8U,  5U}, {5U, 5U, 5U}, "Red"),
      make_face({1U, 3U, 4U}, { 3U,  2U,  8U}, {5U, 5U, 5U}, "Red"),
      make_face({7U, 6U, 8U}, {13U, 12U, 14U}, {6U, 6U, 6U}, "Orange"),
      make_face({7U, 5U, 6U}, {13U, 11U, 12U}, {6U, 6U, 6U}, "Orange"),
  };
  // clang-format on
  for (std::size_t i = 0U; i < faces.size(); ++i)
  {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    const auto expected_face = expected_faces[i];

    ASSERT_TRUE(expected_face.texture_indices.has_value());
    ASSERT_TRUE(expected_face.normal_indices.has_value());

    if (!expected_face.texture_indices.has_value() || !expected_face.normal_indices.has_value())
    {
      continue;
    }

    ASSERT_EQ(faces[i].vertex_indices, expected_face.vertex_indices) << "Face " << i;
    ASSERT_EQ(faces[i].texture_indices, *expected_face.texture_indices) << "Face " << i;
    ASSERT_EQ(faces[i].normal_indices, *expected_face.normal_indices) << "Face " << i;
    ASSERT_EQ(faces[i].material, expected_faces[i].material);
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
  }

  const auto& materials = mesh->materials;
  ASSERT_EQ(materials.size(), 6U);

  for (const auto& [name, material] : materials)
  {
    if (name == "Blue")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "Blue", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {0.0F, 0.0F, 1.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else if (name == "Green")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "Green", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {0.0F, 1.0F, 0.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else if (name == "Orange")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "Orange", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {1.0F, 0.5F, 0.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else if (name == "Red")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "Red", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {1.0F, 0.0F, 0.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else if (name == "White")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "White", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {1.0F, 1.0F, 1.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else if (name == "Yellow")
    {
      const auto expected_material = rtw::sw_renderer::Material{
          "Yellow", {}, {}, {}, {1.0F, 1.0F, 1.0F, 1.0F}, {1.0F, 1.0F, 0.0F, 1.0F}, {0.5F, 0.5F, 0.5F, 1.0F}};
      ASSERT_EQ(material.name, expected_material.name);
      ASSERT_EQ(material.ambient.rgba, expected_material.ambient.rgba);
    }
    else
    {
      FAIL() << "Unexpected material name: " << name;
    }
  }
}
// NOLINTEND(readability-function-cognitive-complexity)
