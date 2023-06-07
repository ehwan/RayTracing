// compile flags
// -lsfml-window -lsfml-system -lsfml-graphics -O2
#include "global.hpp"
#include "reflection.hpp"
#include "world.hpp"
#include "lodepng.h"
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include <thread>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

// image size
int WIDTH = 400;
int HEIGHT = 400;

struct BaseDemoWorld
  : eh::World
{
  eh::SkySphere skysphere;
  eh::DirectionalLightSource sky_source;

  eh::Plane chekered_floor;
  eh::DiffuseReflection floor_reflect;

  eh::Sphere s1, s2, s3;
  eh::DiffuseReflection s1_reflect;
  eh::Refragtion s2_reflect;
  // eh::DiffuseReflection s2_reflect;
  eh::MirrorReflection s3_reflect;

  eh::Triangle triangle;
  eh::MirrorReflection triangle_reflect;

  BaseDemoWorld()
    : eh::World()
  {
    this->resize( WIDTH, HEIGHT );
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


sf::Texture texture;
sf::Sprite sprite;

std::thread render_thread;

int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 1000;
sf::RenderWindow window;

float dt = 0;

BaseDemoWorld world;

// move with WASDRF keys
bool move()
{
  {
    float spd = 2.0f*dt;
    float anglespd = 1.0f*dt;
    if( sf::Keyboard::isKeyPressed( sf::Keyboard::S ) )
    {
      if( sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
      {
        auto angle = world.camera.angle();
        angle.x() -= anglespd;
        world.camera.angle(angle);
        return true;
      }else
      {
        world.camera.move(2,spd);
        return true;
      }
    }else if( sf::Keyboard::isKeyPressed( sf::Keyboard::W ) )
    {
      if( sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
      {
        auto angle = world.camera.angle();
        angle.x() += anglespd;
        world.camera.angle(angle);
        return true;
      }else
      {
        world.camera.move(2,-spd);
        return true;
      }
    }else if( sf::Keyboard::isKeyPressed( sf::Keyboard::A ) )
    {
      if( sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
      {
        auto angle = world.camera.angle();
        angle.y() += anglespd;
        world.camera.angle(angle);
        return true;
      }else
      {
        world.camera.move(0,-spd);
        return true;
      }
    }else if( sf::Keyboard::isKeyPressed( sf::Keyboard::D ) )
    {
      if( sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
      {
        auto angle = world.camera.angle();
        angle.y() -= anglespd;
        world.camera.angle(angle);
        return true;
      }else
      {
        world.camera.move(0,+spd);
        return true;
      }
    }else if( sf::Keyboard::isKeyPressed( sf::Keyboard::R ) )
    {
      world.camera.move(1,spd);
      return true;
    }else if( sf::Keyboard::isKeyPressed( sf::Keyboard::F ) )
    {
      world.camera.move(1,-spd);
      return true;
    }
    return false;
  }
}

int main()
{
  bool rendering_loop = true;
  // rendering thread
  render_thread = std::thread(
      [&]()
      {
        while( rendering_loop )
        {
          world.render_once_balance();
        }
      }
  );

  window.create( sf::VideoMode(WINDOW_WIDTH,WINDOW_HEIGHT), "Hello RayTracing" );

  texture.create( WIDTH, HEIGHT );
  //texture.setSmooth( true );
  sprite.setTexture( texture );
  sprite.setPosition( 0, 0 );
  sprite.setScale( (float)WINDOW_WIDTH/(float)WIDTH, (float)WINDOW_HEIGHT/(float)HEIGHT );

  auto t0 = std::chrono::system_clock::now();
  while( window.isOpen() )
  {
    sf::Event event;
    while( window.pollEvent(event) )
    {
      if( event.type == sf::Event::Closed )
      {
        window.close();
      }
    }
    auto t1 = std::chrono::system_clock::now();
    dt = std::chrono::duration_cast<
      std::chrono::duration<float,std::ratio<1,1>>
    >( t1 - t0 ).count();
    t0 = t1;

    // if not move, accumulate to current framebuffer
    if( move() )
    {
      world.clear_framebuffer();
    }
    window.clear();

    auto fb = world.get_imagebuffer(true);
    texture.update( fb.data() );
    window.draw( sprite );
    window.display();
  }

  rendering_loop = false;
  render_thread.join();

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
