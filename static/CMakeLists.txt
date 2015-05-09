atlas_add_web_library(
    PREFIX apollo
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    SOURCES
        index.html
        index.js
        manage.html
        manage.js
        application.js
        models.js
        style.css
        )

add_custom_target(apollo-static ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/static.a)

