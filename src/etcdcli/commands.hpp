#ifndef _ETCDCLI_COMMANDS_HPP
#define _ETCDCLI_COMMANDS_HPP

void do_member(etcd::etcd& etcd, po::variables_map& vm);

void do_put(etcd::etcd& etcd, po::variables_map& vm);

#endif
