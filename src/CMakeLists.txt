file(GLOB server_sources *.cpp)
add_library(apollo-server-lib ${server_sources})
target_link_libraries(
    apollo-server-lib
    atlas-com-db
    atlas-com-http-server
    atlas-web
    hades
    ${CMAKE_CURRENT_BINARY_DIR}/../web/static.a
    ${ImageMagick_LIBRARIES}
    "exiv2"
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    )

