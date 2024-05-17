#pragma once

#include <geometry.hpp>
#include <string>
#include <vector>

namespace eh
{
std::vector<Triangle> load_stl(std::string const& filename,
                               bool vertex_normal = false);
void write_stl(std::string const& filename,
               std::vector<Triangle> const& triangles);
}