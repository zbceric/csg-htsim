#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <iostream>
#include "eventlist.h"

class Schedule : public EventSource
{
public:
    Schedule(simtime_picosec period, simtime_picosec start, simtime_picosec end, EventList& eventlist); 
    void doNextEvent();
private:
    simtime_picosec _period;    // 调用间隔
    simtime_picosec _start;     // 仿真起始时间
    simtime_picosec _end;       // 仿真结束时间
    int _smallticks;            // 触发次数
};

#endif