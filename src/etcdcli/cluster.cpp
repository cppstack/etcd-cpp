#include <etcd/etcd.hpp>
#include <iostream>
#include <cst/program_options.hpp>

namespace po = cst::program_options;

namespace {

void do_member_list(etcd::etcd& etcd, po::variables_map&)
{
    auto members = etcd.member_list().members();

    for (const auto& m : members) {
        std::cout << m.id() << ' ' << m.name();
        for (const auto& u : m.peerurls())
            std::cout << ' ' << u;
        for (const auto& u : m.clienturls())
            std::cout << ' ' << u;
        std::cout << std::endl;
    }
}

}

void do_member(etcd::etcd& etcd, po::variables_map& vm)
{
    const std::string& subcmd = vm["subcmd"].as<std::string>();

    if (subcmd == "list")
        do_member_list(etcd, vm);
}
