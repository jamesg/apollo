#include "router.hpp"

#include <boost/bind.hpp>

#include "atlas/http/server/mimetypes.hpp"
#include "atlas/http/server/static_text.hpp"

#include "attachment.hpp"
#include "image_scaled.hpp"
#include "item_image.hpp"
#include "rest.hpp"

#define APOLLO_DECLARE_STATIC_STRING(PREFIX) \
    extern "C" { \
        extern char apollo_binary_##PREFIX##_start; \
        extern char apollo_binary_##PREFIX##_end; \
        extern size_t apollo_binary_##PREFIX##_size; \
    }

#define APOLLO_STATIC_STD_STRING(PREFIX) \
    std::string(&apollo_binary_##PREFIX##_start, &apollo_binary_##PREFIX##_end)

APOLLO_DECLARE_STATIC_STRING(index_html)
APOLLO_DECLARE_STATIC_STRING(manage_html)
APOLLO_DECLARE_STATIC_STRING(index_js)
APOLLO_DECLARE_STATIC_STRING(manage_js)
APOLLO_DECLARE_STATIC_STRING(application_js)
APOLLO_DECLARE_STATIC_STRING(models_js)
APOLLO_DECLARE_STATIC_STRING(style_css)

boost::shared_ptr<atlas::http::router> apollo::router(hades::connection& conn)
{
    boost::shared_ptr<atlas::http::router> out(new atlas::http::router);

    atlas::http::mimetypes mime_information;

    auto install_static_text = [&mime_information, out](
            const std::string& url,
            const std::string& text
            )
    {
        std::string extension;
        {
            std::string::size_type dot_pos = url.find_last_of('.');
            if(dot_pos != std::string::npos)
                extension = url.substr(dot_pos+1);
        }
        out->install(
                url,
                boost::bind(
                    &atlas::http::static_text,
                    mime_information.content_type(extension),
                    text,
                    _1,
                    _2,
                    _3,
                    _4
                    )
                );
    };

    // Special case; index.html should be served on requests to /, but as the
    // file extension cannot be deduced from the URL the MIME type must be
    // specified.
    out->install(
            "/",
            boost::bind(
                &atlas::http::static_text,
                mime_information.content_type("html"),
                APOLLO_STATIC_STD_STRING(index_html),
                _1,
                _2,
                _3,
                _4
                )
            );

    //
    // Install static files.
    //

    install_static_text("/index.html", APOLLO_STATIC_STD_STRING(index_html));
    install_static_text("/manage.html", APOLLO_STATIC_STD_STRING(manage_html));
    install_static_text("/index.js", APOLLO_STATIC_STD_STRING(index_js));
    install_static_text("/manage.js", APOLLO_STATIC_STD_STRING(manage_js));
    install_static_text("/application.js", APOLLO_STATIC_STD_STRING(application_js));
    install_static_text("/models.js", APOLLO_STATIC_STD_STRING(models_js));
    install_static_text("/style.css", APOLLO_STATIC_STD_STRING(style_css));

    boost::shared_ptr<atlas::http::router> rest_router(rest::router(conn));
    out->install(
            atlas::http::matcher("/api(.*)"),
            boost::bind(&atlas::http::router::serve, rest_router, _1, _2, _3, _4)
            );

    out->install(
        atlas::http::matcher("/item/([0-9]+)/image", "POST"),
        [&conn](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            const int item_id = boost::lexical_cast<int>(args[1]);
            upload_item_image(conn, mg_conn, item_id, success, failure);
        }
        );
    out->install(
        atlas::http::matcher("/attachment", "POST"),
        [&conn](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            upload_file(
                conn,
                mg_conn,
                "attachment_title",
                "attachment_data",
                [&conn, mg_conn, success](attachment a) {
                    insert_attachment(a, conn);

                    mg_send_status(mg_conn, 200);
                    mg_send_header(mg_conn, "Content-type", "text/json");

                    auto r = atlas::http::json_response(a.info);
                    mg_send_data(mg_conn, r.data.c_str(), r.data.length());

                    success();
                },
                failure
                );
        }
        );
    out->install(
        atlas::http::matcher("/attachment/([0-9]+)", "GET"),
        [&conn, &mime_information](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            const int attachment_id = boost::lexical_cast<int>(args[1]);
                //catch(const boost::bad_lexical_cast& e)
            return download_file(
                mime_information,
                conn,
                mg_conn,
                attachment_id,
                success,
                failure
                );
        }
        );
    out->install(
        atlas::http::matcher("/attachment/([0-9]+)/image/([0-9]+)x([0-9]+)", "GET"),
        [&conn, &mime_information](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            const int attachment_id = boost::lexical_cast<int>(args[1]);
            const int width = boost::lexical_cast<int>(args[2]);
            const int height = boost::lexical_cast<int>(args[3]);
            return image_scaled(
                mime_information,
                conn,
                mg_conn,
                attachment_id,
                width,
                height,
                success,
                failure
                );
        }
        );

    return out;
}

