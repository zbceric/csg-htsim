// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef TOPOLOGY
#define TOPOLOGY
#include "network.h"
#include "loggers.h"

/* Topology: 网络拓扑结构的 interface
 * 提供的接口
 * get_paths: input(src, dst), outout(一个 vector 指针, vector 中存储了路由路径)
 * get_bidir_paths: input(src, dst, reverse), output(一个 vector 指针, vector 中存储了路由路径)
 * no_of_nodes: output(node 数量)
 * add_switch_loggers: 添加交换机的 logger
 */
class Topology {
public:
    virtual vector<const Route*>* get_paths(uint32_t src, uint32_t dest) {
        return get_bidir_paths(src, dest, true);
    }
    virtual vector<const Route*>* get_bidir_paths(uint32_t src, uint32_t dest, bool reverse)=0;
    virtual vector<uint32_t>* get_neighbours(uint32_t src) = 0;  
    virtual uint32_t no_of_nodes() const {
        abort();
    }

    // add loggers to record total queue size at switches
    virtual void add_switch_loggers(Logfile& log, simtime_picosec sample_period) {
        abort();
    }
};

#endif
