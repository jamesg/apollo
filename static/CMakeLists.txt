atlas_add_objcopy_target(apollo index.html index_html.o)
atlas_add_objcopy_target(apollo index.js index_js.o)
atlas_add_objcopy_target(apollo manage.html manage_html.o)
atlas_add_objcopy_target(apollo manage.js manage_js.o)
atlas_add_objcopy_target(apollo application.js application_js.o)
atlas_add_objcopy_target(apollo models.js models_js.o)
atlas_add_objcopy_target(apollo style.css style_css.o)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    COMMAND ar -rc ${CMAKE_CURRENT_BINARY_DIR}/static.a
        ${CMAKE_CURRENT_BINARY_DIR}/index_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/index_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/models_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/style_css.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Archiving static HTML libraries to ${CMAKE_CURRENT_BINARY_DIR}/static.a"
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/index_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/index_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/models_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/style_css.o
    )

add_custom_target(apollo-static ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/static.a)

