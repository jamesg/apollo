#ifndef APOLLO_ITEM_IMAGE_HPP
#define APOLLO_ITEM_IMAGE_HPP

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
    /*!
     * \brief Upload an image and attach it to an item.
     */
    void upload_item_image(
            hades::connection& conn,
            mg_connection *mg_conn,
            const int item_id,
            atlas::http::uri_callback_type callback_success,
            atlas::http::uri_callback_type callback_failure
            );
}

#endif

