#ifndef APOLLO_UPLOAD_HPP
#define APOLLO_UPLOAD_HPP

#include "mongoose.h"

#include "atlas/http/server/uri_type.hpp"

namespace atlas
{
    namespace http
    {
        class mimetypes;
    }
}
namespace hades
{
    class connection;
}
namespace apollo
{
    void upload_file(
            hades::connection& conn,
            mg_connection *mg_conn,
            atlas::http::uri_callback_type callback_success,
            atlas::http::uri_callback_type callback_failure
            );
    void download_file(
            const atlas::http::mimetypes&,
            hades::connection& conn,
            mg_connection *mg_conn,
            const int attachment_id,
            atlas::http::uri_callback_type callback_success,
            atlas::http::uri_callback_type callback_failure
            );
}

#endif

