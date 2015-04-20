cmake_minimum_required(VERSION 2.6)

file(GLOB server_sources *.cpp)
add_library(apollo-server-lib ${server_sources})
target_link_libraries(
    apollo-server-lib
    atlas-com-db
    atlas-com-http-server
    hades
    ${ImageMagick_LIBRARIES}
    "exiv2"
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    )

