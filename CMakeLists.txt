cmake_minimum_required(VERSION 3.5)

set( CMAKE_CXX_COMPILER /usr/bin/clang++ )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) # for clangd compile_commands.json

add_compile_definitions( TEAPOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/example/utah_teapot.stl" )

project( RayTrace CXX )
find_package( Eigen3 )
add_library( RayTrace STATIC
  src/reflection.cpp
)
target_link_libraries(
  RayTrace PUBLIC
  Eigen3::Eigen
)
target_include_directories(
  RayTrace PUBLIC
  ./src/
  ../RTree/include
)
set_target_properties(
  RayTrace PROPERTIES
  CXX_STANDARD 17
)


project( Runtime CXX )
find_package( SFML COMPONENTS system window graphics )
add_executable( Runtime
  example/runtime_sfml.cpp
  example/stl_loader.cpp
)
set_target_properties(
  Runtime PROPERTIES
  CXX_STANDARD 17
)
target_link_libraries(
  Runtime PUBLIC
  RayTrace
  sfml-system
  sfml-window
  sfml-graphics
)