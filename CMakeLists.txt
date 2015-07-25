cmake_minimum_required(VERSION 2.6)

project("Apollo")
enable_testing()

find_package(CURL)
find_package(Threads)
find_package(OpenSSL)
find_package(Boost COMPONENTS date_time filesystem random regex system thread)
find_package(ImageMagick COMPONENTS Magick++)

include_directories("src")
include_directories("include")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ImageMagick_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

add_definitions("-std=c++11")

# Apollo components
add_subdirectory(src)

# Web components
add_subdirectory(web)

add_executable(apollo src/main/apollo.cpp)
add_dependencies(apollo-server-lib apollo-static)
target_link_libraries(
    apollo
    apollo-server-lib
    )
