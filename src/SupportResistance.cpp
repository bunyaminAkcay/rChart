#include "../include/SupportResistance.hpp"
#include <cstdio>
#include <vector>
#include <iostream>

SupportResistance::SupportResistance(std::vector<std::vector<double>>& liquidityHeatmapRef, double rangeBottom, double boxSize, Color color) : Indicator(), liquidityHeatmapRef(liquidityHeatmapRef){
    this->name = "Support Resistance Indicator";
    this->color = color;
    this->indicatorType = IndicatorType::supportresistance;
    this->maxLiquidity = 0;
    this->rangeBottom = rangeBottom;
    this->boxSize = boxSize;
}

SupportResistance::~SupportResistance() {}

void SupportResistance::compute(){
    for (int i = 0; i < liquidityHeatmapRef.size(); ++i) {
        std::vector<double> column;
        for (int j = 0; j < liquidityHeatmapRef[i].size() -1; ++j) {
            double diff = abs(liquidityHeatmapRef[i][j]- liquidityHeatmapRef[i][j+1]);
            if (diff > this->maxLiquidity){
                this->maxLiquidity = diff;
            }
            column.push_back(diff);
        }
        column.push_back(0);
        this->heatmap.push_back(column);
    }
}

void SupportResistance::recompute(){
    heatmap.clear();
    heatmap.shrink_to_fit();
    compute();
}