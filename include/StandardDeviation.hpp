#ifndef STANDARD_DEVIATION_H
#define STANDARD_DEVIATION_H

#include "Indicator.hpp"
#include "Candlestick.hpp"

#include <cassert>

class StandardDeviation : public Indicator
{   
    private:
        std::vector<Candlestick>& candlesticks;
        int period;
    public:

        StandardDeviation(int period, std::vector<Candlestick>& candlesticks, raylib::Color color);
        ~StandardDeviation();
        void compute() override;
};

#endif