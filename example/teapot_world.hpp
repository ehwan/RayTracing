#pragma once

#include "geometry.hpp"
#include "reflection.hpp"
#include "world.hpp"
#include "stl_loader.hpp"

// base demo world;
class TeapotDemo
  : public eh::World
{
  using vec3 = eh::vec3;


  // return chckered-tile color by ray's hit point
  struct FloorColorSource
    : public eh::ReflectionModel
  {
    vec3 get_color( eh::Ray const& r, eh::RayHit const& hit, World &world ) const override
    {
      vec3 point = hit.point( r );
      // checkered-tile

      const float width = 1.0f;
      int xi = (int)std::floor(point.x()/width);
      int zi = (int)std::floor(point.z()/width);

      if( (xi^zi)&1 )
      {
        return vec3::Constant(1);
      }
      return vec3::Constant(0.3f);
    }
  };
public:
  struct 
  {
    eh::Sphere skysphere{ eh::vec3::Zero(), 100.0f };

    eh::Triangle floor1{ vec3(-20.0f, -2.0f, 0.0f), vec3(20.0f, -2.0f, 0.0f), vec3(-20.0f,-2.0f,-40.0f),
                         vec3(0,1,0), vec3(0,1,0), vec3(0,1,0) };
    eh::Triangle floor2{ vec3(-20.0f,-2.0f,-40.0f), vec3(20.0f, -2.0f, 0.0f), vec3(20.0f,-2.0f,-40.0f),
                         vec3(0,1,0), vec3(0,1,0), vec3(0,1,0) };

    std::vector<eh::Triangle> teapot;
  } geometries;

  struct
  {
    eh::LightSource lightsource { eh::vec3::Constant(1) };

    eh::DiffuseReflection diffusive;
    eh::Refragtion water_refragtion;
    eh::MirrorReflection mirror;
    eh::FuzzyMirrorReflection fuzzy_mirror;
    eh::CombineReflection combine;

    FloorColorSource floor_color;
    eh::DiffuseReflection floor_diffuse;
    eh::MultiplyReflection floor;
  } reflections;

  TeapotDemo( int w, int h, int thread_count )
  {
    this->max_bounce = 3;
    this->init( w, h, thread_count );

    geometries.teapot = eh::load_stl( TEAPOT_PATH );
    for( auto &t : geometries.teapot )
    {
      t.p0.x() += 0.2f;
      t.p1.x() += 0.2f;
      t.p2.x() += 0.2f;
      t.p0.y() -= 2;
      t.p1.y() -= 2;
      t.p2.y() -= 2;
      t.p0.z() -= 10;
      t.p1.z() -= 10;
      t.p2.z() -= 10;
    }

    this->insert( {&geometries.skysphere, &reflections.lightsource} );
    this->insert( {&geometries.floor1, &reflections.floor} );
    this->insert( {&geometries.floor2, &reflections.floor} );
    for( auto &t : geometries.teapot )
    {
      this->insert( {&t, &reflections.diffusive} );
    }
    reflections.fuzzy_mirror.fuzzyness = 0.3f;
    reflections.fuzzy_mirror.sample_count = 10;
    reflections.fuzzy_mirror.color = vec3(1.2f,0.8f,0.6f);

    reflections.combine.r1 = &reflections.water_refragtion;
    reflections.combine.r2 = &reflections.diffusive;
    reflections.combine.s1 = 0.5f;
    reflections.combine.s2 = 0.5f;

    // camera position init
    this->camera.position( {0.2,1.0,2.0} );
    this->camera.angle( {-0.3,-0.0,0} );
    this->camera.perspective( 3.141592f/2.0f, 1.0f, 1.0f );
    this->camera.move( 2, 0.5f );

    // objects
    reflections.diffusive.color = eh::vec3( 1.2f, 0.8f, 0.6f );
    reflections.diffusive.sample_count = 10;

    reflections.water_refragtion.color = eh::vec3( 0.6f, 0.8f, 1.0f );
    reflections.water_refragtion.index = 0.85f;

    reflections.mirror.color = eh::vec3( 1.0f, 1.0f ,1.0f );

    reflections.floor_diffuse.sample_count = 10;
    reflections.floor_diffuse.color = vec3(1,1,1);

    reflections.floor.r1 = &reflections.floor_color;
    reflections.floor.r2 = &reflections.floor_diffuse;
  }
};