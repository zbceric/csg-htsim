#ifndef PROGRESS_H
#define PROGRESS_H

#include <iostream>
#include <cstdio>
#include <chrono>
#include <deque>
#include <regex>
#include <unordered_map>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <unordered_set>
#include "eventlist.h"

#include "progress_utils.h"



/* timepoint in real world */
typedef std::chrono::time_point<std::chrono::system_clock> systime_point;
/* nanosecond in real world */
typedef std::chrono::duration<long, std::ratio<1, 1000000000>> systime_nanosec;

/* reference: https://github.com/tqdm/tqdm.git */
class Progress : public EventSource
{
public:
    // Progress(simtime_picosec period, EventList& eventlist, string prompt=string("[Timeline Progress]")); 
    Progress(simtime_picosec start, simtime_picosec end, EventList& eventlist, 
             simtime_picosec period = timeFromMs(10), int fd = STDERR_FILENO, uint32_t iter_min = 0, 
             systime_nanosec min_interval = systime_nanosec(0), systime_nanosec max_interval = systime_nanosec(0),
             Bar::CHARSET ascii = Bar::UTF, Bar::COLOUR colour = Bar::NONE_COLOUR, double smoothing = 0.3);
    virtual void doNextEvent();

    static std::unordered_set<Progress*>  m_progress_set;
private:
    int             m_fd;               // !< STDERR_FILENO or STDOUT_FILENO
    FILE*           m_file;             // !< Standard stream (stderr or stdout).
    
    simtime_picosec m_sim_period;       // !< Simulation time delay between iterations.
    simtime_picosec m_sim_stt;          // !< Simulation start time.
    simtime_picosec m_sim_end;          // !< Simulation end time.
    simtime_picosec m_sim_lst;          // !< Simulation duration.
    simtime_picosec m_sim_now;          // !< Simulation current time.

    systime_point   m_sys_stt;          // !< The system time when simulation starts in the real world.
    systime_point   m_sys_lst;          // !< The system time when last printed in the real world.
    systime_point   m_sys_now;          // !< The system current time in the real world.

    systime_nanosec m_sys_dp_interval;  // !< 
    systime_nanosec m_sys_min_interval; // !<
    systime_nanosec m_sys_max_interval; // !< 
    systime_nanosec m_sys_elapsed;      // !< The time elapsed in the real world since simulation starts.
    systime_nanosec m_sys_remain;       // !< 

    uint32_t        m_iter_tot;         // !< The number of expected iterations.
    uint32_t        m_iter_cur;         // !< Curent iteration.
    uint32_t        m_iter_lst;         // !< The iteration when last printed.
    uint32_t        m_iter_min;         // !< The minimum iteration for printing intervals.

    EMA             m_ema_iter;         // !< Exponentially moving average smoothed iteration.
    EMA             m_ema_time;         // !< Exponentially moving average smoothed system time.
    EMA             m_ema_min_iter;     // !< 
    double          m_smoothing;        // !< Exponential moving average smoothing factor for speed estimates. Range from 0 to 1 [default: 0.3].
    double          m_rate;
    double          m_inv_rate;

    int32_t         m_pos;              // !< The row to display.
    int32_t         m_last_len;         // !< The length of the string output when last printed.
    winsize         m_win_size;         // !< The screen size.

    static std::regex m_regex_ansi;     // !< Regular expression to match ANSI escape codes

    Bar             m_bar;              // !< Progress Bar.

    bool            m_dynamic_iter;

    /* Update and possibly print the progressbar. */
    void update             ();
    /* Move the cursor given pos. */
    void move_to_pos        (int pos);
    /* Print something. */
    void status_print       (const std::string& data);
    /* Force refresh the display of this bar. */
    bool display(string msg=string());
    /* Get the length of the string displayed on the screen (without ANSI). */
    int  disp_len           (const std::string& data);
    /* Get the width of the string displayed on the screen. */
    int  text_width         (const std::string& data);
    /* Trim a string which may contain ANSI control characters. */
    std::string disp_trim   (const std::string& data, size_t length);
    /* Return a string-based progress bar given some parameters */
    std::string format_meter(std::string l_bar, std::string r_bar);
    /* Return a free positon to deplay progress bar */
    static int32_t get_free_pos();

public:
    /* Return console dimensions (width, height) */
    static winsize get_screen_shape(int fd);
    /* Formats a number of seconds as a clock time, [H:]MM:SS */
    static std::string format_interval(systime_nanosec t);
    /* Formats a number (greater than unity) with SI Order of Magnitude prefixes.*/
    static std::string format_sizeof(double num, std::string suffix = std::string(), int divisor = 1000);
    /* Formats a string with vsnprintf */
    static std::string format_string(const char* format, ...);
};

#endif