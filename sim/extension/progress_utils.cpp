#include "progress_utils.h"

/* static member of class Bar */
std::string Bar::COLOUR_RESET = "\x1b[0m";
std::unordered_map<Bar::CHARSET, Bar::bar_charset> Bar::CHARSET_MAP = {
    {BLANK, {.set = {' ', ' '}, .len = 2}},
    {ASCII, {.set = {' ', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#'}, .len = 11}},
    {UTF,   {.set = {' ', 0x258f , 0x258e , 0x258d , 0x258c , 0x258b , 0x258a , 0x2589 , 0x2588}, .len = 9}}
};
std::unordered_map<Bar::COLOUR, std::string> Bar::COLOURS_MAP = {
    {NONE_COLOUR, std::string()},
    {BLACK,     "\x1b[30m"},
    {RED,       "\x1b[31m"},
    {GREEN,     "\x1b[32m"},
    {YELLOW,    "\x1b[33m"},
    {BLUE,      "\x1b[34m"},
    {MAGENTA,   "\x1b[35m"},
    {CYAN,      "\x1b[36m"},
    {WHITE,     "\x1b[37m"},
};


Bar::Bar(uint32_t default_len, CHARSET charset, COLOUR colour)
{
    assert (default_len > 0);
    m_frac          = 0;
    m_default_len   = default_len;
    m_type_charset  = charset;
    m_type_colour   = colour;
    m_charset       = CHARSET_MAP.find(charset)->second;
    m_colour        = COLOURS_MAP.find(colour)->second;
}

/* Set Colour with enumed colour */
void Bar::setColour (COLOUR colour)
{
    m_type_colour = colour;
    m_colour = COLOURS_MAP.find(colour)->second;
}

/* Set Colour with rgb */
void Bar::setColour (int r, int g, int b)
{
    char buff[20];
    snprintf(buff, 20, "\x1b[38;2;%d;%d;%dm", r, g, b);
    m_colour = std::string(buff);
    m_type_colour = COUSTOM;
}

/* Update Bar*/
std::string Bar::update(double frac, uint32_t default_len)
{
    m_default_len = default_len;
    return update(frac);
}


/* Update Bar */
std::string Bar::update(double frac)
{
    assert(frac >= m_frac);

    m_frac = frac;
    uint32_t N_BARS = m_default_len;
    uint32_t nsyms  = m_charset.len - 1;

    /* bar 由三部分组成 (每格代表 10% 进度, 56%: "#####6    ")
     * 计算完整格数以及当前格已经完成的部分
     */
    uint32_t bar_length      = static_cast<uint32_t>(m_frac * N_BARS * nsyms) / nsyms;
    uint32_t frac_bar_length = static_cast<uint32_t>(m_frac * N_BARS * nsyms) % nsyms;

    /* Part 1 */
    std::string res = convertChar16ToString(m_charset.set[nsyms], bar_length);
    
    /* Part 2, 3 */
    if (bar_length < N_BARS)
        res = res + convertChar16ToString(m_charset.set[frac_bar_length], 1) + convertChar16ToString(m_charset.set[0], N_BARS - bar_length - 1);
    
    /* COLOUR */
    return m_type_colour != NONE_COLOUR ? m_colour + res + COLOUR_RESET : res;  // 如果设置了颜色, 前后补上 COLOUR 控制符
}


/* Covert char16_t to std::string */
std::string Bar::convertChar16ToString(char16_t charArray, std::size_t length)
{
    // Step 1: Create a std::u16string from the char16_t array
    std::u16string u16str(length, charArray);

    // Step 2: Convert the std::u16string to a UTF-8 encoded std::string
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::string str = converter.to_bytes(u16str);

    return str;
}


/* Parameters
 * ----------
 * x  : float
 *     New value to include in EMA.
 */
EMA::EMA(double smoothing)
{
    assert(smoothing < 1);
    assert(smoothing > 0);
    m_alpha = smoothing;        // default: 0.3
    m_adjst = 1;
    m_value = 0;
}

double EMA::get()
{
    return m_value / (1 - m_adjst);
}

double EMA::update(double x)
{
    double beta = 1 - m_alpha;  // default: 0.7
    m_value = m_alpha * x + beta * m_value;
    m_adjst *= beta;

    return m_value / (1 - m_adjst);
}
