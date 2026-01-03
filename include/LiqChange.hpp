#ifndef LIQCHANGE_H
#define LIQCHANGE_H

#include "Indicator.hpp"
#include "Candlestick.hpp"
#include "../include-raylib/raylib-cpp.hpp"

#include <algorithm>
#include <cassert>

class LiqChange : public Indicator
{   
    private:
        std::vector<Candlestick>& candlesticks;
        std::vector<double>& liqChangeValues;
        unsigned int len;
    public:

        LiqChange(int len, std::vector<Candlestick>& candlesticks, std::vector<double>& liqChangeValues, raylib::Color color);
        ~LiqChange();
        void compute() override;
};

#endif