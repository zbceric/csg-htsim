#include "progress.h"


/* static member of class Progress. */
std::regex Progress::m_regex_ansi("\\x1b\\[[;\\d]*[A-Za-z]");
std::unordered_set<Progress*> Progress::m_progress_set;

/* Constructor of class Progress.
 * Parameters:
 *  period: simulation time delay [default: 10ms].
 *  file: stderr or stdout [default: stderr].
 * 
 */
Progress::Progress(simtime_picosec start, simtime_picosec end, EventList& eventlist, simtime_picosec period, int fd, uint32_t iter_min,
    systime_nanosec min_interval, systime_nanosec max_interval, Bar::CHARSET ascii, Bar::COLOUR colour, double smoothing)
  : EventSource(eventlist, "Progress"),
    m_fd(fd), m_sim_period(period), m_sim_stt(start), m_sim_end(end), 
    m_sys_min_interval(min_interval), m_sys_max_interval(max_interval),
    m_iter_min(iter_min)
{
    eventlist.sourceIsPendingRel(*this, start);        // 将 Progress 挂载到 eventlist

    if (fd == STDOUT_FILENO)
        m_file = stdout;
    else
        m_file = stderr;

    m_iter_tot = (m_sim_end - m_sim_stt) / m_sim_period;
    m_win_size = get_screen_shape(STDOUT_FILENO);

    if (m_iter_min == 0)
        m_dynamic_iter = true;
    else
        m_dynamic_iter = false;

    m_bar = Bar(m_win_size.ws_col, ascii, colour);
    assert(smoothing > 0);
    assert(smoothing < 1);
    m_ema_iter = EMA(smoothing);
    m_ema_time = EMA(smoothing);
    m_ema_min_iter = EMA(smoothing);

    m_iter_cur = 0;
    m_iter_lst = 0;

    m_pos = get_free_pos();
    m_progress_set.insert(this);
}

/* 打印 progress bar */
void Progress::doNextEvent()
{   
    m_sim_now = eventlist().now();
    m_sys_now = std::chrono::system_clock::now();
    
    assert(m_sim_now >= m_sim_stt);
    assert(m_sim_now <= m_sim_end);

    if (m_iter_cur == 0) {              // Init
        m_sim_stt = m_sim_now;
        m_sim_lst = m_sim_now;
        m_sys_stt = m_sys_now;
        m_sys_lst = m_sys_now;
        m_sys_elapsed = systime_nanosec(0);

        m_rate = -1;
        m_inv_rate = -1;

        display();
    }

    if (m_iter_cur - m_iter_lst > m_iter_min) {
        auto dt = std::chrono::system_clock::now() - m_sys_lst;
        if (dt > m_sys_min_interval)
            this->update();
    }

    m_iter_cur++;
    eventlist().sourceIsPendingRel(*this, m_sim_period);
}

void Progress::update()
{
    uint32_t dn = m_iter_cur - m_iter_lst;
    if (dn >= m_iter_min) {
        auto dt = std::chrono::system_clock::now() - m_sys_lst;
        if (dt > m_sys_min_interval) {
            m_ema_iter.update(dn);
            m_ema_time.update(dt.count());
        }

        display();

        if (m_dynamic_iter) {
            if (m_sys_max_interval.count() && dt > m_sys_max_interval) {
                m_iter_min = dn * (m_sys_min_interval.count() ? m_sys_min_interval : m_sys_max_interval) / dt;
            }
            else if (m_smoothing > 0) {
                m_iter_min = m_ema_min_iter.update(dn * (m_sys_min_interval / (m_sys_min_interval.count() && dt.count() ? dt : systime_nanosec(1000000000))));
            }
            else {
                m_iter_min = max(m_iter_min, dn);
            }
        }
    }

    m_iter_lst = m_iter_cur;
    m_sys_lst  = m_sys_now;
}

/* Force refresh the display of this bar.
 * LOCK ???
 */
bool Progress::display(string msg)
{
    int32_t pos = abs(m_pos);
    if (pos == m_win_size.ws_row - 1) {     // 显示到最后一行了, 输出提示
        msg = std::string(" ... (more hidden) ...");
    }
    else if (pos >= m_win_size.ws_row) {    // 超出最后一行, 不再显示
        return false;
    }

    if (pos)
        move_to_pos(pos);
    if (msg.empty())
        status_print(str());
    else
        status_print(msg);
    if (pos)
        move_to_pos(-pos);
    return true;
}


