#include "reflection.hpp"
#include "global.hpp"
#include "world.hpp"
#include <tuple>

namespace eh
{
thread_local std::mt19937 World::mt_twister = std::mt19937{ std::random_device{}() };

vec3 MirrorReflection::get_color( Ray const& r, RayHit const& hit, World &w )
{
  Ray newray;
  newray.origin = hit.point(r);
  newray.direction = hit.reflect(r);
  newray.bounce = r.bounce + 0.3f;
  return w.get_color(newray).array() * hit.surface->get_color(hit.point(r)).array();
}
vec3 FuzzyMirrorReflection::get_color( Ray const& r, RayHit const& hit, World &w )
{
  vec3 color = vec3::Zero();
  vec3 reflection = hit.reflect(r);
  int sample_count = w.sample_count;
  for( int i=0; i<sample_count; ++i )
  {
    Ray newray;
    newray.origin = hit.point(r);
    newray.bounce = r.bounce + 1;
    float angle = std::sin( w.uniform_dist(w.mt_twister)*w.PI/2.0f )*w.PI/2.0f*fuzzyness;
    float angle2 = w.uniform_dist(w.mt_twister);
    float z = std::cos(angle);
    float x = std::cos(angle2)*std::sin(angle);
    float y = std::sin(angle2)*std::sin(angle);
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(reflection);
    newray.direction = x*unitx + y*unity + z*reflection;
    if( newray.direction.dot(hit.normal) < 0 ){ continue; }

    color += w.get_color(newray);
  }
  color = color/(float)sample_count;

  return color.array() * hit.surface->get_color(hit.point(r)).array();
}

vec3 DiffuseReflection::get_color( Ray const& r, RayHit const& hit, World &w )
{
  // diffusive random rays
  vec3 color = vec3::Zero();
  for( int i=0; i<w.sample_count; ++i )
  {
    Ray diffusive_ray;
    diffusive_ray.origin = hit.point(r);
    diffusive_ray.bounce = r.bounce + 1;

    float angle = std::sin( w.uniform_dist(w.mt_twister)*w.PI/2.0f )*w.PI/2.0f;
    float angle2 = w.uniform_dist(w.mt_twister);
    float z = std::cos(angle);
    float x = std::cos(angle2)*std::sin(angle);
    float y = std::sin(angle2)*std::sin(angle);
    vec3 unitx, unity;
    std::tie(unitx,unity) = make_unit(hit.normal);
    diffusive_ray.direction = x*unitx + y*unity + z*hit.normal;
    // diffusive_ray.direction = w.random_sphere() + r.normal;
    // diffusive_ray.direction.normalize();
    color += w.get_color(diffusive_ray);
  }
  color = color/(float)w.sample_count;
  return color.array() * hit.surface->get_color(hit.point(r)).array();
}
vec3 Refragtion::get_color( Ray const& r,RayHit const& hit, World &w )
{
  vec3 n = hit.normal*hit.normal.dot(r.direction);
  vec3 tangent = r.direction - n;
  float i = index;
  if( hit.normal.dot(r.direction) > 0 ){ i = 1.0f/i; }
  float a2 = index*index*n.squaredNorm();
  float a1 = 1.0f - index*index*tangent.squaredNorm();
  if( a1 <= 0 )
  {
    // full reflection
    Ray newray;
    newray.origin = hit.point(r);
    newray.direction = tangent - n;
    // newray.bounce = r.bounce;
    newray.bounce = r.bounce + 0.3f;
    vec3 color = w.get_color(newray);
    return color.array() * hit.surface->get_color(hit.point(r)).array();
  }
  else {
    // refraction
    float alpha = std::sqrt(a2/a1);
    Ray newray;
    newray.origin = hit.point(r);
    newray.direction = n + alpha * tangent;
    newray.direction.normalize();
    newray.bounce = r.bounce + 0.3f;
    vec3 color = w.get_color(newray);
    return color.array() * hit.surface->get_color(hit.point(r)).array();
  }
}

vec3 CombineReflection::get_color( Ray const& r, RayHit const& hit, World &w )
{
  vec3 c1 = r1->get_color(r,hit,w);
  vec3 c2 = r2->get_color(r,hit,w);
  return c1*s1 + c2*s2;
}
vec3 FaceReflection::get_color( Ray const& r, RayHit const& hit, World &w )
{
  if( hit.normal.dot(r.direction) < 0 )
  {
    return front->get_color(r,hit,w);
  }else {
    return back->get_color(r,hit,w);
  }
}

vec3 DirectionalLightSource::get_color( Ray const& r, RayHit const& hit, World &w )
{
  return hit.surface->get_color(hit.point(r));
}


}
