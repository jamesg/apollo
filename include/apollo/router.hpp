#ifndef APOLLO_ROUTER_HPP
#define APOLLO_ROUTER_HPP

#include <boost/shared_ptr.hpp>

#include "atlas/http/server/mimetypes.hpp"
#include "atlas/http/server/router.hpp"

namespace hades
{
    class connection;
}
namespace apollo
{
    class router : public atlas::http::router
    {
    public:
        router(hades::connection&);

    private:
        void install_static_text(
                const std::string& url,
                const std::string& text
                );
        atlas::http::mimetypes m_mime_information;
    };
}

#endif

