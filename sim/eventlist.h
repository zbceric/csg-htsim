// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-
#ifndef EVENTLIST_H
#define EVENTLIST_H

#include <map>
#include <sys/time.h>
#include "config.h"
#include "loggertypes.h"

class EventList;
class TriggerTarget;

class EventSource : public Logged {
public:
    EventSource(EventList& eventlist, const string& name) : Logged(name), _eventlist(eventlist) {};
    EventSource(const string& name);
    virtual ~EventSource() {};
    virtual void doNextEvent() = 0;
    inline EventList& eventlist() const {return _eventlist;}
protected:
    EventList& _eventlist;
};


/* multimap:  红黑树管理的键值对, 允许插入重复的 key, 因此 key-value 的映射是多对多的;
 * Handler:   multimap <simtime(ps), EventSource> 的迭代器;
 * EventList: 管理每个时间点的事件, 每个时间点可以注册多个事件, 由一个静态的变量全局管理
 *            派生类同样会将事件注册到静态变量 _pendingsources;
 *            此外, 维护了一个静态 vector 管理待处理的 TriggerTarget
 */
class EventList {
public:
    typedef multimap <simtime_picosec, EventSource*>::iterator Handle;
    EventList();
    static void setEndtime(simtime_picosec endtime); // end simulation at endtime (rather than forever)
    static bool doNextEvent(); // returns true if it did anything, false if there's nothing to do
    static void sourceIsPending(EventSource &src, simtime_picosec when);
    static Handle sourceIsPendingGetHandle(EventSource &src, simtime_picosec when);
    static void sourceIsPendingRel(EventSource &src, simtime_picosec timefromnow)
    { sourceIsPending(src, EventList::now()+timefromnow); }
    static void cancelPendingSource(EventSource &src);
    // optimized cancel, if we know the expiry time
    static void cancelPendingSourceByTime(EventSource &src, simtime_picosec when);   
    // optimized cancel by handle - be careful to ensure handle is still valid
    static void cancelPendingSourceByHandle(EventSource &src, Handle handle);       
    static void reschedulePendingSource(EventSource &src, simtime_picosec when);
    static void triggerIsPending(TriggerTarget &target);
    static inline simtime_picosec now() {return EventList::_lasteventtime;}
    static Handle nullHandle() {return _pendingsources.end();}


    static EventList& getTheEventList();
    EventList(const EventList&)      = delete;  // disable Copy Constructor
    void operator=(const EventList&) = delete;  // disable Assign Constructor

private:
    static simtime_picosec _endtime;            // 仿真结束时间 
    static simtime_picosec _lasteventtime;      // 最后时间时间
    typedef multimap <simtime_picosec, EventSource*> pendingsources_t;
    static pendingsources_t _pendingsources;    // 时间到事件源的映射
    static vector <TriggerTarget*> _pending_triggers;   // 待处理的 TriggerTarget 数组

    static int _instanceCount;
    static EventList* _theEventList;
};

#endif
