#pragma once

#include <cstddef>
#include "Bulkmlt.h"

namespace async
{
    struct Worker;
    using handle_t = void *;

    handle_t connect(std::size_t bulk);
    void receive(handle_t handle, const char* data, std::size_t size);
    void disconnect(handle_t handle);

    std::vector<std::shared_ptr<Worker>> _contextCache;
}