/* Return a string-based progress bar given some parameters
 */
std::string Progress::format_meter(std::string l_bar, std::string r_bar)
{
    // if (m_dynamic_ncols)
    //     m_win_size = get_screen_shape(STDOUT_FILENO);

    assert(m_iter_cur <= m_iter_tot);
    std::string elapsed_str = format_interval(m_sys_elapsed);

    if (m_rate < 0 && m_sys_elapsed.count() != 0) {
        m_rate = 1.0 * m_iter_cur / (m_sys_elapsed.count() / 1000000000.0);     // it/s
        m_inv_rate = 1.0 * m_sys_elapsed.count() / m_iter_cur;                  // s/it
        m_sys_remain = (m_iter_tot - m_iter_cur) / m_iter_cur  * m_sys_elapsed;
    }

    std::string rate_fmt     = (m_rate     < 0 ? std::string("?") : format_string("%5.2f", m_rate))     + "it/s";
    std::string rate_inv_fmt = (m_inv_rate < 0 ? std::string("?") : format_string("%5.2f", m_inv_rate)) + "s/it";
    std::string n_fmt     = format_string("%llu", m_iter_cur);
    std::string total_fmt = format_string("%llu", m_iter_tot);

    std::string remain_fmt = m_rate < 0 ? "?" : format_interval(m_sys_remain);
    
    // systime_point eta_dt = m_rate < 0 ? systime_point() : std::chrono::system_clock::now() + m_sys_remain;


    double frac = 1.0 * m_iter_cur / m_iter_tot;
    double percentage = frac * 100;
    l_bar += format_string("%3.0f|", percentage);
    
    if (m_win_size.ws_col == 0)
        return l_bar.substr(0, l_bar.length() - 1) + r_bar.substr(1, l_bar.length() - 1);

    std::string nobar = l_bar + r_bar;
    std::string full_bar = m_bar.update(frac, m_win_size.ws_col ? max(1, m_win_size.ws_col - disp_len(nobar)): 10);
    std::string result = l_bar + full_bar + r_bar;
    return m_win_size.ws_col ? disp_trim(result, m_win_size.ws_col) : result;
}


/* Trim a string which may contain ANSI control characters.
 * 修建可能包含 ANSI 控制字符的 string
 */
std::string Progress::disp_trim (const std::string& data, size_t length)
{
    // 不包含 ANSI 字符串, 返回长度为 length 的子串
    if (static_cast<int>(data.length()) == disp_len(data))
        return data.substr(0, length);

    bool ansi_present = std::regex_search(data, m_regex_ansi);
    
    std::string data_m = data;
    while (disp_len(data_m) > static_cast<int>(length))
    {
        data_m = data_m.substr(0, data_m.length() - 1);
    }

    std::string end("\033[0m");
    if (ansi_present && std::regex_search(data_m, m_regex_ansi)) {
        bool same = end.compare(data_m.substr(data_m.length() - end.length(), end.length()));
        return same ? data_m : data_m + "\033[0m";
    }
    return data_m;
}


string repeat_string(int n, string str)
{
    assert(n >= 0);
    string s(str);
    for (int i = 1; i < n; ++i)
        s = s + str;
    return n == 0 ? "" : s;
}

/* 移动光标
 * pos 为正数, 向下移动 pos 行
 * pos 为负数, 向上移动 pos 行
 */
void Progress::move_to_pos(int pos)
{
    assert(m_file != nullptr);

    if (pos > 0)
        fprintf(m_file, "%s", string(pos, '\n').c_str());
    else if (pos < 0)
        fprintf(m_file, "%s", repeat_string(-pos, "\x1b[A").c_str());
  
    fflush(m_file);
}


/* 计算 string 的显示宽度 (去除 ANSI 控制码) */
int Progress::disp_len(const std::string& data)
{
    std::string sanitized_data = std::regex_replace(data, m_regex_ansi, "");  // 去除 ANSI 控制码
    return text_width(sanitized_data);
}


