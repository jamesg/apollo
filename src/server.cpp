#include "server.hpp"

#include "hades/connection.hpp"
#include "hades/mkstr.hpp"

//#include "atlas/api/auth.hpp"
#include "atlas/http/server/router.hpp"
#include "atlas/http/server/install_static_file.hpp"
#include "atlas/http/server/static_file.hpp"
#include "atlas/jsonrpc/uri.hpp"
#include "atlas/log/log.hpp"

#include "attachment.hpp"
#include "db.hpp"
#include "image_scaled.hpp"
#include "item_image.hpp"
#include "rest.hpp"

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

    auto install_static_file = [this, &options](const char *filename) {
        atlas::http::install_static_file(
                m_http_server,
                *m_mime_information,
                hades::mkstr() << options.html_root << filename,
                filename
                );
    };

    atlas::http::install_static_file(
            m_http_server,
            *m_mime_information,
            hades::mkstr() << options.html_root << "/index.html",
            "/"
            );
    install_static_file("/index.html");
    install_static_file("/manage.html");
    install_static_file("/index.js");
    install_static_file("/manage.js");
    install_static_file("/backbone.js");
    install_static_file("/underscore.js");
    install_static_file("/backbone-min.js");
    install_static_file("/underscore-min.js");
    install_static_file("/jquery.js");
    install_static_file("/modal.css");
    install_static_file("/modal.js");
    install_static_file("/application.js");
    install_static_file("/stacked_application.js");
    install_static_file("/models.js");
    install_static_file("/style.css");
    install_static_file("/pure-min.css");
    install_static_file("/pure-base-grids.css");
    install_static_file("/open-iconic/font/css/open-iconic.css");
    install_static_file("/open-iconic/font/fonts/open-iconic.ttf");
    install_static_file("/open-iconic/font/fonts/open-iconic.woff");
    install_static_file("/ShareTechMono-Regular.ttf");
    install_static_file("/ShareTech-Regular.ttf");

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

