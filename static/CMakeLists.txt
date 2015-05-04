
macro(apollo_add_objcopy_target filename object)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${object}
        COMMAND objcopy --prefix-symbols=apollo --input binary --output elf64-x86-64 --binary-architecture i386 ${filename} ${CMAKE_CURRENT_BINARY_DIR}/${object}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${filename}
        COMMENT "Generating ${object} from ${filename}" VERBATIM
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
endmacro(apollo_add_objcopy_target)

apollo_add_objcopy_target(index.html index_html.o)
apollo_add_objcopy_target(index.js index_js.o)
apollo_add_objcopy_target(manage.html manage_html.o)
apollo_add_objcopy_target(manage.js manage_js.o)
apollo_add_objcopy_target(backbone.js backbone_js.o)
apollo_add_objcopy_target(backbone-min.js backbone-min_js.o)
apollo_add_objcopy_target(underscore.js underscore_js.o)
apollo_add_objcopy_target(underscore-min.js underscore-min_js.o)
apollo_add_objcopy_target(jquery.js jquery_js.o)
apollo_add_objcopy_target(application.js application_js.o)
apollo_add_objcopy_target(stacked_application.js stacked_application_js.o)
apollo_add_objcopy_target(models.js models_js.o)
apollo_add_objcopy_target(style.css style_css.o)
apollo_add_objcopy_target(
    open-iconic/font/css/open-iconic.css
    open_iconic_font_css_open_iconic_css.o)
apollo_add_objcopy_target(
    open-iconic/font/fonts/open-iconic.ttf
    open_iconic_font_fonts_open_iconic_ttf.o)
apollo_add_objcopy_target(
    open-iconic/font/fonts/open-iconic.woff
    open_iconic_font_fonts_open_iconic_woff.o)
apollo_add_objcopy_target(
    ShareTechMono-Regular.ttf
    ShareTechMono_Regular_ttf.o)
apollo_add_objcopy_target(
    ShareTech-Regular.ttf
    ShareTech_Regular_ttf.o)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static.a
    COMMAND ar -qc ${CMAKE_CURRENT_BINARY_DIR}/static.a
        ${CMAKE_CURRENT_BINARY_DIR}/index_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/index_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/backbone_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/backbone-min_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/underscore_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/underscore-min_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/jquery_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/stacked_application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/models_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/style_css.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_css_open_iconic_css.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_fonts_open_iconic_ttf.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_fonts_open_iconic_woff.o
        ${CMAKE_CURRENT_BINARY_DIR}/ShareTechMono_Regular_ttf.o
        ${CMAKE_CURRENT_BINARY_DIR}/ShareTech_Regular_ttf.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Archiving static HTML libraries to ${CMAKE_CURRENT_BINARY_DIR}/static.a"
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/index_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/index_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_html.o
        ${CMAKE_CURRENT_BINARY_DIR}/manage_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/backbone_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/backbone-min_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/underscore_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/underscore-min_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/jquery_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/stacked_application_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/models_js.o
        ${CMAKE_CURRENT_BINARY_DIR}/style_css.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_css_open_iconic_css.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_fonts_open_iconic_ttf.o
        ${CMAKE_CURRENT_BINARY_DIR}/open_iconic_font_fonts_open_iconic_woff.o
        ${CMAKE_CURRENT_BINARY_DIR}/ShareTechMono_Regular_ttf.o
        ${CMAKE_CURRENT_BINARY_DIR}/ShareTech_Regular_ttf.o
    )

add_custom_target(apollo-static ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/static.a)

#add_library(apollo-static)
#target_link_libraries(
    #apollo-static
    #${CMAKE_CURRENT_BINARY_DIR}/index.o
    #)

