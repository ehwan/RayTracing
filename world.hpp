#pragma once

#include "global.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "geometry.hpp"
#include "reflection.hpp"
#include "camera.hpp"

#include <limits>
#include <thread>
#include <vector>
#include <random>
#include <utility>
#include <chrono>
#include <iostream>

namespace eh {

struct World
{
  constexpr static float PI = 3.141592f;

  // random generators
  std::mt19937 mt_twister{ std::random_device{}() };
  std::uniform_real_distribution<float> uniform_dist{ 0.0f, 1.0f };

  std::vector<GeometryObject*> objects;

  int max_bounce = 2;
  int sample_count = 10;

  std::vector<vec3> framebuffer;
  std::vector<int> render_count;
  std::vector<float> render_time;
  int width = 100;
  int height = 100;

  EyeAngle camera;

  World( int width_=100, int height_=100 )
  {
    resize( width_, height_ );
  }
  void resize( int width_, int height_ )
  {
    width = width_;
    height = height_;
    clear_framebuffer();
  }
  void clear_framebuffer()
  {
    framebuffer.resize( width*height, vec3::Zero() );
    render_count.resize( width*height, 0 );
    render_time.resize( width*height, 0.1f );
  }

  // perform raycasting
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

  // make random point on unit sphere
  vec3 random_sphere()
  {
    // angle around y axis
    const float angle1 = uniform_dist(mt_twister)*2*PI;
    // angle above xz plane
    const float angle2 = (uniform_dist(mt_twister)-0.5)*PI;

    const float c = std::cos(angle1);
    return { std::cos(angle2)*c, std::sin(angle1), std::sin(angle2)*c };
  }

  // make random point on unit semi-sphere on xy plane (postive)
  vec3 random_semisphere()
  {
    // angle around z axis
    const float angle1 = uniform_dist(mt_twister)*2*PI;
    // angle above xy plane
    const float angle2 = uniform_dist(mt_twister)*0.5f*PI;

    const float c = std::cos(angle1);
    return { std::cos(angle2)*c, std::sin(angle2)*c, std::sin(angle1) };
  }

  // raytrace and get color from this ray
  vec3 get_color( Ray &r )
  {
    if( r.bounce >= max_bounce ){ return vec3::Zero(); }

    raycast( r );
    if( r.surface == nullptr ){ return vec3::Zero(); }
    return r.surface->reflection->get_color(r,*this,*r.surface);
  }

  using clock_type = std::chrono::high_resolution_clock;

  // calculate color for one pixel (x,y)
  // calculation time will be averaged and stored into render_time
  // for balanced multi-thread
  void render_pixel( int x, int y )
  {
    auto t0 = clock_type::now();
    vec2 rf = vec2::Random();
    float xf = (x + rf(0))/(float)width;
    float yf = (y + rf(1))/(float)height;
    vec3 point = camera(xf,yf);
    Ray ray;
    ray.origin = point;
    ray.direction = point - camera(vec3(0,0,0));
    ray.direction.normalize();
    auto color = get_color(ray);

    auto& rendercount = render_count[y*width+x];
    auto& rendertime = render_time[y*width+x];
    auto& renderpixel = framebuffer[y*width+x];

    auto t1 = clock_type::now();
    auto dur = std::chrono::duration_cast< std::chrono::duration<float,std::ratio<1,1000>> >( t1 - t0 ).count();

    // average new color data to old one
    renderpixel = renderpixel*( (float)rendercount/(float)(rendercount+1) ) + color/(float)(rendercount+1);

    // average calculation time
    rendertime = (rendertime*rendercount + dur)/(float)(rendercount + 1);

    ++rendercount;
  }


  // balanced multi-thread
  // each thread would take balanced-size work based on *render_time*
  void render_once_balance( int num_threads=8 )
  {
    auto t0 = clock_type::now();

    std::cout << "Render to Framebuffer Start\n";

    // function object for thread
    struct worker_t
    {
      World *world;
      int id;

      int begin, end;

      void operator()()
      {
        for( ; begin<end; ++begin )
        {
          world->render_pixel( begin%world->width, begin/world->height );
        }
      }
    };

    // prefix-sum render_time
    std::vector<float> rendertime_sum( width*height+1 );
    rendertime_sum[0] = 0;
    for( int i=1; i<rendertime_sum.size(); ++i )
    {
      rendertime_sum[i] = rendertime_sum[i-1] + render_time[i];
    }

    const float time_per_thread = rendertime_sum[height*width]/num_threads;
    std::cout << "Total Calculation Time Predict : " << rendertime_sum[height*width] << "\n";

    std::vector<std::thread> threads;
    int begin = 0;
    while( begin < width*height )
    {
      float time_for_this_thread;
      int end;
      for( end=begin; end<width*height; ++end )
      {
        time_for_this_thread = rendertime_sum[end] - rendertime_sum[begin];
        if( time_for_this_thread > time_per_thread )
        {
          break;
        }
      }
      worker_t worker;
      worker.id = threads.size();
      worker.world = this;
      std::cout << "Time for Thread" << worker.id << ": " << time_for_this_thread << "\n";
      std::cout << "(" << begin << ") -> (" << end << ")\n";

      worker.begin = begin;
      worker.end = end;
      threads.emplace_back( std::thread(worker) );

      begin = end;
    }

    for( auto &t : threads )
    {
      t.join();
    }

    auto dur = 
      std::chrono::duration_cast< std::chrono::milliseconds >( clock_type::now() - t0 ).count();
    std::cout << "Total Render Time : " << dur << "\n";
    std::cout << "Render End\n\n";
  }
  void render_once()
  {
    auto t0 = clock_type::now();
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

    auto dur = 
      std::chrono::duration_cast< std::chrono::milliseconds >( clock_type::now() - t0 ).count();
    std::cout << "Total Render Time : " << dur << "\n";
    std::cout << "Render End\n\n";

  }

  std::vector<unsigned char> get_imagebuffer()
  {
    std::vector<unsigned char> ret( width*height*3 );
    for( int y=0; y<height; ++y )
    {
      for( int x=0; x<width; ++x )
      {
        auto vi = vec3_to_color(framebuffer[y*width+x]);
        ret[(y*width+x)*3 + 0] = (unsigned char)vi(0);
        ret[(y*width+x)*3 + 1] = (unsigned char)vi(1);
        ret[(y*width+x)*3 + 2] = (unsigned char)vi(2);
      }
    }
    return ret;
  }
};

}
