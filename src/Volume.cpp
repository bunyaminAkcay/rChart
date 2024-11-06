#include "../include/Volume.hpp"

Volume::Volume(std::vector<Candlestick>& candlesticks, raylib::Color color) : Indicator(), candlesticks(candlesticks) {
    this->name = "Volume";
    this->color = color;
    this->indicatorType = IndicatorType::histogram;
}

Volume::~Volume(){

}

void Volume::compute() {
    for (int i = 0; i < candlesticks.size(); i++)
    {
        this->outputData.push_back(candlesticks[i].volume);
    }
}