#include "delay.h"

Delay::Delay(simtime_picosec period, uint32_t delay_us, EventList& eventlist)
    : EventSource(eventlist, "Delay"), m_period(period), m_delay(delay_us)
{
    eventlist.sourceIsPending(*this, period);
}


void Delay::doNextEvent()
{
    eventlist().sourceIsPendingRel(*this, m_period);
    usleep(m_delay);
}