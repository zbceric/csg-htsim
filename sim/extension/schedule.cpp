#include "schedule.h"

/* 将 schedule 加入 eventlist */
Schedule::Schedule(simtime_picosec period, simtime_picosec start, simtime_picosec end, EventList& eventlist)
  : EventSource(eventlist, "Schedule"), 
    _period(period), _start(start), _end(end), _smallticks(0)
{
    eventlist.sourceIsPendingRel(*this, period);        // 将 Schedule 挂载到 eventlist

}


/* eventlist 执行 schedule 后调用 doNextEvent,
 * 再次将 schedule 加入 doNextEvent
 */
void Schedule::doNextEvent() {
    eventlist().sourceIsPendingRel(*this, _period);
    _smallticks++;

    simtime_picosec now = eventlist().now();

    if (now < _start)
        return;

    stringstream str;
    str << "Schedule::now() == ";
    str << itoa(now / 1000 / 1000 / 1000);
    str << "ms [" << 1.0 * (now - _start) / (_end - _start) * 100 << "%" << "]";

    str << " _end = " << itoa(_end / 1000 / 1000 / 1000) << "ms, _start = " << itoa(_start / 1000 / 1000 / 1000) << "ms";

    std::cout << str.str() << std::endl;
}
