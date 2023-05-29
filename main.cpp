#include "global.hpp"
#include "reflection.hpp"
#include "world.hpp"
#include "lodepng.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

int main()
{
  eh::World world;
  world.max_bounce = 3;
  world.sample_count = 10;
  world.width = 400;
  world.height = 400;
  world.clear_framebuffer();

// big sky sphere - light source
  eh::SkySphere skysphere;
  skysphere.color = eh::vec3(1,1,1);
  skysphere.center = eh::vec3::Zero();
  skysphere.radius = 100.0f;
  eh::DirectionalLightSource sky_source;
  // eh::DiffuseReflection sky_reflect;
  skysphere.reflection = &sky_source;
  world.objects.push_back(&skysphere);
  /*
  eh::Sphere lightsource;
  lightsource.center = { 3, 10, -0.7 };
  lightsource.radius = 5;
  lightsource.color = { 8,8,8 };
  eh::DirectionalLightSource lightsource_reflect;
  lightsource.reflection = &lightsource_reflect;
  world.objects.push_back(&lightsource);
  */

// floor - checkered tiles
  eh::Plane floor;
  eh::DiffuseReflection floor_reflect;
  floor.reflection = &floor_reflect;
  floor.center = eh::vec3( 0.0f, -1.0f, 0.0f );
  floor.normal = eh::vec3( 0.0f, 1.0f, 0.0f );
  world.objects.push_back( &floor );

  // world.camera.position( {0,0,0} );
  // world.camera.angle( {0.0f,-0.1f,0.0f} );
  world.camera.position( {0.2,1.0,2.0} );
  world.camera.angle( {-0.3,-0.0,0} );
  world.camera.perspective( 3.141592f/2.0f, 1.0f, 0.1 );
  world.camera.move( 2, 0.5f );
  //world.camera.move( 0, 0.2f );



// objects
  eh::Sphere s1, s2, s3;
  eh::FuzzyMirrorReflection s1_reflect;
  eh::Refragtion s2_reflect;
  // eh::DiffuseReflection s2_reflect;
  eh::MirrorReflection s3_reflect;
  s1.center = eh::vec3( -2.05f, 0.0f, -5.0f );
  s1.radius = 1.0f;
  s1.color = eh::vec3( 1.0f, 0.6f, 0.4f );
  s1_reflect.fuzzyness = 0.25f;
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

  world.objects.push_back( &s1 );
  world.objects.push_back( &s2 );
  world.objects.push_back( &s3 );

  // mirror triangle
  eh::Triangle triangle;
  triangle.p0 = { 0.0f, 1.5f, -4.5f };
  triangle.p1 = { -1.0f, 2.0f, -2.0f };
  triangle.p2 = { 1.0f, 2.0f, -2.0f };
  triangle.n0 = { 0, -1, 0 };
  triangle.n1 = { 0, -1, 0 };
  triangle.n2 = { 0, -1, 0 };
  eh::MirrorReflection mirror_reflect;
  triangle.reflection = &mirror_reflect;
  triangle.color = eh::vec3(1,1,1);
  world.objects.push_back( &triangle );

// rendering loop
  for( int i=0; i<200; ++i )
  {
    std::cout << i << "\n";
    world.render_once();
    if( (i%10) == 0 )
    {
      auto buffer = world.get_imagebuffer();
      std::string filename = std::string("res/result") + std::to_string(i) + std::string(".png");
      lodepng::encode( filename, buffer, world.width, world.height, LodePNGColorType::LCT_RGB );
    }
  }

  return 0;
}


void load_marching_cubes()
{
  // load marching-cubes data
  /*
  std::ifstream file( "/Users/ehwan/workspace/tatsuno/opencl3d/out.txt.nosync" );
  std::vector<std::unique_ptr<eh::Triangle>> marching_cubes;
  eh::Refragtion fluid_refract;
  fluid_refract.index = 0.70f;
  // eh::DiffuseReflection fluid_diffuse;
  eh::FuzzyMirrorReflection fluid_diffuse;
  fluid_diffuse.fuzzyness = 0.5f;
  eh::CombineReflection fluid_combine;
  fluid_combine.r1 = &fluid_refract;
  fluid_combine.r2 = &fluid_diffuse;
  fluid_combine.s1 = 0.65f;
  fluid_combine.s2 = 0.35f;
  eh::FaceReflection fluid_reflect;
  fluid_reflect.front = &fluid_combine;
  fluid_reflect.back = &fluid_refract;
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
        tri->reflection = &fluid_reflect;

        world.objects.push_back( tri );
      }
      break;
    }
  }
  */
}
