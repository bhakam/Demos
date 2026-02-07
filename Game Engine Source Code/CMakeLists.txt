cmake_minimum_required(VERSION 3.28)
project(TestGrounds)

set(CMAKE_CXX_STANDARD 26)
#set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "" FORCE)
#set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "" FORCE)


add_executable(TestGrounds main.cpp)

find_package(Boost CONFIG REQUIRED)
target_link_libraries(TestGrounds PRIVATE Boost::headers)

find_package(Eigen3 REQUIRED NO_MODULE)
target_link_libraries(TestGrounds PRIVATE Eigen3::Eigen)

find_package(OpenMP REQUIRED)
target_link_libraries(TestGrounds PRIVATE OpenMP::OpenMP_CXX)


find_package(SDL2 REQUIRED)
target_include_directories(TestGrounds PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(TestGrounds PRIVATE ${SDL2_LIBRARIES})



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer")

include(FetchContent)
