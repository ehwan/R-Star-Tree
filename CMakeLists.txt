cmake_minimum_required(VERSION 3.5)

# for clangd
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_C_COMPILER /usr/bin/clang )
set( CMAKE_CXX_COMPILER /usr/bin/clang++ )

project( test CXX )
find_package( GTest )
add_executable( test
  test/test.cpp
)
set_target_properties( test PROPERTIES
  CXX_STANDARD 17
)
target_link_libraries( test PUBLIC GTest::gtest )
target_include_directories( test PUBLIC
  ./include
)
