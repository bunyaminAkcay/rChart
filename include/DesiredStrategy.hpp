#ifndef DESIRED_STRATEGY
#define DESIRED_STRATEGY


#include "Indicator.hpp"
#include "Candlestick.hpp"
#include "Order.hpp"
#include <vector>


#include <iostream>



class DesiredStrategy : public Indicator
{
    private:
        std::vector<Candlestick>& candlesticks;
    public:
        std::vector<double> atr; 
        DesiredStrategy(std::vector<Candlestick>& candlesticks, std::vector<double> atr, Color color);
        ~DesiredStrategy();
        void compute();
};

DesiredStrategy::DesiredStrategy(std::vector<Candlestick>& candlesticks, std::vector<double> atr, Color color) : Indicator(), candlesticks(candlesticks), atr(atr)
{
    this->name = "Desired Strategy";
    this->color = color;
    this->indicatorType = IndicatorType::strategy;
}

DesiredStrategy::~DesiredStrategy()
{
}

void DesiredStrategy::compute(){
    
    double takeProfitStopLossRatio = 2;
    double atrMultiplier = 2;//2.3

    for (int i = 0; i < candlesticks.size(); i++)
    {

        double buyStopLoss = candlesticks[i].closePrice - atrMultiplier * atr[i];
        double sellStopLoss = candlesticks[i].closePrice + atrMultiplier * atr[i];
        
        double buyTakeProfit = candlesticks[i].closePrice + atrMultiplier * takeProfitStopLossRatio * atr[i];
        double sellTakeProfit = candlesticks[i].closePrice - atrMultiplier * takeProfitStopLossRatio * atr[i];

        double buyPosition = true;
        double sellPosition = true;

        for (int j = i; j < candlesticks.size(); j++)
        {

            if (buyPosition)
            {
                
                if (candlesticks[j].lowPrice < buyStopLoss)
                {
                    buyPosition = false;
                }
                
                if (candlesticks[j].highPrice > buyTakeProfit)
                {
                    sellPosition = false;
                    break;
                }

            }

            if (sellPosition)
            {
                if (candlesticks[j].highPrice > sellStopLoss)
                {
                    sellPosition = false;
                }
                
                if (candlesticks[j].lowPrice < sellTakeProfit)
                {
                    buyPosition = false;
                    break;
                }
            }

            if (!buyPosition && !sellPosition) {
                break;
            }
        }

        OrderType order;

        if (buyPosition && !sellPosition)
        {
            order = Buy;
        }
        else if (sellPosition && !buyPosition)
        {
            order = Sell;
        }
        else if( buyPosition && sellPosition){
            order = NoInfo;
        }
        else
        {
            order = NoOrder;
        }

        outputData.push_back(double(order));
    }    
}

#endif