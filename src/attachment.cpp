#include "attachment.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "atlas/db/date.hpp"
#include "atlas/log/log.hpp"
#include "atlas/http/server/mimetypes.hpp"
#include "hades/crud.ipp"
#include "hades/get_by_id.hpp"
#include "styx/serialise_json.hpp"

#include "db.hpp"

void apollo::upload_file(
        hades::connection& conn,
        mg_connection *mg_conn,
        atlas::http::uri_callback_type callback_success,
        atlas::http::uri_callback_type callback_failure
        )
{
    attachment a;

    int skip = 0, skip_total = 0;
    const char *data;
    int data_len;
    char var_name[100], file_name[100];
    while((skip = mg_parse_multipart(
                mg_conn->content+skip_total, mg_conn->content_len-skip_total,
                var_name, sizeof(var_name),
                file_name, sizeof(file_name),
                &data, &data_len
                )
         ) > 0)
    {
        skip_total += skip;

        if(strcmp(var_name, "attachment_title") == 0)
            a.get_string<attr::attachment_title>() =
                std::string(data, data_len);

        if(strcmp(var_name, "attachment_data") == 0)
        {
            a.get_string<attr::attachment_data>() = std::string(data, data_len);
            a.get_string<attr::attachment_orig_filename>() = file_name;
        }
    }

    a.get_string<attr::attachment_upload_date>() =
        atlas::db::date::to_string(boost::posix_time::second_clock::universal_time());
    a.insert(conn);

    mg_send_status(mg_conn, 200);
    mg_send_header(mg_conn, "Content-type", "text/json");

    a.erase(attr::attachment_data);
    auto r = atlas::http::json_response(a);
    mg_send_data(mg_conn, r.data.c_str(), r.data.length());

    callback_success();
}

void apollo::download_file(
        const atlas::http::mimetypes& mimetypes,
        hades::connection& conn,
        mg_connection *mg_conn,
        const int attachment_id,
        atlas::http::uri_callback_type callback_success,
        atlas::http::uri_callback_type callback_failure
        )
{
    mg_send_status(mg_conn, 200);

    attachment a =
        hades::get_by_id<attachment>(conn, attachment::id_type{attachment_id});

    const std::string filename(a.get_string<attr::attachment_orig_filename>());
    const std::string extension(filename.substr(filename.find_last_of(".") + 1));
    const std::string mimetype(mimetypes.content_type(extension));

    mg_send_header(mg_conn, "Content-type", mimetype.c_str());
    mg_send_data(
        mg_conn,
        &(a.get_string<attr::attachment_data>().c_str()[0]),
        a.get_string<attr::attachment_data>().size()
        );

    callback_success();
}

