// -*- c-basic-offset: 4; indent-tabs-mode: nil -*-        

#include "eventlist.h"
#include "trigger.h"

simtime_picosec EventList::_endtime = 0;
simtime_picosec EventList::_lasteventtime = 0;
EventList::pendingsources_t EventList::_pendingsources;
vector <TriggerTarget*> EventList::_pending_triggers;
int EventList::_instanceCount = 0;
EventList* EventList::_theEventList = nullptr;

EventList::EventList()
{
    if (EventList::_instanceCount != 0) 
    {
        std::cerr << "There should be only one instance of EventList. Abort." << std::endl;
        abort();
    }

    EventList::_theEventList = this;
    EventList::_instanceCount += 1;
}

EventList& 
EventList::getTheEventList()
{
    if (EventList::_theEventList == nullptr) 
    {
        EventList::_theEventList = new EventList();
    }
    return *EventList::_theEventList;
}

void
EventList::setEndtime(simtime_picosec endtime)
{
    EventList::_endtime = endtime;
}

/* 从全局的 _pending_triggers 的末尾取出一个 activate (类似 stack)
 * 从全局的 _pendingsources 中取出最早的那个事件, 调用其 doNextEvent 函数
 * main 函数中直接调用 while(EventList::doNextEvent()){}
 * 先处理 trigger, 没有 trigger 就跟新时间, 取出新的 eventsource, 调用其 doNextEvent();
 */
bool
EventList::doNextEvent() 
{
    // triggers happen immediately - no time passes; no guarantee that
    // they happen in any particular order (don't assume FIFO or LIFO).
    if (!_pending_triggers.empty()) {
        TriggerTarget *target = _pending_triggers.back();
        _pending_triggers.pop_back();
        target->activate();
        return true;
    }
    
    if (_pendingsources.empty())
        return false;
    
    simtime_picosec nexteventtime = _pendingsources.begin()->first;
    EventSource* nextsource = _pendingsources.begin()->second;
    _pendingsources.erase(_pendingsources.begin());
    assert(nexteventtime >= _lasteventtime);
    _lasteventtime = nexteventtime; // set this before calling doNextEvent, so that this::now() is accurate
    nextsource->doNextEvent();
    return true;
}


/* 向 _pendingsources 插入一个 EventSource, 先检查 event 的结束时间是否在仿真停止时间之前 */
void 
EventList::sourceIsPending(EventSource &src, simtime_picosec when) 
{
    assert(when>=now());
    if (_endtime==0 || when<_endtime)
        _pendingsources.insert(make_pair(when,&src));
}

EventList::Handle
EventList::sourceIsPendingGetHandle(EventSource &src, simtime_picosec when) 
{
    assert(when>=now());
    if (_endtime==0 || when<_endtime) {
        EventList::Handle handle =_pendingsources.insert(make_pair(when,&src));
        return handle;
    }
    return _pendingsources.end();
}

/* 增加一个未处理的 TriggerTarget 到 _pending_triggers */
void
EventList::triggerIsPending(TriggerTarget &target) {
    _pending_triggers.push_back(&target);
}

/* 删除全部和 src 相关的 EventSource */
void 
EventList::cancelPendingSource(EventSource &src) {
    pendingsources_t::iterator i = _pendingsources.begin();
    while (i != _pendingsources.end()) {
        if (i->second == &src) {
            _pendingsources.erase(i);
            return;
        }
        i++;
    }
}

/* 删除某个时间点上的全部 EventSource */
void 
EventList::cancelPendingSourceByTime(EventSource &src, simtime_picosec when) {
    // fast cancellation of a timer - the timer MUST exist
    // this should normally be fast, except if we have a lot of events with exactly the same time value

    auto range = _pendingsources.equal_range(when);

    for (auto i = range.first; i != range.second; ++i) {
        if (i->second == &src) {
            _pendingsources.erase(i);
            return;
        }
    }
    abort();
}

/* 删除迭代器对应的 EventSource */
void EventList::cancelPendingSourceByHandle(EventSource &src, EventList::Handle handle) {
    // If we're cancelling timers often, cancel them by handle.  But
    // be careful - cancelling a handle that has already been
    // cancelled or has already expired is undefined behaviour
    assert(handle->second == &src);
    assert(handle != _pendingsources.end());
    assert(handle->first >= now());
    
    _pendingsources.erase(handle);
}

/* 移除 EventSource, 然后根据新的时间插入到 _pendingsources */
void 
EventList::reschedulePendingSource(EventSource &src, simtime_picosec when) {
    cancelPendingSource(src);
    sourceIsPending(src, when);
}

EventSource::EventSource(const string& name) : EventSource(EventList::getTheEventList(), name) 
{
}
