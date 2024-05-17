#include "stl_loader.hpp"
#include "geometry.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace eh
{

std::vector<Triangle> load_stl_ascii(std::ifstream& ifs)
{
  throw std::runtime_error("ASCII STL is not supported yet");
  return {};
}
std::vector<Triangle> load_stl_binary(std::ifstream& ifs, bool vertex_normal)
{
  ifs.seekg(80, std::ios::beg);
  uint32_t triangle_count;
  ifs.read((char*)&triangle_count, 4);
  std::vector<Triangle> triangles;
  triangles.resize(triangle_count);
  std::cout << "Load STL: " << triangle_count << " triangles" << std::endl;
  for (unsigned int i = 0; i < triangle_count; ++i)
  {
    vec3 normal;
    ifs.read((char*)&normal, 12);
    ifs.read((char*)triangles[i].p0.data(), 12);
    ifs.read((char*)triangles[i].p1.data(), 12);
    ifs.read((char*)triangles[i].p2.data(), 12);

    vec3 a = triangles[i].p1 - triangles[i].p0;
    vec3 b = triangles[i].p2 - triangles[i].p0;
    vec3 c = a.cross(b);
    c.normalize();
    triangles[i].n0 = triangles[i].n1 = triangles[i].n2 = c;
    ifs.seekg(2, std::ios::cur);
  }

  if (vertex_normal == false)
  {
    return triangles;
  }

  // for vertex-normals
  std::vector<std::pair<vec3, vec3>> vertex_normals;
  vertex_normals.reserve(triangles.size() * 3);
  for (auto& t : triangles)
  {
    vertex_normals.push_back(std::make_pair(t.p0, t.n0));
    vertex_normals.push_back(std::make_pair(t.p1, t.n1));
    vertex_normals.push_back(std::make_pair(t.p2, t.n2));
  }

  std::vector<Triangle> new_triangles = triangles;
  for (int i = 0; i < new_triangles.size(); ++i)
  {
    vec3 normal = vec3::Zero();
    int cnt = 0;
    for (auto const& vn : vertex_normals)
    {
      if ((new_triangles[i].p0 - vn.first).squaredNorm() < 1e-6)
      {
        normal += vn.second;
        ++cnt;
      }
    }
    new_triangles[i].n0 = normal / cnt;

    normal = vec3::Zero();
    cnt = 0;
    for (auto const& vn : vertex_normals)
    {
      if ((new_triangles[i].p1 - vn.first).squaredNorm() < 1e-6)
      {
        normal += vn.second;
        ++cnt;
      }
    }
    new_triangles[i].n1 = normal / cnt;

    normal = vec3::Zero();
    cnt = 0;
    for (auto const& vn : vertex_normals)
    {
      if ((new_triangles[i].p2 - vn.first).squaredNorm() < 1e-6)
      {
        normal += vn.second;
        ++cnt;
      }
    }
    new_triangles[i].n2 = normal / cnt;

    new_triangles[i].n0.normalize();
    new_triangles[i].n1.normalize();
    new_triangles[i].n2.normalize();
  }

  return new_triangles;
}
std::vector<Triangle> load_stl(std::string const& filename, bool vertex_normal)
{
  std::ifstream ifs(filename, std::ios::binary);
  char first_five_bytes[5];
  ifs.read(first_five_bytes, 5);
  ifs.close();

  if (std::string(first_five_bytes, 5) != "solid")
  {
    // binary
    ifs = std::ifstream(filename, std::ios::binary);
    return load_stl_binary(ifs, vertex_normal);
  }
  ifs = std::ifstream(filename);
  return load_stl_ascii(ifs);
}

void write_stl(std::string const& filename,
               std::vector<Triangle> const& triangles)
{
  std::ofstream ofs(filename, std::ios::binary);
  char header[80] = { 0 };
  ofs.write(header, 80);
  uint32_t triangle_count = triangles.size();
  ofs.write((char*)&triangle_count, 4);
  for (auto const& t : triangles)
  {
    vec3 normal = t.n0;
    ofs.write((char*)&normal, 12);
    ofs.write((char*)t.p0.data(), 12);
    ofs.write((char*)t.p1.data(), 12);
    ofs.write((char*)t.p2.data(), 12);
    uint16_t attribute = 0;
    ofs.write((char*)&attribute, 2);
  }
  ofs.close();
}

}