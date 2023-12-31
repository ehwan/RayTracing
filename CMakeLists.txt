cmake_minimum_required(VERSION 3.5)

set( CMAKE_CXX_COMPILER /usr/bin/clang++ )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) # for clangd compile_commands.json

find_package( SFML COMPONENTS system window graphics )
find_package( Eigen )
project( RayTrace CXX )
add_executable( RayTrace
  reflection.cpp
  lodepng.cpp
  main.cpp
)
set_target_properties(
  RayTrace PROPERTIES
  CXX_STANDARD 17
)
target_link_libraries(
  RayTrace PUBLIC
  sfml-system
  sfml-window
  sfml-graphics
)