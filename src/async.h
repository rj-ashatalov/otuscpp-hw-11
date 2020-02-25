#pragma once

#include <cstddef>
#include "Bulkmlt.h"

namespace async
{

    using handle_t = std::weak_ptr<Bulkmlt>;

    handle_t connect(std::size_t bulk);
    void receive(handle_t handle, const char* data, std::size_t size);
    void disconnect(handle_t handle);

    std::vector<std::shared_ptr<Bulkmlt>> _contextCatch;
}
