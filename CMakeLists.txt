cmake_minimum_required(VERSION 3.5)

# for clangd
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_CXX_COMPILER /usr/bin/clang++ )

project( Sample CXX )
add_executable( Sample
  example/sample/sample.cpp
)
set_target_properties( Sample PROPERTIES
  CXX_STANDARD 17
)
target_include_directories( Sample PUBLIC
  ./include
)

project( test CXX )
find_package( GTest )
add_executable( test
  test/rtree.cpp
  test/main.cpp
)
set_target_properties( test PROPERTIES
  CXX_STANDARD 17
)
target_link_libraries( test PUBLIC GTest::gtest )
target_include_directories( test PUBLIC
  ./include
)

project( visualize_1d CXX )
add_executable( visualize_1d
  example/visualize_1d/main.cpp
)
set_target_properties( visualize_1d PROPERTIES
  CXX_STANDARD 17
)
target_include_directories( visualize_1d PUBLIC
  ./include
)

project( visualize_2d CXX )
add_executable( visualize_2d
  example/visualize_2d/main.cpp
)
set_target_properties( visualize_2d PROPERTIES
  CXX_STANDARD 17
)
target_include_directories( visualize_2d PUBLIC
  ./include
)

project( visualize_3d CXX )
add_executable( visualize_3d
  example/visualize_3d/main.cpp
)
set_target_properties( visualize_3d PROPERTIES
  CXX_STANDARD 17
)
target_include_directories( visualize_3d PUBLIC
  ./include
)

project( eigen CXX )
find_package( Eigen3 )
add_executable( eigen
  example/eigen/main.cpp
)
set_target_properties( eigen PROPERTIES
  CXX_STANDARD 17
)
target_include_directories( eigen PUBLIC
  ./include
)
target_link_libraries( eigen PUBLIC
  Eigen3::Eigen
)