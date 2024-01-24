#pragma once

#include "global.hpp"
#include "math.hpp"
#include "ray.hpp"
#include "geometry.hpp"
#include "reflection.hpp"
#include "camera.hpp"

#include <algorithm>
#include <limits>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>

#include "rtree_adapt.hpp"

namespace eh {

struct Object
{
  GeometryObject *geometry;
  ReflectionModel *reflect;
};

/*
  World is a container of objects and camera
  and also a raytracer
*/
class World
{
public:
  using rtree_type = eh::rtree::RTree<BoundingBox,BoundingBox,Object,4,8>;

  rtree_type objects;

  constexpr static float PI = 3.141592f;

  // maximum number of recursive raytracing
  float max_bounce = 2;

  // framebuffer size
  int width = 100;
  int height = 100;

  std::vector<vec3> framebuffer;
  // average calculate time for each pixel
  std::vector<float> calculation_time, calculation_time_prefixsum;
  int sample_count = 0;
  int shoot_count = 4;

  // per thread objects
  struct per_thread_t
  {
    std::mt19937 mt_twister;
    // pixels range
    int begin, end;

    std::thread thread;

    float calculation_time;
  };
  std::vector<per_thread_t> per_threads;

  std::uniform_real_distribution<float> uniform_dist{ 0.0f, 1.0f };

  EyeAngle camera;

  void insert( Object obj )
  {
    objects.insert( { obj.geometry->bounding_box(), obj } );
  }
  float random01( int thread_id )
  {
    return uniform_dist( per_threads[thread_id].mt_twister );
  }
  void init( int width_, int height_, int thread_num )
  {
    width = width_;
    height = height_;
    framebuffer.resize( width*height );
    calculation_time.resize( width*height );
    calculation_time_prefixsum.resize( width*height+1 );

    per_threads.resize( thread_num );
    // random generator for seeds...
    std::mt19937 mt_twister{ std::random_device{}() };
    using seed_type = decltype( std::random_device{}() );
    std::uniform_int_distribution<seed_type> seed_distribution{
      std::numeric_limits<seed_type>::min(),
      std::numeric_limits<seed_type>::max()
    };
    const int per_thread_pixels = width*height/thread_num;
    for( int i=0; i<thread_num; ++i )
    {
      per_threads[i].mt_twister = std::mt19937{ seed_distribution(mt_twister) };
      per_threads[i].begin = i*per_thread_pixels;
      per_threads[i].end = (i+1)*per_thread_pixels;
      if( i == thread_num-1 )
      {
        per_threads[i].end = width*height;
      }
    }

    clear_framebuffer();
  }

  void rebalance_thread_range()
  {
    calculation_time_prefixsum[0] = 0;
    for( int i=0; i<width*height; ++i )
    {
      calculation_time_prefixsum[i+1] = calculation_time_prefixsum[i] + calculation_time[i];
    }
    float balanced_time = calculation_time_prefixsum[width*height]/per_threads.size();
    int begin = 0;
    for( int i=0; i<per_threads.size()-1; ++i )
    {
      int balanced_point = 
        std::upper_bound( 
          calculation_time_prefixsum.begin()+begin, 
          calculation_time_prefixsum.end(), 
          balanced_time*(i+1)
        ) - calculation_time_prefixsum.begin();
      per_threads[i].begin = begin;
      per_threads[i].end = balanced_point;
      begin = balanced_point;
    }
    per_threads.back().begin = begin;
    per_threads.back().end = width*height;
  }

  void clear_framebuffer()
  {
    sample_count = 0;
  }

  // raycasting in rtree dfs wrapper
  void rtree_raycast_wrapper( Ray const& ray, rtree_type::node_type *node, int leaf_level, RayHit &cur )
  {
    if( leaf_level == 0 )
    {
      // node is leaf node
      for( auto &c : *node->as_leaf() )
      {
        float tmin, tmax;
        if( c.first.raycast(ray, tmin, tmax) == false ){ continue; }
        if( cur.t < tmin ){ continue; }
        auto raycast_result = c.second.geometry->raycast( ray );
        if( raycast_result.surface && raycast_result.t < cur.t )
        {
          cur = raycast_result;
          cur.surface = &c.second;
        }
      }
    }else {
      for( auto &c : *node )
      {
        float tmin, tmax;
        if( c.first.raycast(ray, tmin, tmax) == false ){ continue; }
        if( cur.t < tmin ){ continue; }
        rtree_raycast_wrapper( ray, c.second->as_node(), leaf_level-1, cur );
      }
    }
  }

