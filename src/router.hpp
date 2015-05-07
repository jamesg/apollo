#ifndef APOLLO_ROUTER_HPP
#define APOLLO_ROUTER_HPP

#include <boost/shared_ptr.hpp>

#include "atlas/http/server/router.hpp"

namespace hades
{
    class connection;
}
namespace apollo
{
    boost::shared_ptr<atlas::http::router> router(hades::connection&);
}

#endif

