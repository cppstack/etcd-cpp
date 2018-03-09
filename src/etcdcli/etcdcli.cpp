#include <etcd/etcd.hpp>
#include <iostream>
#include "program_options.hpp"

namespace po = cst::program_options;

namespace {

void parse_options(int argc, char *const argv[], po::variables_map& vm)
{
    po::options_description desc("etcdcli [options...] command...");
    po::positional_options_description pos_desc;

    desc.add_options()
      ("endpoints", po::value<std::string>()->default_value("127.0.0.1:2379"), "etcd end points", "endpoints")
      ("command", po::value<std::vector<std::string>>()->required(), "", "", true)
      ("help,h", "display help text");

    pos_desc.add("command", -1);

    try {
        po::store(parse_command_line(argc, argv, desc, pos_desc), vm);

        if (vm.count("help")) {
            std::cerr << desc << "\n"
                      << "Commands:\n"
                      << "  put            Puts the given key into the store\n"
                      << "  member list    Lists all members in the cluster\n";
            std::exit(0);
        }

        po::notify(vm);

    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }
}

void do_member_list(etcd::etcd& etcd)
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

void do_member(etcd::etcd& etcd, const std::vector<std::string>& cmd)
{
    if (cmd.size() < 2)
        throw std::invalid_argument("missing sub command for 'member'");

    if (cmd[1] == "list")
        do_member_list(etcd);
    else
        throw std::invalid_argument("unknown sub command for 'member'");
}

void do_put(etcd::etcd& etcd, const std::vector<std::string>& cmd)
{
    if (cmd.size() < 3)
        throw std::invalid_argument("missing key or value for command 'put'");

    etcd.put(cmd[1], cmd[2]);

    std::cout << "OK\n";
}

} // unnamed namespace

int main(int argc, char *const argv[])
{
    po::variables_map vm;
    parse_options(argc, argv, vm);

    etcd::etcd etcd(vm["endpoints"].as<std::string>());

    const auto& cmd = vm["command"].as<std::vector<std::string>>();

    if (cmd[0] == "member")
        do_member(etcd, cmd);
    else if (cmd[0] == "put")
        do_put(etcd, cmd);
    else
        throw std::invalid_argument("unknown command");

    return 0;
}
