#include "../include/StandardDeviation.hpp"

StandardDeviation::StandardDeviation(int period, std::vector<Candlestick>& candlesticks, raylib::Color color): Indicator(), candlesticks(candlesticks), period(period)
{
    this->name = "Standard Deviation";
    this->color = color;
    this->indicatorType = IndicatorType::line;
}

StandardDeviation::~StandardDeviation()
{
}

void StandardDeviation::compute(){
    for (int i = 0; i < candlesticks.size(); i++)
    {

        if (i < period){
            outputData.push_back(0);
        }
        else
        {
            double sum = 0.0, sumSquared = 0.0;
            for (size_t j = i - period; j < i; j++)
            {
                double closePrice = candlesticks[j].closePrice;
                sum += closePrice;
                sumSquared += closePrice * closePrice;
            }

            double mean = sum / period;
            double variance = (sumSquared / period) - (mean * mean);
            double stdDev = std::sqrt(variance);

            outputData.push_back(stdDev);
        }
    }
}