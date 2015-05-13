atlas_add_web_library(
    PREFIX apollo
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    TARGET apollo-static
    SOURCES
        index.html
        index.js
        manage.html
        manage.js
        application.js
        models.js
        style.css
        )

