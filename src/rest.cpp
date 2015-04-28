#include "rest.hpp"

#include "atlas/http/server/router.hpp"
#include "atlas/http/server/server.hpp"
#include "hades/crud.ipp"
#include "hades/custom_select.hpp"
#include "hades/devoid.hpp"
#include "hades/exists.hpp"
#include "hades/join.hpp"
#include "hades/mkstr.hpp"
#include "styx/cast.hpp"

#include "db.hpp"

namespace
{
    constexpr char item_count[] = "item_count";
}

void apollo::rest::install(hades::connection& conn, atlas::http::server& server)
{
    // Options.
    server.router().install<std::string>(
        atlas::http::matcher("/option/([^/]+)", "DELETE"),
        [&conn](const std::string option_name) {
            if(
                hades::devoid(
                    "DELETE FROM option WHERE option_name = ?",
                    hades::row<std::string>(option_name),
                    conn
                    ) == 1
              )
                return atlas::http::json_response(true);
            else
                return atlas::http::json_error_response("deleting option");
        }
        );
    server.router().install<std::string>(
        atlas::http::matcher("/option/([^/]+)", "GET"),
        [&conn](const std::string option_name) {
            return atlas::http::json_response(
                hades::get_one<option>(
                    conn,
                    hades::where("option_name = ?", hades::row<std::string>(option_name))
                    )
                );
        }
        );
    server.router().install<>(
        atlas::http::matcher("/option", "GET"),
        [&conn]() {
            styx::list l = hades::get_collection<option>(conn);
            styx::object out;
            for(const styx::element& e : l)
            {
                option o(e);
                out.get_string(o.get_string<attr::option_name>()) =
                    o.get_string<attr::option_value>();
            }
            return atlas::http::json_response(out);
        }
        );
    server.router().install_json<>(
        atlas::http::matcher("/option", "PUT"),
        [&conn](const styx::element e) {
            styx::object o(e);
            for(const std::pair<std::string, styx::element> p : o)
            {
                option(p.first, styx::cast<std::string>(p.second)).save(conn);
            }

            styx::list l = hades::get_collection<option>(conn);
            styx::object out;
            for(const styx::element& e : l)
            {
                option o(e);
                out.get_string(o.get_string<attr::option_name>()) =
                    o.get_string<attr::option_value>();
            }
            return atlas::http::json_response(out);
        }
        );
    // Type collection.
    server.router().install<>(
        atlas::http::matcher("/type", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::custom_select<type, attr::type_id, attr::type_name, item_count>(
                    conn,
                    "SELECT type.type_id, type.type_name, COUNT(item.item_id) "
                    "FROM type LEFT OUTER JOIN item "
                    "ON type.type_id = item.type_id "
                    "GROUP BY type.type_id "
                    )
                );
        }
        );
    server.router().install_json<>(
        atlas::http::matcher("/type", "POST"),
        [&conn](const styx::element type_e) {
            type t(type_e);
            if(
                hades::exists<type>(
                    conn,
                    hades::where(
                        "type_name = ?",
                        hades::row<std::string>(t.get_string<attr::type_name>())
                        )
                    )
              )
                return atlas::http::json_error_response(
                    hades::mkstr() << "type '" <<
                        t.get_string<attr::type_name>() << "' already present"
                    );
            t.insert(conn);
            return atlas::http::json_response(t);
        }
        );

    // Type detail.
    server.router().install<int>(
        atlas::http::matcher("/type/([0-9]+)", "DELETE"),
        [&conn](const int type_id) {
            if(type_id == 0)
                return atlas::http::json_error_response("cannot delete the 'unknown' type");

            hades::devoid(
                "UPDATE item SET type_id = 0 WHERE type_id = ?",
                hades::row<int>(type_id),
                conn
                );
            type t;
            t.set_id(type::id_type{type_id});
            if(t.destroy(conn))
                return atlas::http::json_response(t);
            else
                return atlas::http::json_error_response("deleting type");
        }
        );
    server.router().install<int>(
        atlas::http::matcher("/type/([0-9]+)", "GET"),
        [&conn](const int type_id) {
            return atlas::http::json_response(
                hades::get_by_id<type>(conn, type::id_type{type_id})
                );
        }
        );
    server.router().install_json<int>(
        atlas::http::matcher("/type/([0-9]+)", "PUT"),
        [&conn](const styx::element e, const int type_id) {
            type t(e);
            t.update(conn);
            return atlas::http::json_response(t);
        }
        );

    // Type item collection.
    server.router().install<int>(
        atlas::http::matcher("/type/([0-9]+)/item", "GET"),
        [&conn](const int type_id) {
            return atlas::http::json_response(
                hades::join<type, item>(
                    conn,
                    hades::where(
                        "type.type_id = item.type_id AND type.type_id = ?",
                        hades::row<int>(type_id)
                        )
                    )
                );
        }
        );

    // Item collection.
    server.router().install<>(
        atlas::http::matcher("/item", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::get_collection<item>(conn)
                );
        }
        );
    server.router().install_json<>(
        atlas::http::matcher("/item", "POST"),
        [&conn](const styx::element item_e) {
            item i(item_e);
            i.insert(conn);
            return atlas::http::json_response(i);
        }
        );

    // Item detail.
    server.router().install<int>(
        atlas::http::matcher("/item/([0-9]+)", "DELETE"),
        [&conn](const int item_id) {
            item i;
            i.set_id(item::id_type{item_id});
            if(i.destroy(conn))
                return atlas::http::json_response(i);
            else
                return atlas::http::json_error_response("deleting item");
        }
        );
    server.router().install<int>(
        atlas::http::matcher("/item/([0-9]+)", "GET"),
        [&conn](const int item_id) {
            styx::list items = hades::outer_join<item, maker>(
                conn,
                hades::where(
                    "item.maker_id = maker.maker_id and "
                    "item.item_id = ?",
                    hades::row<int>(item_id)
                    )
                );
            if(items.size() != 1)
                return atlas::http::json_error_response(
                    hades::mkstr() << "no item with id " << item_id
                    );
            return atlas::http::json_response(items.at(0));
        }
        );
    server.router().install_json<int>(
        atlas::http::matcher("/item/([0-9]+)", "PUT"),
        [&conn](const styx::element e, const int item_id) {
            item i(e);
            if(i.update(conn))
                return atlas::http::json_response(i);
            else
                return atlas::http::json_error_response("Saving item");
        }
        );

    // Item images.
    server.router().install<int>(
        atlas::http::matcher("/item/([0-9]+)/image", "GET"),
        [&conn](const int item_id) {
            return atlas::http::json_response(
                hades::join<attachment_info, image_of>(
                    conn,
                    hades::where(
                        "attachment.attachment_id = image_of.attachment_id AND "
                        "image_of.item_id = ? ",
                        hades::row<int>(item_id)
                        )
                    )
                );
        }
        );

    // Maker collection.
    server.router().install<>(
        atlas::http::matcher("/maker", "GET"),
        [&conn]() {
            return atlas::http::json_response(
                hades::custom_select<maker, attr::maker_id, attr::maker_name, item_count>(
                    conn,
                    "SELECT maker.maker_id, maker.maker_name, COUNT(item.item_id) "
                    "FROM maker LEFT OUTER JOIN item "
                    "ON maker.maker_id = item.maker_id "
                    "GROUP BY maker.maker_id "
                    )
                );
        }
        );
    server.router().install_json<>(
        atlas::http::matcher("/maker", "POST"),
        [&conn](const styx::element maker_e) {
            maker m(maker_e);
            if(
                hades::exists<maker>(
                    conn,
                    hades::where(
                        "maker_name = ?",
                        hades::row<std::string>(m.get_string<attr::maker_name>())
                        )
                    )
              )
                return atlas::http::json_error_response(
                    hades::mkstr() << "maker '" <<
                        m.get_string<attr::maker_name>() << "' already present"
                    );

            m.insert(conn);
            return atlas::http::json_response(m);
        }
        );

    // Maker detail.
    server.router().install<int>(
        atlas::http::matcher("/maker/([0-9]+)", "DELETE"),
        [&conn](const int maker_id) {
            if(maker_id == 0)
                return atlas::http::json_error_response("cannot delete the 'unknown' maker");

            hades::devoid(
                "UPDATE item SET maker_id = 0 WHERE maker_id = ?",
                hades::row<int>(maker_id),
                conn
                );
            maker m;
            m.set_id(maker::id_type{maker_id});
            if(m.destroy(conn))
                return atlas::http::json_response(m);
            else
                return atlas::http::json_error_response("deleting maker");
        }
        );
    server.router().install<int>(
        atlas::http::matcher("/maker/([0-9]+)", "GET"),
        [&conn](const int maker_id) {
            return atlas::http::json_response(
                hades::get_by_id<maker>(conn, maker::id_type{maker_id})
                );
        }
        );
    server.router().install_json<int>(
        atlas::http::matcher("/maker/([0-9]+)", "PUT"),
        [&conn](const styx::element maker_e, const int maker_id) {
            maker m(maker_e);
            m.update(conn);
            return atlas::http::json_response(m);
        }
        );
    server.router().install<int>(
        atlas::http::matcher("/maker/([0-9]+)/item", "GET"),
        [&conn](const int maker_id) {
            return atlas::http::json_response(
                hades::outer_join<item, maker>(
                    conn,
                    hades::where(
                        "item.maker_id = maker.maker_id and "
                        "item.maker_id = ?",
                        hades::row<int>(maker_id)
                        )
                    )
                );
        }
        );

    // Year.
    server.router().install<int>(
        atlas::http::matcher("/year/([0-9]+)/item", "GET"),
        [&conn](const int year) {
            return atlas::http::json_response(
                hades::get_collection<item>(
                    conn,
                    hades::where("item.year = ?", hades::row<int>(year))
                    )
                );
        }
        );

    // Attachments.
    server.router().install<int>(
        atlas::http::matcher("/attachment/([0-9]+)/info", "GET"),
        [&conn](const int attachment_id) {
            return atlas::http::json_response(
                hades::get_by_id<attachment_info>(
                    conn,
                    attachment_info::id_type{attachment_id}
                    )
                );
        }
        );
    server.router().install_json<int>(
        atlas::http::matcher("/attachment/([0-9]+)/info", "PUT"),
        [&conn](styx::element a_e, const int attachment_id) {
            attachment_info a(a_e);
            a.update(conn);
            return atlas::http::json_response(
                hades::get_by_id<attachment_info>(
                    conn,
                    attachment_info::id_type{attachment_id}
                    )
                );
        }
        );
    server.router().install<int>(
        atlas::http::matcher("/attachment/([0-9]+)/info", "DELETE"),
        [&conn](const int attachment_id) {
            attachment_info a;
            a.set_id(attachment_info::id_type{attachment_id});
            if(a.destroy(conn))
                return atlas::http::json_response(a);
            else
                return atlas::http::json_error_response("deleting attachment");
        }
        );
}

