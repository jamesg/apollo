#include "db.hpp"

#include "hades/devoid.hpp"

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
const char apollo::attr::attachment_data[] = "attachment_data";
const char apollo::relvar::type[] = "type";
const char apollo::relvar::item[] = "item";
const char apollo::relvar::maker[] = "maker";
const char apollo::relvar::attachment[] = "attachment";
const char apollo::relvar::image_of[] = "image_of";

void apollo::db::create(hades::connection& conn)
{
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS type ( "
        " type_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " type_name VARCHAR UNIQUE NOT NULL "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS item ( "
        " item_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " type_id INTEGER REFERENCES type(type_id), "
        " maker_id INTEGER REFERENCES maker(maker_id), "
        " item_name VARCHAR, "
        " item_notes VARCHAR, "
        " item_year INTEGER, "
        " item_cond INTEGER DEFAULT 0, "
        " item_cond_notes VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS maker ( "
        " maker_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " maker_name VARCHAR "
        " ) ",
        conn
        );
    hades::devoid(
        "CREATE TABLE IF NOT EXISTS attachment ( "
        " attachment_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        " attachment_orig_filename VARCHAR, "
        " attachment_title VARCHAR, "
        " attachment_upload_date VARCHAR, "
        " attachment_data BLOB "
        " ) ",
        conn
        );
}

