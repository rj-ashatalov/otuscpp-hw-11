#include "async.h"

namespace async
{

    handle_t connect(std::size_t bulk)
    {
        auto ctx = _contextCatch.emplace_back(new Bulkmlt(bulk));
        return ctx;
    }

    void receive(handle_t handle, const char* data, std::size_t size)
    {
        handle->Execute(data, size);
    }

    void disconnect(handle_t handle)
    {
        _contextCatch.erase(std::find(_contextCatch.begin(), _contextCatch.end(), handle));
    }

}
