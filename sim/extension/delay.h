#ifndef DELAY_H
#define DELAY_H

#include <iostream>
#include <chrono>
#include <unistd.h>

#include "eventlist.h"


class Delay : public EventSource
{
public:
    Delay(simtime_picosec period, uint32_t delay_us, EventList& eventlist); 
    void doNextEvent();
private:
    simtime_picosec     m_period;
    uint32_t            m_delay;
    uint64_t            m_smallticks;
};

#endif