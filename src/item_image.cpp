#include "item_image.hpp"

#include "hades/crud.ipp"

#include "attachment.hpp"

void apollo::upload_item_image(
        hades::connection& conn,
        mg_connection *mg_conn,
        const int item_id,
        atlas::http::uri_callback_type callback_success,
        atlas::http::uri_callback_type callback_failure
        )
{
    upload_file(
        conn,
        mg_conn,
        "attachment_title",
        "attachment_data",
        [&conn, mg_conn, callback_success, item_id](attachment a) {
            insert_attachment(a, conn);

            image_of iof;
            iof.get_int<attr::attachment_id>() =
                a.info.get_int<attr::attachment_id>();
            iof.get_int<attr::item_id>() = item_id;
            iof.insert(conn);

            // Send details of the attachment.
            auto r = atlas::http::json_response(a.info);

            mg_send_status(mg_conn, 200);
            mg_send_header(mg_conn, "Content-type", "text/json");
            mg_send_data(mg_conn, r.data.c_str(), r.data.length());

            callback_success();
        },
        callback_failure
        );
}

