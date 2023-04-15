#include "global.hpp"
#include "world.hpp"
#include "lodepng.h"
#include <memory>
#include <iostream>

int main()
{
  eh::World world;
  world.max_bounce = 3;
  world.sample_count = 30;
  world.width = 400;
  world.height = 400;
  world.clear_framebuffer();

// light source
  eh::Sphere lightsource;
  lightsource.center = eh::vec3( 0, 100, 0 );
  lightsource.radius = 50.0f;

// light source reflection model
  eh::DirectionalLightSource lightsource_reflect;
  float light_power = 6.0f;
  lightsource_reflect.source_color = eh::vec3(light_power,light_power,light_power);
  lightsource.reflection = &lightsource_reflect;
  world.objects.push_back(&lightsource);


// objects
  eh::Sphere s1, s2, s3;
  eh::FuzzyMirrorReflection s1_reflect;
  eh::DiffuseReflection s2_reflect;
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

  s3.center = eh::vec3( 0.0f, 0.0f, -5.0f );
  s3.radius = 1.0f;
  s3.reflection = &s3_reflect;

  world.objects.push_back( &s1 );
  world.objects.push_back( &s2 );
  world.objects.push_back( &s3 );

// floor - check tiles
  eh::Plane floor;
  eh::DiffuseReflection floor_reflect;
  floor.reflection = &floor_reflect;
  floor.center = eh::vec3( 0.0f, -2.0f, 0.0f );
  floor.normal = eh::vec3( 0.0f, 1.0f, 0.0f );
  world.objects.push_back( &floor );

  for( int i=0; i<200; ++i )
  {
    std::cout << i << "\n";
    world.render_once();
    if( (i%20) == 0 )
    {
      auto buffer = world.get_imagebuffer();
      std::string filename = std::string("res/b") + std::to_string(i) + std::string(".png");
      lodepng::encode( filename, buffer, world.width, world.height, LodePNGColorType::LCT_RGB );
    }
  }

  return 0;
}
