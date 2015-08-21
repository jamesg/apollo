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

#include "apollo/db.hpp"

namespace
{
    constexpr char item_count[] = "item_count";
}

boost::shared_ptr<atlas::http::router> apollo::rest::router(
    boost::shared_ptr<boost::asio::io_service> io,
    hades::connection& conn
)
{
    boost::shared_ptr<atlas::http::router>
        router(new atlas::http::router(io));

    //
    // Options.
    //

    router->install<std::string>(
        atlas::http::matcher("/option/([^/]+)", "DELETE"),
        [&conn](const std::string option_name) {
            if(
                hades::devoid(
                    "DELETE FROM apollo_option WHERE option_name = ?",
                    hades::row<std::string>(option_name),
                    conn
                    ) == 1
              )
                return atlas::http::json_response(true);
            else
                return atlas::http::json_error_response("deleting option");
        }
    );
    router->install<std::string>(
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
    router->install<>(
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
    router->install_json<styx::object>(
        atlas::http::matcher("/option", "PUT"),
        [&conn](styx::object o) {
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

    //
    // Types.
    //

    // Types that relate to a collection.
    router->install<styx::int_type>(
        atlas::http::matcher("/collection/([0-9]+)/type", "GET"),
        [&conn](const styx::int_type collection_id) {
            return atlas::http::json_response(
                hades::custom_select<type, hades::row<styx::int_type>, attr::type_id, attr::type_name, item_count>(
                    conn,
                    "SELECT apollo_type.type_id, apollo_type.type_name, COUNT(apollo_item.item_id) "
                    "FROM apollo_type "
                    "LEFT OUTER JOIN apollo_item "
                    "ON apollo_type.type_id = apollo_item.type_id "
                    "JOIN apollo_collection_type "
                    "ON apollo_type.type_id = apollo_collection_type.type_id "
                    "WHERE apollo_collection_type.collection_id = ? "
                    "GROUP BY apollo_type.type_id "
                    "ORDER BY apollo_type.type_name ASC ",
                    hades::row<styx::int_type>(collection_id)
                )
            );
        }
    );
    router->install<>(
        atlas::http::matcher("/type", "GET"),
        [&conn]() {
            styx::list types = hades::custom_select<
                type, attr::type_id, attr::type_name, item_count>(
                conn,
                "SELECT apollo_type.type_id, apollo_type.type_name, COUNT(apollo_item.item_id) "
                "FROM apollo_type LEFT OUTER JOIN apollo_item "
                "ON apollo_type.type_id = apollo_item.type_id "
                "GROUP BY apollo_type.type_id "
                "ORDER BY apollo_type.type_name ASC "
            );
            for(styx::element& e : types)
            {
                styx::object& o = boost::get<styx::object>(e);
                o.get_list("collections") = hades::custom_select<
                    collection,
                    hades::row<styx::int_type>,
                    attr::collection_id,
                    attr::collection_name>(
                        conn,
                        "SELECT apollo_collection.collection_id, apollo_collection.collection_name "
                        " FROM  apollo_collection, apollo_collection_type "
                        " WHERE apollo_collection.collection_id = apollo_collection_type.collection_id "
                        " AND apollo_collection_type.type_id = ? ",
                        hades::row<styx::int_type>(o.copy_int(attr::type_id))
                    );
            }
            return atlas::http::json_response(types);
        }
    );
    router->install_json<type>(
        atlas::http::matcher("/type", "POST"),
        [&conn](type t) {
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
            styx::list collections;
            for(collection c : t.get_list("collections"))
                collections.append(
                    collection_type(c.id(), t.id())
                );
            collection_type::save_collection(collections, conn);
            return atlas::http::json_response(t);
        }
    );
    router->install<int>(
        atlas::http::matcher("/type/([0-9]+)", "DELETE"),
        [&conn](const int type_id) {
            if(type_id == 0)
                return atlas::http::json_error_response("cannot delete the 'unknown' type");

            hades::devoid(
                "UPDATE apollo_item SET type_id = 0 WHERE type_id = ?",
                hades::row<styx::int_type>(type_id),
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
    router->install<int>(
        atlas::http::matcher("/type/([0-9]+)", "GET"),
        [&conn](const int type_id) {
            type t = hades::get_by_id<type>(conn, type::id_type{type_id});
            t.get_list("collections") = hades::custom_select<
                collection,
                hades::row<styx::int_type>,
                attr::collection_id,
                attr::collection_name>(
                    conn,
                    "SELECT apollo_collection.collection_id, apollo_collection.collection_name "
                    " FROM  apollo_collection, apollo_collection_type "
                    " WHERE apollo_collection.collection_id = apollo_collection_type.collection_id "
                    " AND apollo_collection_type.type_id = ? ",
                    hades::row<styx::int_type>(t.copy_int<attr::type_id>())
                );
            return atlas::http::json_response(t);
        }
    );
    router->install_json<type, int>(
        atlas::http::matcher("/type/([0-9]+)", "PUT"),
        [&conn](type t, const int type_id) {
            t.update(conn);
            styx::list collections;
            for(collection c : t.get_list("collections"))
                collections.append(
                    collection_type(c.id(), t.id())
                );
            collection_type::overwrite_collection(
                collections,
                hades::where(
                    "apollo_collection_type.type_id = ? ",
                    hades::row<styx::int_type>(t.copy_int<attr::type_id>())
                ),
                conn
            );
            return atlas::http::json_response(t);
        }
    );

    // Items of a given type.
    router->install<int>(
        atlas::http::matcher("/type/([0-9]+)/item", "GET"),
        [&conn](const int type_id) {
            return atlas::http::json_response(
                hades::join<type, item>(
                    conn,
                    hades::filter(
                        hades::where(
                            "apollo_type.type_id = apollo_item.type_id AND apollo_type.type_id = ?",
                            hades::row<styx::int_type>(type_id)
                            ),
                        hades::order_by("apollo_type.type_name ASC")
                        )
                    )
                );
        }
        );

    // All items.
    router->install<>(
        atlas::http::matcher("/item", "GET"),
        [&conn]() {
            styx::list items = hades::join<item, maker>(
                conn,
                hades::filter(
                    hades::where("apollo_item.maker_id = apollo_maker.maker_id"),
                    hades::order_by("apollo_item.item_year ASC ")
                )
            );
            for(styx::element& e : items) {
                styx::object& i = boost::get<styx::object>(e);
                i.get_list("collections") = hades::custom_select<
                    collection,
                    hades::row<styx::int_type>,
                    attr::collection_id,
                    attr::collection_name>(
                        conn,
                        "SELECT apollo_collection.collection_id, collection_name "
                        "FROM apollo_collection, apollo_item_in_collection "
                        "WHERE apollo_collection.collection_id = "
                        " apollo_item_in_collection.collection_id "
                        "AND apollo_item_in_collection.item_id = ? ",
                        hades::row<styx::int_type>(i.copy_int(attr::item_id))
                    );
            }
            return atlas::http::json_response(items);
        }
        );
    router->install_json<item>(
        atlas::http::matcher("/item", "POST"),
        [&conn](item i) {
            i.insert(conn);
            if(i.has_key("collections"))
            {
                styx::list collections;
                for(const styx::element& e : i.get_list("collections"))
                {
                    collection c(e);
                    item_in_collection ic;
                    ic.get_int<attr::collection_id>() =
                        c.copy_int<attr::collection_id>();
                    ic.get_int<attr::item_id>() = i.copy_int<attr::item_id>();
                    collections.append(ic);
                }
                item_in_collection::overwrite_collection(
                    collections,
                    hades::where(
                        "apollo_item_in_collection.item_id = ?",
                        hades::row<styx::int_type>(i.copy_int<attr::item_id>())
                    ),
                    conn
                );
            }
            return atlas::http::json_response(i);
        }
        );

    // Item detail.
    router->install<int>(
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
    router->install<int>(
        atlas::http::matcher("/item/([0-9]+)", "GET"),
        [&conn](const int item_id) {
            item i = styx::first(
                hades::outer_join<item, maker>(
                    conn,
                    "apollo_item.maker_id = apollo_maker.maker_id",
                    hades::where(
                        "apollo_item.item_id = ?",
                        hades::row<styx::int_type>(item_id)
                    )
                )
            );
            i.get_list("collections") = hades::custom_select<
                collection,
                hades::row<styx::int_type>,
                attr::collection_id,
                attr::collection_name>(
                    conn,
                    "SELECT apollo_collection.collection_id, collection_name "
                    "FROM apollo_collection, apollo_item_in_collection "
                    "WHERE apollo_collection.collection_id = "
                    " apollo_item_in_collection.collection_id "
                    "AND apollo_item_in_collection.item_id = ? ",
                    hades::row<styx::int_type>(i.copy_int<attr::item_id>())
                );
            return atlas::http::json_response(i);
        }
        );
    router->install_json<item, int>(
        atlas::http::matcher("/item/([0-9]+)", "PUT"),
        [&conn](item i, const int item_id) {
            i.update(conn);
            if(i.has_key("collections"))
            {
                styx::list collections;
                for(const styx::element& e : i.get_list("collections"))
                {
                    collection c(e);
                    item_in_collection ic;
                    ic.get_int<attr::collection_id>() =
                        c.copy_int<attr::collection_id>();
                    ic.get_int<attr::item_id>() = i.copy_int<attr::item_id>();
                    collections.append(ic);
                }
                item_in_collection::overwrite_collection(
                    collections,
                    hades::where(
                        "apollo_item_in_collection.item_id = ?",
                        hades::row<styx::int_type>(i.copy_int<attr::item_id>())
                    ),
                    conn
                );
            }
            return atlas::http::json_response(i);
        }
        );

    // Item images.
    router->install<int>(
        atlas::http::matcher("/item/([0-9]+)/image", "GET"),
        [&conn](const int item_id) {
            return atlas::http::json_response(
                hades::join<attachment_info, image_of>(
                    conn,
                    hades::where(
                        "apollo_attachment.attachment_id = apollo_image_of.attachment_id AND "
                        "apollo_image_of.item_id = ? ",
                        hades::row<styx::int_type>(item_id)
                        )
                    )
                );
        }
        );

    //
    // Makers.
    //

    // Makers related to a given collection.
    router->install<styx::int_type>(
        atlas::http::matcher("/collection/([0-9]+)/maker", "GET"),
        [&conn](const styx::int_type collection_id) {
            return atlas::http::json_response(
                hades::custom_select<type, hades::row<styx::int_type>, attr::maker_id, attr::maker_name, item_count>(
                    conn,
                    "SELECT apollo_maker.maker_id, apollo_maker.maker_name, COUNT(apollo_item.item_id) "
                    "FROM apollo_maker "
                    "LEFT OUTER JOIN apollo_item "
                    "ON apollo_maker.maker_id = apollo_item.maker_id "
                    "JOIN apollo_collection_maker "
                    "ON apollo_maker.maker_id = apollo_collection_maker.maker_id "
                    "WHERE apollo_collection_maker.collection_id = ? "
                    "GROUP BY apollo_maker.maker_id "
                    "ORDER BY apollo_maker.maker_name ASC ",
                    hades::row<styx::int_type>(collection_id)
                )
            );
        }
    );
    router->install<>(
        atlas::http::matcher("/maker", "GET"),
        [&conn]() {
            styx::list makers = hades::custom_select<maker, attr::maker_id, attr::maker_name, item_count>(
                conn,
                "SELECT apollo_maker.maker_id, apollo_maker.maker_name, COUNT(apollo_item.item_id) "
                "FROM apollo_maker LEFT OUTER JOIN apollo_item "
                "ON apollo_maker.maker_id = apollo_item.maker_id "
                "GROUP BY apollo_maker.maker_id "
                "ORDER BY apollo_maker.maker_name ASC "
            );
            for(styx::element& e : makers)
            {
                styx::object& o = boost::get<styx::object>(e);
                o.get_list("collections") = hades::custom_select<
                    collection,
                    hades::row<styx::int_type>,
                    attr::collection_id,
                    attr::collection_name>(
                        conn,
                        "SELECT apollo_collection.collection_id, apollo_collection.collection_name "
                        " FROM  apollo_collection, apollo_collection_maker "
                        " WHERE apollo_collection.collection_id = apollo_collection_maker.collection_id "
                        " AND apollo_collection_maker.maker_id = ? ",
                        hades::row<styx::int_type>(o.copy_int(attr::maker_id))
                    );
            }
            return atlas::http::json_response(makers);
        }
    );
    router->install_json<maker>(
        atlas::http::matcher("/maker", "POST"),
        [&conn](maker m) {
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
            styx::list collections;
            for(collection c : m.get_list("collections"))
                collections.append(
                    collection_maker(c.id(), m.id())
                );
            collection_maker::save_collection(collections, conn);
            return atlas::http::json_response(m);
        }
        );

    // Maker detail.
    router->install<int>(
        atlas::http::matcher("/maker/([0-9]+)", "DELETE"),
        [&conn](const int maker_id) {
            if(maker_id == 0)
                return atlas::http::json_error_response("cannot delete the 'unknown' maker");

            hades::devoid(
                "UPDATE apollo_item SET maker_id = 0 WHERE maker_id = ?",
                hades::row<styx::int_type>(maker_id),
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
    router->install<int>(
        atlas::http::matcher("/maker/([0-9]+)", "GET"),
        [&conn](const int maker_id) {
            maker m = hades::get_by_id<maker>(conn, maker::id_type{maker_id});
            m.get_list("collections") = hades::custom_select<
                collection,
                hades::row<styx::int_type>,
                attr::collection_id,
                attr::collection_name>(
                    conn,
                    "SELECT apollo_collection.collection_id, apollo_collection.collection_name "
                    " FROM  apollo_collection, apollo_collection_maker "
                    " WHERE apollo_collection.collection_id = apollo_collection_maker.collection_id "
                    " AND apollo_collection_maker.maker_id = ? ",
                    hades::row<styx::int_type>(m.copy_int<attr::maker_id>())
                );
            return atlas::http::json_response(m);
        }
        );
    router->install_json<maker, int>(
        atlas::http::matcher("/maker/([0-9]+)", "PUT"),
        [&conn](maker m, const int maker_id) {
            m.update(conn);
            styx::list collections;
            for(collection c : m.get_list("collections"))
                collections.append(
                    collection_maker(c.id(), m.id())
                );
            collection_maker::overwrite_collection(
                collections,
                hades::where(
                    "apollo_collection_maker.maker_id = ? ",
                    hades::row<styx::int_type>(m.copy_int<attr::maker_id>())
                ),
                conn
            );
            return atlas::http::json_response(m);
        }
        );
    router->install<int>(
        atlas::http::matcher("/maker/([0-9]+)/item", "GET"),
        [&conn](const int maker_id) {
            return atlas::http::json_response(
                hades::outer_join<item, maker>(
                    conn,
                    "apollo_item.maker_id = apollo_maker.maker_id",
                    hades::filter(
                        hades::where(
                            "apollo_item.maker_id = ?",
                            hades::row<styx::int_type>(maker_id)
                            ),
                        hades::order_by("apollo_item.item_name ASC")
                        )
                    )
                );
        }
        );

    // Year.
    router->install<int>(
        atlas::http::matcher("/year/([0-9]+)/item", "GET"),
        [&conn](const int year) {
            return atlas::http::json_response(
                hades::get_collection<item>(
                    conn,
                    hades::filter(
                        hades::where(
                            "apollo_item.year = ?",
                            hades::row<styx::int_type>(year)
                        ),
                        hades::order_by("apollo_item.item_name ASC")
                        )
                    )
                );
        }
        );

    // Attachments.
    router->install<int>(
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
    router->install_json<attachment_info, int>(
        atlas::http::matcher("/attachment/([0-9]+)/info", "PUT"),
        [&conn](attachment_info a, const int attachment_id) {
            a.update(conn);
            return atlas::http::json_response(
                hades::get_by_id<attachment_info>(
                    conn,
                    attachment_info::id_type{attachment_id}
                    )
                );
        }
        );
    router->install<int>(
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

    //
    // Collections.
    //

    router->install<>(
        atlas::http::matcher("/collection", "GET"),
        [&conn]() {
            return atlas::http::json_response(collection::get_collection(conn));
        }
    );
    router->install_json<collection>(
        atlas::http::matcher("/collection", "POST"),
        [&conn](collection c) {
            c.save(conn);
            return atlas::http::json_response(c);
        }
    );
    router->install_json<collection>(
        atlas::http::matcher("/collection/([0-9]+)", "PUT"),
        [&conn](collection c) {
            c.update(conn);
            return atlas::http::json_response(c);
        }
    );

    return router;
}
