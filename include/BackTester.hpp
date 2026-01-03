#ifndef BACKTESTER_H
#define BACKTESTER_H

#include <vector>
#include "Candlestick.hpp"

class BackTester{
    private:
        std::vector<Candlestick>& candlesticks;
    
    public:
        BackTester(std::vector<Candlestick>& candlesticks);
        double backtest(std::vector<double>& orderUpperPrices, std::vector<double>& orderLowerPrices, std::vector<double>& orderTypes);
};

#endif