  // perform raycasting
  RayHit raycast( Ray const& ray )
  {
    RayHit ret;
    ret.t = std::numeric_limits<float>::max();
    rtree_raycast_wrapper( ray, objects.root()->as_node(), objects.leaf_level(), ret );
    return ret;
  }

  // raytrace and get color from this ray
  vec3 get_color( Ray const& r )
  {
    // stop recursive-raytracing
    if( r.bounce >= max_bounce ){ return vec3::Zero(); }

    auto hit = raycast( r );
    if( hit.surface == nullptr ){ return vec3::Zero(); }

    return hit.surface->reflect->get_color( r, hit, *this );
  }

  using clock_type = std::chrono::high_resolution_clock;

  // calculate color for one pixel (x,y)
  // calculation time will be averaged and stored into render_time
  // for balanced multi-thread
  void render_pixel( int x, int y, int thread_id )
  {
    auto t0 = clock_type::now();

    vec3 color = vec3::Zero();
    for( int k=0; k<shoot_count; ++k )
    {
      float xf = (x + random01(thread_id))/(float)width;
      float yf = (y + random01(thread_id))/(float)height;
      vec3 point = camera(xf,yf);
      Ray ray( point, (point - camera(vec3(0,0,0))).normalized(), thread_id );
      color += get_color(ray);
    }
    color /= shoot_count;



    auto& rendertime = calculation_time[y*width+x];
    vec3& renderpixel = framebuffer[y*width+x];

    auto t1 = clock_type::now();
    auto dur = std::chrono::duration_cast< std::chrono::duration<float,std::ratio<1,1000>> >( t1 - t0 ).count();

    if( sample_count == 0 )
    {
      rendertime = 0;
      renderpixel = vec3::Zero();
    }

    // average new color data to old one
    renderpixel = renderpixel*( (float)sample_count/(float)(sample_count+1) ) + color/(float)(sample_count+1);
    // average calculation time
    rendertime = (rendertime*sample_count+ dur)/(float)(sample_count + 1);
  }


  // balanced multi-thread
  // each thread would take balanced-size work based on *render_time*
  void render()
  {
    auto t0 = clock_type::now();

    std::cout << "Render to Framebuffer Start\n";
    for( int i=0; i<per_threads.size(); ++i )
    {
      std::cout << "Thread" << i << ": ";
      std::cout << "(" << per_threads[i].begin << "->" << per_threads[i].end << ")";
      std::cout << ", " << per_threads[i].end - per_threads[i].begin << " pixels\n";
      per_threads[i].thread = std::thread(
        []( World *world, int thread_id )
        {
          auto t0 = clock_type::now();
          for( int i=world->per_threads[thread_id].begin; i<world->per_threads[thread_id].end; ++i )
          {
            world->render_pixel( i%world->width, i/world->width, thread_id );
          }
          auto dur = std::chrono::duration_cast< std::chrono::milliseconds >( clock_type::now() - t0 ).count();
          world->per_threads[thread_id].calculation_time = dur;
        },
        this, i
      );
    }
    for( int i=0; i<per_threads.size(); ++i )
    {
      per_threads[i].thread.join();
      std::cout << "Thread" << i << " CalcTime: ";
      std::cout << per_threads[i].calculation_time << "\n";
    }

    auto dur = 
      std::chrono::duration_cast< std::chrono::milliseconds >( clock_type::now() - t0 ).count();
    std::cout << "Total Render Time : " << dur << "\n";
    std::cout << "Render End\n\n";

    if( sample_count < 1000000 )
    {
      ++sample_count;
    }
  }

  std::vector<unsigned char> get_imagebuffer( bool alpha=false )
  {
    std::vector<unsigned char> ret( alpha ? width*height*4 : width*height*3 );
    for( int i=0; i<framebuffer.size(); ++i )
    {
      auto vi = vec3_to_color( framebuffer[i] );
      if( alpha )
      {
        ret[ 4*i + 0 ] = (unsigned char)vi(0);
        ret[ 4*i + 1 ] = (unsigned char)vi(1);
        ret[ 4*i + 2 ] = (unsigned char)vi(2);
        ret[ 4*i + 3 ] = (unsigned char)255;
      }else
      {
        ret[ 3*i + 0 ] = (unsigned char)vi(0);
        ret[ 3*i + 1 ] = (unsigned char)vi(1);
        ret[ 3*i + 2 ] = (unsigned char)vi(2);
      }
    }
    return ret;
  }
};


}
