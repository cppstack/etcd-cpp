#include <etcd/etcd.hpp>
#include <iostream>
#include <cst/program_options.hpp>

namespace po = cst::program_options;

namespace {

po::variables_map parse_global_options(int argc, char *const argv[])
{
    po::variables_map vm;
    po::parsed_options parsed;

    po::options_description desc("Usage: etcdcli [options...] command...");

    auto check_command = [](const std::string& cmd) {
        if (cmd != "member" && cmd != "put") {
            std::cerr << "invalid command" << std::endl;
            std::exit(1);
        }
    };

    desc.add_options()
      ("endpoints", po::value<std::string>()->required()->default_value("127.0.0.1:2379"), "etcd end points", "endpoints")
      ("command", po::value<std::string>()->required()->notifier(check_command), "", "", true)
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
            std::cerr << desc
                      << "\nCommands:\n"
                      << "  put            Puts the given key into the store\n"
                      << "  member list    Lists all members in the cluster\n"
                      << std::endl;
            std::exit(0);
        }
    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    const std::string& command = vm["command"].as<std::string>();

    po::options_description desc_;
    po::positional_options_description pos_;

    if (command == "member") {
        desc_.add_options()
          ("subcmd", po::value<std::string>()->required()->notifier(
              [](const std::string& cmd) {
                  if (cmd != "list") {
                      std::cerr << "invalid sub member command" << std::endl;
                      std::exit(1);
                  }
              }), "", "", true);

        pos_.add("subcmd", 1);

    } else if (command == "put") {
        desc_.add_options()
          ("key", po::value<std::string>()->required(), "", "", true)
          ("value", po::value<std::string>()->required(), "", "", true);

        pos_.add("key", 1);
        pos_.add("value", 1);
    }

    try {
        po::store(po::command_line_parser(parsed.unrecognized_options).
                      options(desc_).positional(pos_).run(), vm);
        po::notify(vm);
    } catch (const po::option_error& e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    }

    return vm;
}

} // unnamed namespace

#include "commands.hpp"

int main(int argc, char *const argv[])
{
    po::variables_map vm = parse_global_options(argc, argv);

    etcd::etcd etcd(vm["endpoints"].as<std::string>());

    const std::string& command = vm["command"].as<std::string>();

    if (command == "member")
        do_member(etcd, vm);
    else if (command == "put")
        do_put(etcd, vm);

    return 0;
}
