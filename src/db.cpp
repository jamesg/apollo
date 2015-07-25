#include "apollo/db.hpp"

#include "hades/devoid.hpp"
#include "hades/exists.hpp"
#include "hades/filter.hpp"

const char apollo::attr::option_name[] = "option_name";
const char apollo::attr::option_value[] = "option_value";
const char apollo::attr::type_id[] = "type_id";
const char apollo::attr::type_name[] = "type_name";
const char apollo::attr::item_id[] = "item_id";
const char apollo::attr::item_name[] = "item_name";
const char apollo::attr::item_notes[] = "item_notes";
const char apollo::attr::item_year[] = "item_year";
const char apollo::attr::item_cond[] = "item_cond";
const char apollo::attr::item_cond_notes[] = "item_cond_notes";
const char apollo::attr::maker_id[] = "maker_id";
const char apollo::attr::maker_name[] = "maker_name";
const char apollo::attr::attachment_id[] = "attachment_id";
const char apollo::attr::attachment_orig_filename[] = "attachment_orig_filename";
const char apollo::attr::attachment_title[] = "attachment_title";
const char apollo::attr::attachment_upload_date[] = "attachment_upload_date";
const char apollo::attr::collection_id[] = "collection_id";
const char apollo::attr::collection_name[] = "collection_name";
const char apollo::relvar::option[] = "apollo_option";
const char apollo::relvar::type[] = "apollo_type";
const char apollo::relvar::item[] = "apollo_item";
const char apollo::relvar::maker[] = "apollo_maker";
const char apollo::relvar::attachment[] = "apollo_attachment";
const char apollo::relvar::image_of[] = "apollo_image_of";
const char apollo::relvar::collection[] = "apollo_collection";
const char apollo::relvar::item_in_collection[] = "apollo_item_in_collection";
const char apollo::relvar::collection_maker[] = "apollo_collection_maker";
const char apollo::relvar::collection_type[] = "apollo_collection_type";

void apollo::db::create(hades::connection& conn)
{
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_option ( "
        " option_name VARCHAR PRIMARY KEY, "
        " option_value VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_type ( "
        " type_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " type_name VARCHAR UNIQUE NOT NULL "
        " ) ",
        conn
        );
    if(!hades::exists<type>(conn, hades::where("type_id = 0")))
        hades::devoid(
            "INSERT INTO apollo_type(type_id, type_name) VALUES(0, 'Unknown')",
            conn
            );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_maker ( "
        " maker_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " maker_name VARCHAR "
        " ) ",
        conn
        );
    if(!hades::exists<maker>(conn, hades::where("maker_id = 0")))
        hades::devoid(
            "INSERT INTO apollo_maker(maker_id, maker_name) VALUES(0, 'Unknown')",
            conn
            );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_item ( "
        " item_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " type_id INTEGER REFERENCES apollo_type(type_id), "
        " maker_id INTEGER REFERENCES apollo_maker(maker_id), "
        " item_name VARCHAR, "
        " item_notes VARCHAR, "
        " item_year INTEGER, "
        " item_cond INTEGER DEFAULT 0, "
        " item_cond_notes VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_attachment ( "
        " attachment_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " attachment_orig_filename VARCHAR, "
        " attachment_title VARCHAR, "
        " attachment_upload_date VARCHAR, "
        " attachment_data BLOB "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_image_of ( "
        " attachment_id INTEGER REFERENCES apollo_attachment(attachment_id) "
        "  ON DELETE CASCADE, "
        " item_id INTEGER REFERENCES apollo_item(item_id) "
        "  ON DELETE CASCADE "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_collection ( "
        " collection_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " collection_name VARCHAR "
        ") ",
        conn
    );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_item_in_collection ( "
        " item_id INTEGER REFERENCES apollo_item(item_id), "
        " collection_id INTEGER REFERENCES apollo_collection(collection_id), "
        " UNIQUE(item_id, collection_id) "
        ") ",
        conn
    );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_collection_maker ( "
        " maker_id INTEGER REFERENCES apollo_maker(maker_id), "
        " collection_id INTEGER REFERENCES apollo_collection(collection_id), "
        " UNIQUE(maker_id, collection_id) "
        ") ",
        conn
    );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS apollo_collection_type ( "
        " type_id INTEGER REFERENCES apollo_type(type_id), "
        " collection_id INTEGER REFERENCES apollo_collection(collection_id), "
        " UNIQUE(type_id, collection_id) "
        ") ",
        conn
    );
}
