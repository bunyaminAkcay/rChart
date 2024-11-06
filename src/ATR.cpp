#include "../include/ATR.hpp"

ATR::ATR(int period, std::vector<Candlestick>& candlesticks, raylib::Color color): Indicator(), candlesticks(candlesticks), period(period)
{
    this->name = "Average True Range";
    this->color = color;
    this->indicatorType = IndicatorType::line;
}

ATR::~ATR()
{
}

void ATR::compute(){
    for (int i = 0; i < candlesticks.size(); i++)
    {
        double sum = 0;
        int last = i - period;
            
        if (last < 0){
            last = 0;
        }
        
        for (int j = last ; j < i ; j++){
            //dikkat j = 0 olduğu durum seg fault verir.
            if (j != 0)
            {
                sum += trueRange(candlesticks[j], candlesticks[j-1].closePrice);
            }
            //sum += trueRange(candlesticks[j], candlesticks[j-1].closePrice);
        }

        if (i == 0){
            outputData.push_back(0);
        }
        else
        {
            outputData.push_back(sum/(i-last));
        }
    }
}

double ATR::trueRange(Candlestick& candlestick, double lastClosePrice){
    double hl = candlestick.highPrice - candlestick.lowPrice;
    double hCp = abs(candlestick.highPrice - lastClosePrice);
    double lCp = abs(candlestick.lowPrice - lastClosePrice);
    double TR = std::max(std::max(hl, hCp), lCp);
    return TR;
}