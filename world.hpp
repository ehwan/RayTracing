#pragma once

#include "global.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "geometry.hpp"
#include "reflection.hpp"
#include <limits>
#include <thread>
#include <vector>
#include <random>
#include <utility>

namespace eh {

struct World
{
  constexpr static float PI = 3.141592f;
  std::mt19937 mt_twister{ std::random_device{}() };
  std::uniform_real_distribution<float> uniform_dist{ 0.0f, 1.0f };

  std::vector<GeometryObject*> objects;
  std::vector<std::pair<vec3,int>> framebuffer;
  int max_bounce = 2;
  int sample_count = 10;
  int width = 100;
  int height = 100;

  void raycast( Ray &r )
  {
    r.t = std::numeric_limits<float>::infinity();
    r.surface = nullptr;
    Ray test_ray = r;
    for( auto *g : objects )
    {
      test_ray.surface = nullptr;
      g->raycast(test_ray);
      if( test_ray.surface != nullptr && test_ray.t < r.t )
      {
        r = test_ray;
      }
    }
  }

  vec3 random_sphere()
  {
    // angle around y axis
    const float angle1 = uniform_dist(mt_twister)*2*PI;
    // angle above xz plane
    const float angle2 = (uniform_dist(mt_twister)-0.5)*PI;

    const float c = std::cos(angle1);
    return { std::cos(angle2)*c, std::sin(angle1), std::sin(angle2)*c };
  }
  vec3 random_semisphere()
  {
    // angle around z axis
    const float angle1 = uniform_dist(mt_twister)*2*PI;
    // angle above xy plane
    const float angle2 = uniform_dist(mt_twister)*0.5f*PI;

    const float c = std::cos(angle1);
    return { std::cos(angle2)*c, std::sin(angle2)*c, std::sin(angle1) };
  }
  vec3 get_color( Ray &r )
  {
    if( r.bounce == max_bounce ){ return vec3::Zero(); }

    raycast( r );
    if( r.surface == nullptr ){ return vec3::Zero(); }
    return r.surface->reflection->get_color(r,*this,*r.surface);
  }
  void clear_framebuffer()
  {
    framebuffer.resize( width*height, {vec3::Zero(),0} );
  }
  void render_pixel( int x, int y )
  {
    // eyepos = (0,0,0)
    // eye normal = (0,0,-1);
    vec2 rf = vec2::Random();
    float xf = x + rf(0);
    float yf = y + rf(1);
    xf = (xf/width)*2.0f - 1.0f;
    yf = (yf/height)*2.0f - 1.0f;
    vec3 point = { xf, -yf, -1.0f };
    Ray ray;
    ray.origin = point;
    ray.direction = point.normalized();

    auto color = get_color(ray);

    auto &p = framebuffer[y*width+x];
    p.first = p.first*(float)p.second/(float)(p.second+1) + color/(float)(p.second+1);
    ++p.second;
  }
  void render_once()
  {
    /*
    std::vector<std::thread> threads;
    struct renderer_t
    {
      World *world;
      int y;
      void operator()()
      {
        for( int x=0; x<world->width; ++x )
        {
          world->render_pixel(x,y);
        }
      }
    };
    for( int y=0; y<height; ++y )
    {
      renderer_t runner;
      runner.world = this;
      runner.y = y;
      threads.emplace_back( runner );
    }
    for( auto &t : threads )
    {
      t.join();
    }
    */
    for( int y=0; y<height; ++y )
    {
      for( int x=0; x<width; ++x )
      {
        render_pixel( x, y );
      }
    }
  }

  std::vector<unsigned char> get_imagebuffer()
  {
    std::vector<unsigned char> ret( width*height*3 );
    for( int y=0; y<height; ++y )
    {
      for( int x=0; x<width; ++x )
      {
        auto vi = vec3_to_color(framebuffer[y*width+x].first);
        ret[(y*width+x)*3 + 0] = (unsigned char)vi(0);
        ret[(y*width+x)*3 + 1] = (unsigned char)vi(1);
        ret[(y*width+x)*3 + 2] = (unsigned char)vi(2);
      }
    }
    return ret;
  }
};

}