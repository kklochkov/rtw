#include "sw_renderer/obj_loader.h"

#include <fstream>
#include <sstream>

namespace rtw::sw_renderer
{

namespace
{

Point3F parse_vertex(const std::string& line)
{
  std::istringstream iss(line);
  iss.ignore(1); // Ignore "v"

  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;

  iss >> x >> y >> z;

  return Point3F{x, y, z};
}

TexCoordF parse_tex_coord(const std::string& line)
{
  std::istringstream iss(line);
  iss.ignore(2); // Ignore "vt"

  float u = 0.0F;
  float v = 0.0F;

  iss >> u >> v;

  return TexCoordF{u, v};
}

Vector3F parse_normal(const std::string& line)
{
  std::istringstream iss(line);
  iss.ignore(2); // Ignore "vn"

  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;

  iss >> x >> y >> z;

  return Vector3F{x, y, z};
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
        mesh.vertices.emplace_back(parse_vertex(line));
      }
      break;
      case 't': // Texture coordinate
      {
        mesh.tex_coords.emplace_back(parse_tex_coord(line));
      }
      break;
      case 'n': // Normal
      {
        mesh.normals.emplace_back(parse_normal(line));
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
    return std::nullopt;
  }

  auto result = load_obj(file);
  for (const auto& material : result.materials)
  {
    const auto material_path = path.parent_path() / material;
    std::ifstream material_file(material_path);
    if (!material_file.is_open())
    {
      return std::nullopt;
    }

    load_mtl(material_file, result.mesh);
  }
  return result.mesh;
}

} // namespace rtw::sw_renderer
