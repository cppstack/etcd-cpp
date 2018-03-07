#include <etcd/etcd.hpp>
#include <iostream>

int main(int /*argc*/, char *const /*argv*/[])
{
    etcd::etcd etcd("localhost:2379");

    for (const auto& m : etcd.members()) {
        std::cout << m.id() << ' ' << m.name();
        for (const auto& u : m.peer_urls())
            std::cout << ' ' << u;
        for (const auto& u : m.client_urls())
            std::cout << ' ' << u;
        std::cout << std::endl;
    }

    return 0;
}
