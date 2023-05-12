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
#include "camera.hpp"

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

  EyeAngle camera;

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
    if( r.bounce >= max_bounce ){ return vec3::Zero(); }

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
    vec2 rf = vec2::Random();
    float xf = (x + rf(0))/(float)width;
    float yf = (y + rf(1))/(float)height;
    vec3 point = camera(xf,yf);
    Ray ray;
    ray.origin = point;
    ray.direction = point - camera(vec3(0,0,0));
    ray.direction.normalize();
    auto color = get_color(ray);

    auto &p = framebuffer[y*width+x];
    p.first = p.first*(float)p.second/(float)(p.second+1) + color/(float)(p.second+1);
    ++p.second;
  }
  void render_once()
  {
    int num_threads = 8;
    struct worker_t
    {
      int ybegin, yend;
      World *world;
      int id;
      void operator()()
      {
        int p = 0;
        for( int y=ybegin; y<yend; ++y )
        {
          for( int x=0; x<world->width; ++x )
          {
            int i = (y-ybegin)*world->width + x;
            world->render_pixel(x,y);
          }
        }
      }
    };
    std::vector<std::thread> threads( num_threads );
    for( int t=0; t<num_threads; ++t )
    {
      int stride = height/num_threads;
      worker_t worker;
      worker.id = t;
      worker.world = this;
      worker.ybegin = t*stride;
      worker.yend = (t+1)*stride;
      if( t == num_threads-1 )
      {
        worker.yend = height;
      }
      threads[t] = std::thread( worker );
    }
    for( int t=0; t<num_threads; ++t )
    {
      threads[t].join();
    }
    /*
    for( int y=0; y<height; ++y )
    {
      for( int x=0; x<width; ++x )
      {
        render_pixel( x, y );
      }
    }
    */
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