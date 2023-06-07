#pragma once

#include "world.hpp"

#include <fstream>
#include <memory>

// base demo world;
// contains SkySphere ( LightSource ), Checkered-Tile Plane
struct BaseDemoWorld
  : eh::World
{
  eh::SkySphere skysphere;
  eh::DirectionalLightSource sky_source;

  eh::Plane chekered_floor;
  eh::DiffuseReflection floor_reflect;

  BaseDemoWorld( int W, int H )
    : eh::World( W, H )
  {
    this->max_bounce = 3;
    this->sample_count = 10;

    // big sky sphere - light source
    skysphere.color = eh::vec3(1,1,1);
    skysphere.center = eh::vec3::Zero();
    skysphere.radius = 100.0f;
    skysphere.reflection = &sky_source;
    this->objects.push_back(&skysphere);

    // floor - checkered tiles
    chekered_floor.reflection = &floor_reflect;
    chekered_floor.center = eh::vec3( 0.0f, -1.0f, 0.0f );
    chekered_floor.normal = eh::vec3( 0.0f, 1.0f, 0.0f );
    this->objects.push_back( &chekered_floor );


    // camera position init
    this->camera.position( {0.2,1.0,2.0} );
    this->camera.angle( {-0.3,-0.0,0} );
    this->camera.perspective( 3.141592f/2.0f, 1.0f, 0.1 );
    this->camera.move( 2, 0.5f );
  }
};

struct SphereDemo
  : BaseDemoWorld
{
  eh::Sphere s1, s2, s3;
  eh::DiffuseReflection s1_reflect;
  eh::Refragtion s2_reflect;
  // eh::DiffuseReflection s2_reflect;
  eh::MirrorReflection s3_reflect;

  eh::Triangle triangle;
  eh::MirrorReflection triangle_reflect;

  SphereDemo( int w, int h )
    : BaseDemoWorld(w,h)
  {
    // objects
    s1.center = eh::vec3( -2.05f, 0.0f, -5.0f );
    s1.radius = 1.0f;
    s1.color = eh::vec3( 1.0f, 0.6f, 0.4f );
    s1.reflection = &s1_reflect;

    s2.center = eh::vec3( 2.05f, 0.0f, -5.0f );
    s2.radius = 1.0f;
    s2.color = eh::vec3( 0.6f, 0.8f, 1.0f );
    s2.reflection = &s2_reflect;
    s2_reflect.index = 0.85f;

    s3.center = eh::vec3( 0.0f, 0.0f, -5.0f );
    s3.radius = 1.0f;
    s3.color = eh::vec3(1,1,1);
    s3.reflection = &s3_reflect;

    this->objects.push_back( &s1 );
    this->objects.push_back( &s2 );
    this->objects.push_back( &s3 );

    // mirror triangle
    triangle.p0 = { 0.0f, 1.5f, -4.5f };
    triangle.p1 = { -1.0f, 2.0f, -2.0f };
    triangle.p2 = { 1.0f, 2.0f, -2.0f };
    triangle.n0 = { 0, -1, 0 };
    triangle.n1 = { 0, -1, 0 };
    triangle.n2 = { 0, -1, 0 };
    triangle.reflection = &triangle_reflect;
    triangle.color = eh::vec3(1,1,1);
    this->objects.push_back( &triangle );
  }
};
struct MarchingCubesDemoWorld
  : BaseDemoWorld
{
  // marching cubes vertex data file
  std::ifstream file{ "sph_marching_cubes.txt.outsync" };
  std::vector< std::unique_ptr<eh::Triangle> > marching_cubes;
  eh::Refragtion fluid_refract;
  eh::DiffuseReflection fluid_diffuse;
  eh::MirrorReflection fluid_mirror;

  MarchingCubesDemoWorld( int w, int h )
    : BaseDemoWorld(w,h)
  {
    // fluid refraction index \approx 0.70 ( Water )
    fluid_refract.index = 0.70f;

    // load marching cubes vertex data from file
    while( 1 )
    {
      float t;
      int nvert;
      int ntri;
      file.read( (char*)&t, sizeof(t) );
      file.read( (char*)&nvert, sizeof(nvert) );
      file.read( (char*)&ntri, sizeof(ntri) );
      std::cout << t << "\n" << nvert << "\n" << ntri << "\n";

      if( t < 1.1f )
      {
        file.seekg( sizeof(float)*3*nvert*2, file.cur );
        file.seekg( sizeof(unsigned int)*3*ntri, file.cur );
      }else {
        std::vector<float> verts( 3*nvert );
        std::vector<float> norms( 3*nvert );
        std::vector<unsigned int> tris( 3*ntri );
        file.read( (char*)verts.data(), sizeof(float)*3*nvert );
        file.read( (char*)norms.data(), sizeof(float)*3*nvert );
        file.read( (char*)tris.data(), sizeof(unsigned int)*3*ntri );

        marching_cubes.resize( ntri );
        for( int i=0; i<ntri; ++i )
        {
          eh::Triangle *tri = new eh::Triangle;
          int i0 = tris[3*i];
          int i1 = tris[3*i+1];
          int i2 = tris[3*i+2];
          tri->p0 = eh::vec3( verts[3*i0], verts[3*i0+1], verts[3*i0+2] );
          tri->p1 = eh::vec3( verts[3*i1], verts[3*i1+1], verts[3*i1+2] );
          tri->p2 = eh::vec3( verts[3*i2], verts[3*i2+1], verts[3*i2+2] );
          tri->n0 = eh::vec3( norms[3*i0], norms[3*i0+1], norms[3*i0+2] );
          tri->n1 = eh::vec3( norms[3*i1], norms[3*i1+1], norms[3*i1+2] );
          tri->n2 = eh::vec3( norms[3*i2], norms[3*i2+1], norms[3*i2+2] );
          marching_cubes[i].reset( tri );
          tri->color = eh::vec3( 0.6f, 0.8f, 1.0f );
          tri->reflection = &fluid_refract;

          this->objects.push_back( tri );
        }
        break;
      }
    }
  }
};
