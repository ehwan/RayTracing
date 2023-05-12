#pragma once

#include "math.hpp"
#include <cmath>

namespace eh
{

class Eye
{
protected:
  vec3 axis_[3];
  vec3 position_;
  float aspect_ratio_;
  float tan_theta_;
  float near_;

public:
  Eye()
  {
    axis_[0] = vec3::UnitX();
    axis_[1] = vec3::UnitY();
    axis_[2] = vec3::UnitZ();
    position_.setZero();
  }

  vec3 const& axis( unsigned int i ) const
  {
    return axis_[ i ];
  }
  vec3 const& position() const
  {
    return position_;
  }
  void position( Eigen::Vector3f const& p )
  {
    position_ = p;
  }
  void move( unsigned int i , float factor )
  {
    position_ += axis_[ i ] * factor;
  }

  void set( Eigen::Vector3f const& x , Eigen::Vector3f const& y , Eigen::Vector3f const& z )
  {
    axis_[0] = x;
    axis_[1] = y;
    axis_[2] = z;
  }
  void look( Eigen::Vector3f const& to , Eigen::Vector3f const& up )
  {
    // z axis
    axis_[2] = -to;

    // x = y cross z
    axis_[0] = up.cross( axis_[2] ).normalized();

    // y = z cross x
    axis_[1] = axis_[2].cross( axis_[0] );
  }
  void look( Eigen::Vector3f const& to )
  {
    look( to , Eigen::Vector3f::UnitY() );
  }
  void perspective( float theta, float aspect_ratio, float near )
  {
    tan_theta_ = std::tan( theta*0.5f );
    aspect_ratio_ = aspect_ratio;
    near_ = near;
  }
  vec3 operator()( vec3 p ) const
  {
    return p.x()*axis_[0] + p.y()*axis_[1] + p.z()*axis_[2] + position_;
  }
  vec3 operator()( float i, float j ) const
  {
    float H = tan_theta_*near_;
    float W = H*aspect_ratio_;
    vec3 p = { W*(i-0.5f), -H*(j-0.5f), -near_ };
    return operator()(p);
  }
};
class EyeAngle
  : public Eye
{
protected:
  Eigen::Array3f angle_;

public:
  EyeAngle()
  {
    angle_.setZero();
  }

  Eigen::Array3f const& angle() const
  {
    return angle_;
  }
  void angle( Eigen::Array3f const& arr )
  {
    angle_.x() = std::min( 1.57079632679f , std::max( arr.x() , -1.57079632679f ) );
    angle_.y() = std::fmod( arr.y() , 6.28318530718f );
    angle_.z() = std::min( 3.14159265359f , std::max( arr.z() , -3.14159265359f ) );

    const Eigen::Array3f cos = angle_.cos();
    const Eigen::Array3f sin = angle_.sin();

    const Eigen::Vector3f axis1{
      cos.y()*cos.z() , cos.y()*sin.z() , -sin.y()
    };
    const Eigen::Vector3f axis3{
      cos.x()*sin.y() , -sin.x() , cos.x()*cos.y()
    };
    set( axis1 , axis3.cross( axis1 ) , axis3 );
  }
};

}