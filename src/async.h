#pragma once

#include <cstddef>
#include "Bulkmlt.h"
#include <memory>

namespace async
{
    struct Worker;
    std::vector<std::shared_ptr<Worker>> _contextCache;

    using handle_t = Worker *;

    handle_t connect(std::size_t bulk);
    void receive(handle_t handle, const char* data, std::size_t size);
    void disconnect(handle_t handle);
}