/* 计算 string 的显示宽度 (遍历字符串计算长度) */
int Progress::text_width(const std::string& data)
{
    int width = 0;
    for (const char& c : data) {
        wchar_t wc;
        std::mbstate_t state = std::mbstate_t();
        int bytes = std::mbrtowc(&wc, &c, 1, &state);
        if (bytes > 0) {
            width += wcwidth(wc);
        }
    }
    return width;
}

/* 输出一行 */
void Progress::status_print(const std::string& data)
{
    assert(m_file != nullptr);

    int len = disp_len(data);

    if (m_last_len - len > 0)
        fprintf(m_file, "\r%s%s", data.c_str(), string(m_last_len - len, ' ').c_str()); // cover
    else
        fprintf(m_file, "\r%s", data.c_str());
    fflush(m_file);

    m_last_len = len;
}


/* Formats a number of seconds as a clock time, [H:]MM:SS
 * Parameters
 * ----------
 * t  : int
 *      Number of seconds.
 * 
 * Returns
 * -------
 * out  : str
 *      [H:]MM:SS
 */
std::string Progress::format_interval(systime_nanosec t)
{
    int count = t.count() / 1000000000;
    int min   = count / 60;
    int sec   = count % 60;
    int hour  = min / 60;

    char buff[60];
    if (hour != 0) {
        sprintf(buff, "%d:%02d:%02d", hour, min % 60, sec);
    } else {
        sprintf(buff, "%02d:%02d", min % 60, sec);
    }

    return std::string(buff);
}


/* Formats a number (greater than unity) with SI Order of Magnitude prefixes.
 * Parameters
 * ----------
 * num  : float
 *     Number ( >= 1) to format.
 * suffix  : str, optional
 *     Post-postfix [default: std::string()].
 * divisor  : float, optional
 *     Divisor between prefixes [default: 1000].
 * Returns
 * -------
 * out  : str
 *     Number with Order of Magnitude SI unit postfix.
 */
std::string Progress::format_sizeof(double num, std::string suffix, int divisor)
{   
        static char unit[8][2] = {"", "k", "M", "G", "T", "P", "E", "Z"};
    char buff[20];
    int i = 0;

    for (i = 0; i < 8; ++i)
    {
        if (num < 999.5) {
            if (num < 99.95) {
                if (num <9.995) {
                    snprintf(buff, 20, "%1.2f%s%s", num, unit[i], suffix.c_str());
                    return std::string(buff);
                }
                snprintf(buff, 20, "%2.1f%s%s", num, unit[i], suffix.c_str());
                return std::string(buff);
            }
            snprintf(buff, 20, "%3.0f%s%s", num, unit[i], suffix.c_str());
            return std::string(buff);
        }
        num /= divisor;
    }

    snprintf(buff, 20, "%3.1fY%s", num, suffix.c_str());
    return std::string(buff);
}

/* 获取 Screen 的尺寸
 * fd:
 *     STDIN_FILENO:  标准输入
 *     STDOUT_FILENO: 标准输出
 */
winsize Progress::get_screen_shape(int fd)
{
    winsize ws;
    // if (ioctl(fd, TIOCGWINSZ, &ws) != -1) {
    //     int rows = ws.ws_row;
    //     int cols = ws.ws_col;
    //     std::cout << "Terminal Size: " << cols << " columns x " << rows << " rows" << std::endl;
    // }
    int ret = ioctl(fd, TIOCGWINSZ, &ws);
    assert(ret != -1);
    return ws;
}


std::string Progress::format_string(const char* format, ...)
{
    va_list args, args_copy;
    va_start (args, format);
    va_copy  (args_copy, args);
    const int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    std::string res(size + 1, '\0');
    std::vsnprintf(&res[0], res.size(), format, args);

    va_end(args);

    return res;
}

int32_t Progress::get_free_pos()
{
    int32_t size = m_progress_set.size() + 1;
    int32_t* pos = static_cast<int32_t*>(std::malloc(size * sizeof(int32_t)));
    std::memset(pos, 0, size * sizeof(int32_t));

    for (auto it : m_progress_set)
        if (it->m_pos < size)
            pos[it->m_pos] = 1;

    int32_t min_pos;
    for (int32_t i = 0; i < size; ++i)
    {
        min_pos = i;
        if (pos[i] == 0) break;
    }
    std::free(pos);
    return min_pos;
}