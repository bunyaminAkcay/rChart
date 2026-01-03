#include "../include/LiqChange.hpp"
#include <cstdlib>
#include <iostream>

LiqChange::LiqChange(int len, std::vector<Candlestick>& candlesticks, std::vector<double>& liqChangeValues, raylib::Color color): Indicator(), candlesticks(candlesticks), liqChangeValues(liqChangeValues){
    this->name = "Liq Change Indicator";
    this->color = color;
    this->indicatorType = IndicatorType::line;
    this->len = len;
}


LiqChange::~LiqChange(){

}


void LiqChange::compute(){
    for (int i = 0; i < candlesticks.size(); i++)
    {
        if (i <= len){
            outputData.push_back(0);
        }
        else {
                
            double cummulatedLiqChange = 0;

            for (int j = 0; j < len; j++) {
                
                double diff =  abs(liqChangeValues[i -j] - liqChangeValues[i -j - 1]); // burası problemli
                cummulatedLiqChange += diff;
            }

            outputData.push_back(cummulatedLiqChange);
        }
    }
}