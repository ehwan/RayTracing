#include "reflection.hpp"
#include "global.hpp"
#include "world.hpp"
#include <tuple>

namespace eh
{

vec3 MirrorReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  Ray newray = r.reflect();
  newray.bounce = r.bounce + 1;
  return w.get_color(newray);
}
vec3 FuzzyMirrorReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  vec3 color = vec3::Zero();
  int sample_count = w.sample_count;
  for( int i=0; i<sample_count; ++i )
  {
    Ray newray;
    newray.direction = r.normal + fuzzyness*w.random_sphere();
    if( newray.direction.dot(r.normal) < 0 ){ continue; }
    newray.direction.normalize();
    newray.bounce = r.bounce + 1;
    newray.origin = r.point();

    color += w.get_color(newray);
  }
  color = color/(float)sample_count;

  return color.array()*g.get_color(r.point()).array();
}

vec3 DiffuseReflection::get_color( Ray &r, World &w, GeometryObject &g )
{
  // diffusive random rays
  vec3 color = vec3::Zero();
  for( int i=0; i<w.sample_count; ++i )
  {
    Ray diffusive_ray;
    diffusive_ray.origin = r.point();
    diffusive_ray.direction = w.random_sphere() + r.normal;
    diffusive_ray.direction.normalize();
    diffusive_ray.bounce = r.bounce + 1;

    color += w.get_color(diffusive_ray);
  }
  color = color/(float)w.sample_count;
  return color.array() * g.get_color(r.point()).array();
}

vec3 DirectionalLightSource::get_color( Ray &r, World &w, GeometryObject &g )
{
  float coeff = std::abs(r.direction.dot( r.normal ));
  return coeff * source_color;
  // return source_color;
}


}