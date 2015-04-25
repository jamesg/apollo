#ifndef APOLLO_ATTACHMENT_HPP
#define APOLLO_ATTACHMENT_HPP

#include <vector>

#include "mongoose.h"

#include "atlas/http/server/uri_type.hpp"

#include "db.hpp"

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
     * \brief A file attachment including file data and metadata.
     */
    struct attachment
    {
        attachment_info info;
        std::vector<unsigned char> data;
    };

    /*!
     * \brief Retrieve an attachment from the database.
     */
    attachment get_attachment(hades::connection&, const attachment_info::id_type);
    /*!
     * \brief Save an attachment to the database.
     *
     * \note Sets the id of the attachment.
     */
    void insert_attachment(attachment&, hades::connection&);

    /*!
     * \brief Handle an incoming file upload but do not send any data to the
     * client.
     */
    void upload_file(
            hades::connection& conn,
            mg_connection *mg_conn,
            const char *title_attr,
            const char *data_attr,
            std::function<void(attachment)> callback_success,
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

