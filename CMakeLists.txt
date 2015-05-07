cmake_minimum_required(VERSION 2.6)

project("Apollo")
enable_testing()

find_package(CURL)
find_package(Threads)
find_package(OpenSSL)
find_package(Boost COMPONENTS date_time filesystem random regex system thread)
find_package(ImageMagick COMPONENTS Magick++)

include_directories("atlas/lib/mongoose/include")
include_directories("atlas/include")
include_directories("src")
include_directories("commandline/include")
include_directories("hades/include")
include_directories("hades/lib/sqlite3/include")
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ImageMagick_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

add_definitions("-std=c++11")

# Hades components
add_subdirectory(hades)

# Atlas components
add_subdirectory(atlas)

# Apollo components
add_subdirectory(src)

# Web components
add_subdirectory(static)

add_executable(apollo src/main/apollo.cpp)
add_dependencies(apollo apollo-static)
target_link_libraries(
    apollo
    apollo-server-lib
    )

