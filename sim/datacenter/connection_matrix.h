// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef connection_matrix
#define connection_matrix

#include "main.h"
#include "tcp.h"
#include "topology.h"
#include "randomqueue.h"
#include "fat_tree_switch.h"
#include "eventlist.h"
#include <list>
#include <map>

#define NO_START ((simtime_picosec)0xffffffffffffffff)

/* 一个连接
 * src: 源
 * dst: 目的地
 */
struct connection{
    int src, dst, size;
    flowid_t flowid; 
    triggerid_t send_done_trigger;
    triggerid_t recv_done_trigger;
    triggerid_t trigger;
    simtime_picosec start;
    int priority;
};

typedef enum {UNSPECIFIED, SINGLE_SHOT, MULTI_SHOT, BARRIER} trigger_type;

// hold (temporary) state to set up trigger
struct trigger {
    triggerid_t id;
    trigger_type type;
    int count; // used for barriers
    vector <flowid_t> flows; // flows to be triggered by this trigger
    Trigger *trigger;  // the actual trigger
};

//describe link failures
struct failure {
    FatTreeSwitch::switch_type switch_type;
    uint32_t switch_id;
    uint32_t link_id;
};


/* ConnectionMatrix: 给定 HOST 数量, 随机生成一定数量的 Connection 关系
 * triggers: map [id -> trigger]
 * connections: map [src -> vector*], vector->dest, 根据 src 索引 vector, vector 中存储了 dest
 */
class ConnectionMatrix{
public:
    ConnectionMatrix(uint32_t );        // 指定 server 数量

    /* 添加 Connection 到 connections */
    void addConnection(uint32_t src, uint32_t dest);

    /* 为 server 构建 connection [指定数量 / Incast] */
    void setPermutation(uint32_t conn);                         // setPermutation(conn, 1) [rack_size 无效]
    void setPermutation(uint32_t conn, uint32_t rack_size);     // 创建 conn 个 connection (src != dst, dst 不重复), rack_size 似乎不起作用, conn 可大于 N
    void setPermutation();                                      // 创建   N  个 connection (src != dst, dst 不重复)
    void setPermutationShuffle(uint32_t conn);                  // 采用洗牌算法, 创建 conn 个 connection (src != dst, dst 不重复), 限制 conn 小于等于 N
    void setPermutationShuffleIncast(uint32_t conn);            // 采用洗牌算法, 创建 conn 个 connection (src != dst, dst 不重复), 剩余 N-conn 个 src 向同一个 dst 发送
    void setRandom(uint32_t conns);                             // 随机设置 conns 个 connection, 无额外约束, src != 0, dst != N-1
    void setStride(uint32_t many);                              // 创建 conn 个 connection (src, (dest+N/2) % N), conns 小于等于 N
    
    
    void setLocalTraffic(Topology* top);
    void setStaggeredRandom(Topology* top, uint32_t conns, double local);
    void setStaggeredPermutation(Topology* top, double local);
    void setVL2();
    void setHotspot(uint32_t hosts_per_spot, uint32_t count);
    void setHotspotOutcast(uint32_t hosts_per_hotspot, uint32_t count);
    void setIncastLocalPerm(uint32_t hosts_per_hotspot);
  
    void setIncast(uint32_t hosts_per_hotspot, uint32_t center);
    void setOutcast(uint32_t src, uint32_t hosts_per_hotspot, uint32_t center);
    void setManytoMany(uint32_t hosts);

    bool save(const char * filename);
    bool save(FILE*);
    bool load(const char * filename);  
    /*bool load(FILE*);*/
    bool load(istream& file);
  
    vector<connection*>* getAllConnections();       // 获取全部 connection
    Trigger* getTrigger(triggerid_t id, EventList& eventlist);
    void bindTriggers(connection* c, EventList& eventlist);

    uint32_t N;     // server 数量
    vector<connection*>* conns;
    map<uint32_t, vector<uint32_t>*> connections;
    vector<failure*> failures; 
private:
    map<triggerid_t, trigger*> triggers;
};

#endif
