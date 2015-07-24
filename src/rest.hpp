#ifndef APOLLO_REST_HPP
#define APOLLO_REST_HPP

#include <boost/shared_ptr.hpp>

#include "atlas/http/server/router.hpp"

namespace hades
{
    class connection;
}
namespace atlas
{
    namespace http
    {
        class server;
    }
}
namespace apollo
{
    namespace rest
    {
        /*!
         * \brief Install Apollo REST URIs to a HTTP server.
         */
        boost::shared_ptr<atlas::http::router> router(
            boost::shared_ptr<boost::asio::io_service>,
            hades::connection&
        );
    }
}

#endif
