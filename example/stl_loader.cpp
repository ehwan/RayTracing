#include "stl_loader.hpp"
#include "geometry.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace eh {

std::vector<Triangle> load_stl_ascii( std::ifstream &ifs )
{
  throw std::runtime_error("ASCII STL is not supported yet");
  return {};
}
std::vector<Triangle> load_stl_binary( std::ifstream &ifs )
{
  ifs.seekg( 80, std::ios::beg );
  uint32_t triangle_count;
  ifs.read( (char*)&triangle_count, 4 );
  std::vector<Triangle> triangles;
  triangles.resize( triangle_count );
  std::cout << "Load STL: " << triangle_count << " triangles" << std::endl;
  for( unsigned int i=0; i<triangle_count; ++i )
  {
    vec3 normal;
    ifs.read( (char*)&normal, 12 );
    normal.normalize();
    ifs.read( (char*)triangles[i].p0.data(), 12 );
    ifs.read( (char*)triangles[i].p1.data(), 12 );
    ifs.read( (char*)triangles[i].p2.data(), 12 );
    triangles[i].n0 = triangles[i].n1 = triangles[i].n2 = normal;
    ifs.seekg( 2, std::ios::cur );
  }
  return triangles;
}
std::vector<Triangle> load_stl( std::string const& filename )
{
  std::ifstream ifs( filename, std::ios::binary );
  char first_five_bytes[5];
  ifs.read( first_five_bytes, 5 );
  ifs.close();

  if( std::string(first_five_bytes,5) != "solid" )
  {
    // binary
    ifs = std::ifstream( filename, std::ios::binary );
    return load_stl_binary( ifs );
  }
  ifs = std::ifstream( filename );
  return load_stl_ascii( ifs );
}

void write_stl( std::string const& filename, std::vector<Triangle> const& triangles )
{
  std::ofstream ofs( filename, std::ios::binary );
  char header[80] = {0};
  ofs.write( header, 80 );
  uint32_t triangle_count = triangles.size();
  ofs.write( (char*)&triangle_count, 4 );
  for( auto const& t : triangles )
  {
    vec3 normal = t.n0;
    ofs.write( (char*)&normal, 12 );
    ofs.write( (char*)t.p0.data(), 12 );
    ofs.write( (char*)t.p1.data(), 12 );
    ofs.write( (char*)t.p2.data(), 12 );
    uint16_t attribute = 0;
    ofs.write( (char*)&attribute, 2 );
  }
  ofs.close();
}

}