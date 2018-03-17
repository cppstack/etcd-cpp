#include <etcd/etcd.hpp>
#include <iostream>
#include "cst/program_options.hpp"

namespace po = cst::program_options;

namespace {

void parse_global_options(int argc, char *const argv[],
                          po::parsed_options& parsed, po::variables_map& vm)
{
    po::options_description desc("Usage: etcdcli [options...] command...");

    desc.add_options()
      ("endpoints", po::value<std::string>()->required()->default_value("127.0.0.1:2379"), "etcd end points", "endpoints")
      ("command", po::value<std::string>()->required(), "", "", true)
      ("help,h", "display help text");

    po::positional_options_description pos;
    pos.add("command", 1);

    try {
        parsed = po::command_line_parser(argc, argv).
                     options(desc).
                     positional(pos).
                     allow_unregistered().
                     run();

        po::store(parsed, vm);

        if (vm.count("help") || !vm.count("command")) {
            std::cerr << desc << "\n"
                      << "Commands:\n"
                      << "  put            Puts the given key into the store\n"
                      << "  member list    Lists all members in the cluster\n";
                      << std::endl;
            std::exit(0);
        }
    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }
}

void do_member_list(etcd::etcd& etcd, po::variables_map&)
{
    for (const auto& m : etcd.members()) {
        std::cout << m.id() << ' ' << m.name();
        for (const auto& u : m.peer_urls())
            std::cout << ' ' << u;
        for (const auto& u : m.client_urls())
            std::cout << ' ' << u;
        std::cout << std::endl;
    }
}

void do_member(po::parsed_options& parsed, po::variables_map& vm)
{
    std::string subcmd;

    po::options_description desc("member options");
    desc.add_options()
      ("subcmd", po::value<std::string>(&subcmd)->required(), "", "", true);

    po::positional_options_description pos;
    pos.add("subcmd", 1);

    try {
        po::store(po::command_line_parser(parsed.unrecognized_options).
                      options(desc).positional(pos).run(), vm);
        po::notify(vm);
    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    etcd::etcd etcd(vm["endpoints"].as<std::string>());

    if (subcmd == "list")
        do_member_list(etcd, vm);
    else
        throw std::invalid_argument("unknown sub command for 'member'");
}

void do_put(po::parsed_options& parsed, po::variables_map& vm)
{
    po::options_description desc("put options");
    desc.add_options()
      ("key", po::value<std::string>()->required(), "", "", true)
      ("value", po::value<std::string>()->required(), "", "", true);

    po::positional_options_description pos;
    pos.add("key", 1);
    pos.add("value", 1);

    try {
        po::store(po::command_line_parser(parsed.unrecognized_options).
                      options(desc).positional(pos).run(), vm);
        po::notify(vm);
    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    etcd::etcd etcd(vm["endpoints"].as<std::string>());

    etcd.put(vm["key"].as<std::string>(), vm["value"].as<std::string>());

    std::cout << "OK" << std::endl;
}

} // unnamed namespace

int main(int argc, char *const argv[])
{
    po::parsed_options parsed;
    po::variables_map vm;
    parse_global_options(argc, argv, parsed, vm);

    const std::string& command = vm["command"].as<std::string>();

    if (command == "member")
        do_member(parsed, vm);
    else if (command == "put")
        do_put(parsed, vm);
    else
        throw std::invalid_argument("unknown command");

    return 0;
}
