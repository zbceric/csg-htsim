#ifndef PROGRESS_UTILS_H
#define PROGRESS_UTILS_H

#include <iostream>
#include <unordered_map>
#include <cassert>

#include <locale>
#include <codecvt>

class Bar {
public:
    enum COLOUR {
        NONE_COLOUR,
        BLACK, RED, GREEN, YELLOW,
        BLUE, MAGENTA, CYAN, WHITE,
        COUSTOM
    };

    enum CHARSET {
        BLANK, ASCII, UTF
    };

    struct bar_charset {
        char16_t    set[12];
        uint16_t    len;
    };

    static std::string COLOUR_RESET;
    static std::unordered_map<CHARSET, bar_charset> CHARSET_MAP;
    static std::unordered_map<COLOUR, std::string> COLOURS_MAP;

private:
    COLOUR      m_type_colour;
    CHARSET     m_type_charset;
    bar_charset m_charset;
    std::string m_colour;
    double      m_frac;
    uint32_t    m_default_len;

public:
    Bar(uint32_t default_len=10, CHARSET charset=UTF, COLOUR colour=NONE_COLOUR);

    /* Set Colour with enumed colour */
    void setColour (COLOUR colour);
    /* Set Colour with rgb */
    void setColour (int r, int g, int b);
    /* Update Bar */
    std::string update(double frac);
    std::string update(double frac, uint32_t default_len);

    /* Covert char16_t to std::string */
    static std::string convertChar16ToString(char16_t charArray, std::size_t length);
};


/* Exponential moving average: smoothing to give progressively lower
 * weights to older values.
 * 
 * Parameters
 * ----------
 * smoothing  : float, optional
 *     Smoothing factor in range [0, 1], [default: 0.3].
 *     Increase to give more weight to recent values.
 *     Ranges from 0 (yields old value) to 1 (yields new value).
 */

class EMA {
public:
    EMA (double smoothing=0.3);

    double get();
    double update(double x);

private:
    double      m_alpha;
    double      m_adjst;
    double      m_value;
};

#endif
