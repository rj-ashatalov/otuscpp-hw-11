#pragma once

#include <cstddef>
#include "Bulkmlt.h"

namespace async
{
    std::mutex lockPrint;

    using handle_t = void (*)();

    const handle_t connect(std::size_t bulk);
    void receive(handle_t handle, const char* data, std::size_t size);
    void disconnect(handle_t handle);

    struct Worker;
    std::vector<std::shared_ptr<Worker>> _contextCatch;
}
