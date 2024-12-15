#include "sw_renderer/obj_loader.h"

#include <fmt/core.h>

#include <fstream>
#include <sstream>

namespace rtw::sw_renderer
{

namespace
{

void parse_vertex(const std::string& line, math::Point3f& vertex)
{
  std::istringstream iss(line);
  iss.ignore(1); // Ignore "v"

  iss >> vertex.x() >> vertex.y() >> vertex.z();
}

void parse_tex_coord(const std::string& line, TexCoord2f& tex_coord)
{
  std::istringstream iss(line);
  iss.ignore(2); // Ignore "vt"

  iss >> tex_coord.u() >> tex_coord.v();
}

void parse_normal(const std::string& line, math::Vector3f& normal)
{
  std::istringstream iss(line);
  iss.ignore(2); // Ignore "vn"

  iss >> normal.x() >> normal.y() >> normal.z();
}

bool try_parse_index(std::istringstream& iss, std::uint32_t& index)
{
  const bool ok = std::isdigit(iss.peek()) != 0;
  if (ok)
  {
    iss >> index;
    --index; // OBJ indices start at 1
  }

  if (iss.peek() == '/')
  {
    iss.ignore(1); // Ignore "/"
  }
  return ok;
}

void parse_face(const std::string& line, Face& face)
{
  std::istringstream iss(line);
  iss.ignore(1); // Ignore "f"

  for (std::size_t i = 0U; i < 3U; ++i)
  {
    iss.ignore(1); // Ignore " "
    std::uint32_t index = 0U;
    if (const bool ok = try_parse_index(iss, index))
    {
      assert(ok);
      face.vertex_indices[i] = index;
    }

    if (try_parse_index(iss, index))
    {
      auto& texture_indices = face.texture_indices.has_value() ? *face.texture_indices : face.texture_indices.emplace();
      texture_indices[i] = index;
    }

    if (try_parse_index(iss, index))
    {
      auto& normal_indices = face.normal_indices.has_value() ? *face.normal_indices : face.normal_indices.emplace();
      normal_indices[i] = index;
    }
  }
}

std::string parse_material(const std::string& line, const std::string_view prefix)
{
  std::istringstream iss(line);
  iss.ignore(static_cast<std::streamsize>(prefix.size()));

  std::string material;
  iss >> material;
  return material;
}

Color parse_color(const std::string& line)
{
  std::istringstream iss(line);
  iss.ignore(3); // Ignore "K"

  float r = 1.0F;
  float g = 1.0F;
  float b = 1.0F;
  iss >> r >> g >> b;

  return Color{r, g, b};
}

} // namespace

ObjParseResult load_obj(std::istream& stream)
{
  ObjParseResult result;
  Mesh& mesh = result.mesh;
  std::string material;

  for (std::string line; std::getline(stream, line);)
  {
    if (line.empty() || line[0] == '#')
    {
      continue;
    }

    if (line[0] == 'm')
    {
      // Material library
      result.materials.emplace_back(parse_material(line, "mtllib"));
    }
    else if (line[0] == 'v')
    {
      switch (line[1])
      {
      case ' ': // Vertex
      {
        math::Point3f& vertex = mesh.vertices.emplace_back();
        parse_vertex(line, vertex);
      }
      break;
      case 't': // Texture coordinate
      {
        TexCoord2f& tex_coord = mesh.tex_coords.emplace_back();
        parse_tex_coord(line, tex_coord);
      }
      break;
      case 'n': // Normal
      {
        math::Vector3f& normal = mesh.normals.emplace_back();
        parse_normal(line, normal);
      }
      break;
      default:
        break;
      }
    }
    else if (line[0] == 'u')
    {
      // Use material
      material = parse_material(line, "usemtl");
    }
    else if (line[0] == 'f')
    {
      // Face
      Face& face = mesh.faces.emplace_back();
      face.material = material;
      parse_face(line, face);
    }
  }

  return result;
}

void load_mtl(std::istream& stream, Mesh& mesh)
{
  Material material;

  const auto try_add_material = [](Mesh& mesh, Material& material)
  {
    if (!material.name.empty())
    {
      for (const auto& texture : {material.ambient_texture, material.diffuse_texture, material.specular_texture})
      {
        if (!texture.empty())
        {
          mesh.textures.emplace(texture, Texture{});
        }
      }

      mesh.materials.emplace(material.name, std::move(material));
      material = {};
    }
  };

  for (std::string line; std::getline(stream, line);)
  {
    if (line.empty() || line[0] == '#')
    {
      try_add_material(mesh, material);
      continue;
    }

    if (line[0] == 'n')
    {
      // New material
      material.name = parse_material(line, "newmtl");
    }
    else if (line[0] == 'K')
    {
      // Color
      switch (line[1])
      {
      case 'a': // Ambient
        material.ambient = parse_color(line);
        break;
      case 'd': // Diffuse
        material.diffuse = parse_color(line);
        break;
      case 's': // Specular
        material.specular = parse_color(line);
        break;
      default:
        break;
      }
    }
    else if (line[0] == 'm')
    {
      // Texture
      switch (line[5])
      {
      case 'a': // Ambient
        material.ambient_texture = parse_material(line, "map_Ka");
        break;
      case 'd': // Diffuse
        material.diffuse_texture = parse_material(line, "map_Kd");
        break;
      case 's': // Specular
        material.specular_texture = parse_material(line, "map_Ks");
        break;
      default:
        break;
      }
    }
  }

  try_add_material(mesh, material);
}

std::optional<Mesh> load_obj(const std::filesystem::path& path)
{
  std::ifstream file(path);
  if (!file.is_open())
  {
    fmt::print("load_obj: could not open file {}.\n", path.c_str());
    return std::nullopt;
  }

  auto result = load_obj(file);
  for (const auto& material : result.materials)
  {
    const auto material_path = path.parent_path() / material;
    std::ifstream material_file(material_path);
    if (!material_file.is_open())
    {
      fmt::print("load_obj: could not open file {}.\n", material_path.c_str());
      return std::nullopt;
    }

    load_mtl(material_file, result.mesh);
  }
  fmt::print("load_obj: loaded {} vertices, {} texture coordinates, {} normals, {} faces, materials {}, textures {}.\n",
             result.mesh.vertices.size(), result.mesh.tex_coords.size(), result.mesh.normals.size(),
             result.mesh.faces.size(), result.mesh.materials.size(), result.mesh.textures.size());
  return result.mesh;
}

} // namespace rtw::sw_renderer
