#ifndef APOLLO_DB_HPP
#define APOLLO_DB_HPP

#include "hades/crud.hpp"
#include "hades/has_candidate_key.hpp"
#include "hades/relation.hpp"
#include "hades/tuple.hpp"

namespace apollo
{
    namespace attr
    {
        extern const char option_name[];
        extern const char option_value[];
        extern const char type_id[];
        extern const char type_name[];
        extern const char item_id[];
        extern const char item_name[];
        extern const char item_notes[];
        extern const char item_year[];
        extern const char item_cond[];
        extern const char item_cond_notes[];
        extern const char maker_id[];
        extern const char maker_name[];
        extern const char attachment_id[];
        extern const char attachment_orig_filename[];
        extern const char attachment_title[];
        extern const char attachment_upload_date[];
        extern const char collection_id[];
        extern const char collection_name[];
    }
    namespace relvar
    {
        extern const char option[];
        extern const char type[];
        extern const char item[];
        extern const char maker[];
        extern const char attachment[];
        extern const char image_of[];
        extern const char collection[];
        extern const char item_in_collection[];
    }

    class option :
        public hades::tuple<attr::option_name, attr::option_value>,
        public hades::has_candidate_key<attr::option_name>,
        public hades::relation<relvar::option>,
        public hades::crud<option>
    {
    public:
        option()
        {
        }

        option(const styx::element& e) :
            styx::object(e)
        {
        }

        option(const std::string& name, const std::string& value)
        {
            get_string<attr::option_name>() = name;
            get_string<attr::option_value>() = value;
        }
    };

    class type :
        public hades::tuple<attr::type_id, attr::type_name>,
        public hades::has_candidate_key<attr::type_id>,
        public hades::relation<relvar::type>,
        public hades::crud<type>
    {
    public:
        type()
        {
        }

        type(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class item :
        public hades::tuple<
            attr::item_id,
            attr::item_name,
            attr::item_notes,
            attr::item_year,
            attr::item_cond,
            attr::item_cond_notes,
            attr::maker_id,
            attr::type_id>,
        public hades::has_candidate_key<attr::item_id>,
        public hades::relation<relvar::item>,
        public hades::crud<item>
    {
    public:
        item()
        {
        }

        item(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class maker :
        public hades::tuple<attr::maker_id, attr::maker_name>,
        public hades::has_candidate_key<attr::maker_id>,
        public hades::relation<relvar::maker>,
        public hades::crud<maker>
    {
    public:
        maker()
        {
        }

        maker(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class attachment_info :
        public hades::tuple<
            attr::attachment_id,
            attr::attachment_orig_filename,
            attr::attachment_title,
            attr::attachment_upload_date>,
        public hades::has_candidate_key<attr::attachment_id>,
        public hades::relation<relvar::attachment>,
        public hades::crud<attachment_info>
    {
    public:
        attachment_info()
        {
        }

        attachment_info(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class image_of :
        public hades::tuple<attr::item_id, attr::attachment_id>,
        public hades::has_candidate_key<attr::item_id, attr::attachment_id>,
        public hades::relation<relvar::image_of>,
        public hades::crud<image_of>
    {
    public:
        image_of()
        {
        }

        image_of(const int item_id, const int attachment_id)
        {
            get_int<attr::item_id>() = item_id;
            get_int<attr::attachment_id>() = attachment_id;
        }

        explicit image_of(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class collection :
        public hades::tuple<attr::collection_id, attr::collection_name>,
        public hades::has_candidate_key<attr::collection_id>,
        public hades::relation<relvar::collection>,
        public hades::crud<collection>
    {
    public:
        collection()
        {
        }
        explicit collection(const styx::element& e) :
            styx::object(e)
        {
        }
    };

    class item_in_collection :
        public hades::tuple<attr::item_id, attr::collection_id>,
        public hades::has_candidate_key<attr::item_id, attr::collection_id>,
        public hades::relation<relvar::item_in_collection>,
        public hades::crud<item_in_collection>
    {
    public:
        item_in_collection()
        {
        }
        explicit item_in_collection(const styx::element& e)
        {
        }
        item_in_collection(
            const item::id_type& item_id,
            const collection::id_type& collection_id
        )
        {
            get_int<attr::item_id>() = item_id.copy_int<attr::item_id>();
            get_int<attr::collection_id>() =
                collection_id.copy_int<attr::collection_id>();
        }
    };

    namespace db
    {
        void create(hades::connection&);
    }
}

#endif
