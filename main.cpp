// compile flags
// -lsfml-window -lsfml-system -lsfml-graphics -O2
#include "global.hpp"
#include "reflection.hpp"
#include "world.hpp"
#include "lodepng.h"
#include <iostream>
#include <vector>

#include <thread>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "demo.hpp"

// image size
int WIDTH = 400;
int HEIGHT = 400;


sf::Texture texture;
sf::Sprite sprite;

std::thread render_thread;

int WINDOW_WIDTH = 1000;
int WINDOW_HEIGHT = 1000;
sf::RenderWindow window;

float dt = 0;

SphereDemo world( WIDTH, HEIGHT );
//MarchingCubesDemoWorld world(WIDTH,HEIGHT);


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


