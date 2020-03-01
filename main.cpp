#include <iostream>
#include <BulkImpl.h>

#include "async.h"

int main(int, char *[]) {
//    std::size_t bulk = 5;
//    auto h = async::connect(bulk);
//    auto h2 = async::connect(bulk);
//    async::receive(h, "1", 1);
//    async::receive(h2, "1\n", 2);
//    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
//    async::receive(h, "b\nc\nd\n}\n89\n", 11);
//    async::disconnect(h);
//    async::disconnect(h2);

    //simple one thread
    std::size_t bulk = 3;
    auto h = async::connect(bulk);
    async::receive(h, "1\n2\n3\n", 3);
    async::disconnect(h);

    //legacy
    /*auto bulk = std::make_shared<Bulkmlt>(3);
    BulkImpl impl(bulk);

    bulk->ExecuteAll("1\n2\n3\n", 3);

    impl.Complete();*/

    return 0;
}
