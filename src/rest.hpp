#ifndef APOLLO_REST_HPP
#define APOLLO_REST_HPP

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
        void install(hades::connection&, atlas::http::server&);
    }
}

#endif

