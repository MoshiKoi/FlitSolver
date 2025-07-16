cmake_minimum_required(VERSION 3.28)
project(flit)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(FLITSOLVER_BUILD_TESTS "Build tests" OFF)

find_package(libassert CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)

if (FLITSOLVER_BUILD_TESTS)
    find_package(Catch2 3 REQUIRED CONFIG)
    include(CTest)
    include(Catch)
endif()

add_subdirectory(src)
