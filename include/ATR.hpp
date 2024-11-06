#ifndef ATR_H
#define ATR_H

#include "Indicator.hpp"
#include "Candlestick.hpp"
#include "../include-raylib/raylib-cpp.hpp"

#include <algorithm>
#include <cassert>

class ATR : public Indicator
{   
    private:
        double trueRange(Candlestick& Candlestick, double lastClosePrice);
        std::vector<Candlestick>& candlesticks;
        int period;
    public:

        ATR(int period, std::vector<Candlestick>& candlesticks, raylib::Color color);
        ~ATR();
        void compute() override;
};

#endif