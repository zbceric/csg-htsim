#include "delay.h"
#include "progress.h"

EventList eventlist;

int main(int argc, char **argv)
{
    eventlist.setEndtime(timeFromSec(4));
    Delay d(timeFromUs(10u), 10, eventlist);  
    Progress p(0, timeFromSec(4), eventlist);

    while(eventlist.doNextEvent()) {}
    return 0;
}