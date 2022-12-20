cmake_minimum_required(VERSION 2.6.4)

project(CFS_Telescope CXX)


set( sources entrypoint.cpp
             src/TelescopeApp.cpp
             src/Telescope.cpp
             h/TelescopeApp.hpp
             h/Telescope.hpp
              )

# Create the app module
add_cfe_app(telescope ${sources})

target_link_libraries( telescope PUBLIC messages cpp_base )

target_include_directories( telescope PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/h)

target_compile_options( telescope PRIVATE -fno-stack-protector )
