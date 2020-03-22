#include <iostream>
#include <BulkImpl.h>

#include "async.h"

int main(int, char *[]) {
    //loose empty line
    const char *data="0 0\n\n1 1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n{\n11\n}\n{\n12\n13\n14\n}\n15\n{\n16\n{\n17\n{\n18\n}\n19\n}\n20\n}\n{\n21\n22\n23\n";
    auto h = async::connect(3);
    async::receive(h, data, std::strlen(data));
    async::disconnect(h);


    //only one multiply
    /*std::size_t bulk = 3;
    auto h1 = async::connect(bulk);
    auto h2 = async::connect(bulk);
    auto h3 = async::connect(bulk);
    char data[] = "0\n1\n2\n3\n";
    for(auto c: data)
    {
        auto cc = new decltype(c){c};
        async::receive(h1, cc, sizeof(*cc));
        async::receive(h2, cc, sizeof(*cc));
        async::receive(h3, cc, sizeof(*cc));
        delete cc;
    }
    async::disconnect(h1);
    async::disconnect(h2);
    async::disconnect(h3);*/

    //only one
    /*std::size_t bulk = 3;
    auto h = async::connect(bulk);
    char data[] = "0\n1\n2\n3\n";
    for(auto c: data)
    {
        auto cc = new decltype(c){c};
        async::receive(h, cc, sizeof(*cc));
        delete cc;
    }
    async::disconnect(h);*/

    /*std::size_t bulk = 5;
    auto h = async::connect(bulk);
    auto h2 = async::connect(bulk);
    async::receive(h, "1", 1);
    async::receive(h2, "1\n", 2);
    async::receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
    async::receive(h, "b\nc\nd\n}\n89\n", 11);
    async::disconnect(h);
    async::disconnect(h2);*/

    //simple one thread
    /*std::size_t bulk = 3;
    auto h = async::connect(bulk);
    async::receive(h, "1\n2\n3\n", 3);
    async::disconnect(h);
     */

    //legacy
    /*auto bulk = std::make_shared<Bulkmt>(3);
    BulkImpl impl(bulk);

    bulk->ExecuteAll("1\n2\n3\n", 3);

    impl.Complete();*/

    return 0;
}
