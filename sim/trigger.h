// -*- c-basic-offset: 4; indent-tabs-mode: nil -*- 

#ifndef TRIGGER_H
#define TRIGGER_H

/*
 * A trigger is something that gets called when something else
 * completes.  For example, a sender may activate a trigger when it
 * finishes a transfer.  A trigger can then activate something else,
 * such as causing a flow to start.
 *
 * At the moment a trigger is a single shot device.  Once it's
 * triggered and performed its action it's done.
 *
 * Something that can be triggered should inherit from TriggerTarget and implement activate();
 */

#include <vector>
#include "config.h"
#include "network.h"
#include "eventlist.h"

#define TRIGGER_START ((simtime_picosec)-1)
typedef uint32_t triggerid_t;

// Triggers call activate on TriggerTargets to cause them to do something.
class TriggerTarget {
public:
    virtual void activate() = 0;
};

/* Trigger: 由一个 id 标记, 每个 Trigger 包含一系列 TriggerTarget
 * TriggerTarget: 一个 interface, 提供了一个 activate 函数接口, 其派生类调用 activate 激活
 */
class Trigger {
public:
    Trigger(EventList& eventlist, triggerid_t id);
    void add_target(TriggerTarget& target);
    virtual void activate() = 0;
protected:
    EventList &_eventlist;
    triggerid_t _id;
    vector <TriggerTarget*> _targets;
};

// SingleShotTrigger takes one or more trigger targets and activates
// them all on the first call to activate().  Will abort if called
// twice as most targets cannot be restarted.
class SingleShotTrigger: public Trigger {
/* SingleShotTrigger 采用一个或多个触发目标, 并在第一次调用 activate() 时将它们全部激活
 * 如果调用两次将会中止，因为大多数目标无法重新启动。
 */
public:
    SingleShotTrigger(EventList& eventlist, triggerid_t id);
    virtual void activate();
private:
    bool _done;
};

// MultiShotTrigger takes count trigger targets and activates
// them sequentially when its own activate() is called.  Will abort if called
// more than count.
class MultiShotTrigger: public Trigger {
/* MultiShotTrigger 在每次调用触发函数时, 将一个 TriggerTarget 写入 EventList 
 */
public:
    MultiShotTrigger(EventList& eventlist, triggerid_t id);
    virtual void activate();
private:
    uint32_t _next;
};

// BarrierTrigger takes one of more trigger events, and a count of the
// number of activations needed before it triggers.  Needs precisely
// this many activations before it will fire.
class BarrierTrigger: public Trigger {
/* 需要指定次数触发, 随后才会将 TriggerTarget 全部写入 EventList
 */
public:
    BarrierTrigger(EventList& eventlist, triggerid_t id, size_t activations_needed);
    virtual void activate();
private:
    size_t _activations_remaining;
};

#endif

    
