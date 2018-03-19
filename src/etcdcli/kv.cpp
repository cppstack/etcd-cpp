#include <etcd/etcd.hpp>
#include <iostream>
#include <cst/program_options.hpp>

namespace po = cst::program_options;

void do_put(etcd::etcd& etcd, po::variables_map& vm)
{
    etcd.put(vm["key"].as<std::string>(), vm["value"].as<std::string>());

    std::cout << "OK" << std::endl;
}
