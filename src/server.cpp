#include "server.hpp"

#include "hades/connection.hpp"
#include "hades/mkstr.hpp"

//#include "atlas/api/auth.hpp"
#include "atlas/http/server/router.hpp"
#include "atlas/http/server/install_static_file.hpp"
#include "atlas/http/server/static_file.hpp"
#include "atlas/http/server/static_text.hpp"
#include "atlas/jsonrpc/uri.hpp"
#include "atlas/log/log.hpp"

#include "attachment.hpp"
#include "db.hpp"
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
APOLLO_DECLARE_STATIC_STRING(backbone_js)
APOLLO_DECLARE_STATIC_STRING(underscore_js)
APOLLO_DECLARE_STATIC_STRING(backbone_min_js)
APOLLO_DECLARE_STATIC_STRING(underscore_js)
APOLLO_DECLARE_STATIC_STRING(underscore_min_js)
APOLLO_DECLARE_STATIC_STRING(jquery_js)
APOLLO_DECLARE_STATIC_STRING(application_js)
APOLLO_DECLARE_STATIC_STRING(stacked_application_js)
APOLLO_DECLARE_STATIC_STRING(models_js)
APOLLO_DECLARE_STATIC_STRING(style_css)
APOLLO_DECLARE_STATIC_STRING(open_iconic_font_css_open_iconic_css)
APOLLO_DECLARE_STATIC_STRING(open_iconic_font_fonts_open_iconic_ttf)
APOLLO_DECLARE_STATIC_STRING(open_iconic_font_fonts_open_iconic_woff)
APOLLO_DECLARE_STATIC_STRING(ShareTechMono_Regular_ttf)
APOLLO_DECLARE_STATIC_STRING(ShareTech_Regular_ttf)

apollo::server::server(
        const server::options& options,
        boost::shared_ptr<boost::asio::io_service> io_
        ) :
    m_io(io_),
    m_http_server(
        m_io,
        options.address.c_str(),
        options.port.c_str()
        ),
    m_mime_information(new atlas::http::mimetypes())
{
    if(!options.port.length())
        throw std::runtime_error("port number is required");
    if(!options.db_file.length())
        throw std::runtime_error("database file is required");

    atlas::log::information("apollo::server::server") << "opening db file " <<
        options.db_file;
    m_connection.reset(new hades::connection(options.db_file));
    db::create(*m_connection);

    auto install_static_text = [this](
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
        m_http_server.router().install(
                url,
                boost::bind(
                    &atlas::http::static_text,
                    m_mime_information->content_type(extension),
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
    m_http_server.router().install(
            "/",
            boost::bind(
                &atlas::http::static_text,
                m_mime_information->content_type("html"),
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
    install_static_text("/backbone.js", APOLLO_STATIC_STD_STRING(backbone_js));
    install_static_text("/backbone-min.js", APOLLO_STATIC_STD_STRING(backbone_min_js));
    install_static_text("/underscore.js", APOLLO_STATIC_STD_STRING(underscore_js));
    install_static_text("/underscore-min.js", APOLLO_STATIC_STD_STRING(underscore_min_js));
    install_static_text("/jquery.js", APOLLO_STATIC_STD_STRING(jquery_js));
    install_static_text("/application.js", APOLLO_STATIC_STD_STRING(application_js));
    install_static_text("/stacked_application.js", APOLLO_STATIC_STD_STRING(stacked_application_js));
    install_static_text("/models.js", APOLLO_STATIC_STD_STRING(models_js));
    install_static_text("/style.css", APOLLO_STATIC_STD_STRING(style_css));
    install_static_text("/open-iconic/font/css/open-iconic.css", APOLLO_STATIC_STD_STRING(open_iconic_font_css_open_iconic_css));
    install_static_text("/open-iconic/font/fonts/open-iconic.ttf", APOLLO_STATIC_STD_STRING(open_iconic_font_fonts_open_iconic_ttf));
    install_static_text("/open-iconic/font/fonts/open-iconic.woff", APOLLO_STATIC_STD_STRING(open_iconic_font_fonts_open_iconic_woff));
    install_static_text("/ShareTechMono-Regular.ttf", APOLLO_STATIC_STD_STRING(ShareTechMono_Regular_ttf));
    install_static_text("/ShareTech-Regular.ttf", APOLLO_STATIC_STD_STRING(ShareTech_Regular_ttf));

    boost::shared_ptr<atlas::http::router> rest_router(rest::router(*m_connection));
    m_http_server.router().install(
            atlas::http::matcher("/api(.*)"),
            boost::bind(&atlas::http::router::serve, rest_router, _1, _2, _3, _4)
            );
    //rest::install(*m_connection, m_http_server);
    m_http_server.router().install(
        atlas::http::matcher("/item/([0-9]+)/image", "POST"),
        [this](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            const int item_id = boost::lexical_cast<int>(args[1]);
            upload_item_image(
                *m_connection,
                mg_conn,
                item_id,
                success,
                failure
                );
        }
        );
    m_http_server.router().install(
        atlas::http::matcher("/attachment", "POST"),
        [this](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            upload_file(
                *m_connection,
                mg_conn,
                "attachment_title",
                "attachment_data",
                [this, mg_conn, success](attachment a) {
                    insert_attachment(a, *m_connection);

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
    m_http_server.router().install(
        atlas::http::matcher("/attachment/([0-9]+)", "GET"),
        [this](
            mg_connection *mg_conn,
            boost::smatch args,
            atlas::http::uri_callback_type success,
            atlas::http::uri_callback_type failure
            )
        {
            const int attachment_id = boost::lexical_cast<int>(args[1]);
                //catch(const boost::bad_lexical_cast& e)
            return download_file(
                *m_mime_information,
                *m_connection,
                mg_conn,
                attachment_id,
                success,
                failure
                );
        }
        );
    m_http_server.router().install(
        atlas::http::matcher("/attachment/([0-9]+)/image/([0-9]+)x([0-9]+)", "GET"),
        [this](
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
                *m_mime_information,
                *m_connection,
                mg_conn,
                attachment_id,
                width,
                height,
                success,
                failure
                );
        }
        );

    //atlas::api::auth::install(*m_connection, m_auth_api_server);
    //m_http_server.router().install(
        //atlas::http::matcher("/auth", "post"),
        //boost::bind(&atlas::jsonrpc::uri, m_io, boost::ref(m_auth_api_server), _1, _2, _3, _4)
        //);

    atlas::log::information("apollo::server::server") <<
        "server listening on port " << options.port;
}

apollo::server::~server()
{
}

void apollo::server::start()
{
    atlas::log::information("apollo::server::start") << "running server";

    m_http_server.start();
}

void apollo::server::stop()
{
    atlas::log::information("apollo::server::stop") << "stopping server";
    m_http_server.stop();
}

hades::connection& apollo::server::db()
{
    return *m_connection;
}

boost::shared_ptr<boost::asio::io_service> apollo::server::io()
{
    return m_io;
}

