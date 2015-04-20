#ifndef APOLLO_IMAGE_SCALED_HPP
#define APOLLO_IMAGE_SCALED_HPP

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
    void image_scaled(
            const atlas::http::mimetypes&,
            hades::connection& conn,
            mg_connection *mg_conn,
            const int attachment_id,
            int width,
            int height,
            atlas::http::uri_callback_type callback_success,
            atlas::http::uri_callback_type callback_failure
            );
}

#endif

