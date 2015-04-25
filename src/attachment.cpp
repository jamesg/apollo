#include "attachment.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include "atlas/db/date.hpp"
#include "atlas/log/log.hpp"
#include "atlas/http/server/mimetypes.hpp"
#include "hades/crud.ipp"
#include "hades/detail/last_insert_rowid.hpp"
#include "hades/get_by_id.hpp"
#include "hades/step.hpp"
#include "styx/serialise_json.hpp"

#include "db.hpp"

void apollo::upload_file(
        hades::connection& conn,
        mg_connection *mg_conn,
        const char *title_attr,
        const char *data_attr,
        std::function<void(attachment)> callback_success,
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

        if(strcmp(var_name, title_attr) == 0)
            a.info.get_string<attr::attachment_title>() =
                std::string(data, data_len);

        if(strcmp(var_name, data_attr) == 0)
        {
            a.data.insert(
                    a.data.end(),
                    reinterpret_cast<const unsigned char*>(data),
                    reinterpret_cast<const unsigned char*>(data)+data_len
                    );
            a.info.get_string<attr::attachment_orig_filename>() = file_name;
        }

        a.info.get_string<attr::attachment_upload_date>() =
            atlas::db::date::to_string(
                boost::posix_time::second_clock::universal_time()
                );
    }

    try
    {
        callback_success(a);
    }
    catch(const std::exception&)
    {
        callback_failure();
    }
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

    attachment a = get_attachment(conn, attachment_info::id_type{attachment_id});

    const std::string filename(a.info.get_string<attr::attachment_orig_filename>());
    const std::string extension(filename.substr(filename.find_last_of(".") + 1));
    const std::string mimetype(mimetypes.content_type(extension));
    atlas::log::test("apollo::download_file") << "id " << attachment_id;
    atlas::log::test("apollo::download_file") << "filename " << filename;
    atlas::log::test("apollo::download_file") << "extension " << extension;
    atlas::log::test("apollo::download_file") << "mimetype " << mimetype;

    mg_send_header(mg_conn, "Content-type", mimetype.c_str());
    mg_send_data(mg_conn, &(a.data[0]), a.data.size());

    callback_success();
}

apollo::attachment apollo::get_attachment(
        hades::connection& conn,
        attachment_info::id_type id
        )
{
    attachment out;
    out.info = hades::get_by_id<attachment_info>(conn, id);
    atlas::log::test("apollo::get_attachment") << styx::serialise_json(out.info);

    sqlite3_stmt *stmt;
    sqlite3_prepare(
            conn.handle(),
            "SELECT attachment_data FROM attachment WHERE attachment_id = ?",
            -1,
            &stmt,
            nullptr
            );
    sqlite3_bind_int(stmt, 1, id.get_int<attr::attachment_id>());
    hades::step(stmt);
    out.data = std::vector<unsigned char>(
            (unsigned char*)sqlite3_column_blob(stmt, 0),
            (unsigned char*)sqlite3_column_blob(stmt, 0) + sqlite3_column_bytes(stmt, 0)
            );
    return out;
}

void apollo::insert_attachment(attachment& a, hades::connection& conn)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare(
            conn.handle(),
            "INSERT INTO attachment( "
            " attachment_title, attachment_orig_filename, "
            " attachment_upload_date, attachment_data "
            " ) VALUES (?, ?, ?, ?)",
            -1,
            &stmt,
            nullptr
            );
    hades::bind(1, a.info.copy_string<attr::attachment_title>(), stmt);
    hades::bind(2, a.info.copy_string<attr::attachment_orig_filename>(), stmt);
    hades::bind(3, a.info.copy_string<attr::attachment_upload_date>(), stmt);
    hades::bind(4, &(a.data[0]), a.data.size(), stmt);
    hades::step(stmt);
    sqlite3_finalize(stmt);
    a.info.set_id(// = hades::get_by_id<attachment_info>(
        //conn,
        attachment_info::id_type{hades::detail::last_insert_rowid(conn)}
        );
}

