#pragma once

#include <eigen3/Eigen/Dense>
#include <utility>
namespace eh {
  using vec2 = Eigen::Vector2f;
  using vec3 = Eigen::Vector3f;
  using vec4 = Eigen::Vector4f;

  using vec2i = Eigen::Vector2i;
  using vec3i = Eigen::Vector3i;
  using vec4i = Eigen::Vector4i;

  inline vec3i vec3_to_color( vec3 c )
  {
    return (c.array().min(1.0f) * vec3(255.99f,255.99f,255.99f).array()).cast<int>().matrix();
  }
  inline std::pair<vec3,vec3> make_unit( vec3 unitz )
  {
    vec3 unity = unitz.cross(vec3::Random());
    unity.normalize();
    vec3 unitx = unity.cross(unitz);
    return { unitx, unity };
  }
}