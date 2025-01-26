#include "../include/LiquidityHeatmap.hpp"
#include <math.h>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <numeric>

LiquidityHeatmap::LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color)
    : Indicator(),
    candlesticks(candlesticks),
    volume(volume),
    volatilityValues(volatilityValues),
    resolution(resolution),
    rangeBottom(candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)),
    rangeTop(candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)),
    boxSize(((candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)) - (candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)))/double(resolution))
{
    this->name = "Liquidity Heatmap";
    this->color = color;
    this->indicatorType = IndicatorType::liquidityheatmap;

}

std::vector<float> LiquidityHeatmap::softmax(const std::vector<float>& logits) {
    std::vector<float> exp_logits(logits.size());
    float max_logit = *std::max_element(logits.begin(), logits.end());

    // Compute e^(logit - max_logit)
    std::transform(logits.begin(), logits.end(), exp_logits.begin(),
                   [max_logit](float logit) { return std::exp(logit - max_logit); });

    // Compute sum of exponentials
    float sum_exp_logits = std::accumulate(exp_logits.begin(), exp_logits.end(), 0.0f);

    // Compute softmax probabilities
    std::vector<float> probabilities(logits.size());
    std::transform(exp_logits.begin(), exp_logits.end(), probabilities.begin(),
                   [sum_exp_logits](float exp_logit) { return exp_logit / sum_exp_logits; });

    return probabilities;
}

LiquidityHeatmap::~LiquidityHeatmap(){
}

void LiquidityHeatmap::recompute(){
    heatmap.clear();
    heatmap.shrink_to_fit();
    liquidatedVolume.clear();
    liquidatedVolume.shrink_to_fit();
    marketVolume.clear();
    marketVolume.shrink_to_fit();
    maxLiquidity = 0;
    compute();
}

void LiquidityHeatmap::compute(){

    int compressDataIndexes[] = {0, 7, 21, 42, 77, 133, 224, 371}; 
    int compressDataMultipliers[] = {1, 2, 3, 5, 8, 13, 21};
    const int compressDataSize = 98;

    
    double totalVolume = 0;

    for (int k = 0; k < volume.size(); k++)
    {
        totalVolume += volume[k];
    }
    
    double averageVolume = totalVolume / volume.size();
    
    totalLiqVol = 50 * averageVolume;
    totalMarketVol = 50 * averageVolume;

    for (int index = 0; index < candlesticks.size(); index++)
    {
        std::vector<double> heatmapColumn;
        liquidatedVolume.push_back(0);
        

        Candlestick c = candlesticks[index];

        double meanPrice = (c.closePrice + c.openPrice) * 0.5;

        std::vector<double> lastLiq;

        if (index == 0){
            lastLiq = std::vector<double>(resolution, 0);
        }
        else
        {
            lastLiq = heatmap[index-1];
        }
        
        for (int i = 0; i < resolution; i++)
        {
            double price = rangeBottom + (i + 1) * boxSize;
            double liq;

            if (price > c.lowPrice && price < c.highPrice ){

                liquidatedVolume[index] += lastLiq[i];
                liq = 0;
            }
            else{
                double sd = volatilityMultiplier * volatilityValues[index];
                double lqVol = i == 0 ? 0 : liquidatedVolume[index-1];
                double vol = i == 0 ? 0 : volume[index-1];
                double lastMarketVol = abs(vol - lqVol);
                double liquidityDensity = (price * liquidityDensityCoefficient) * (totalMarketVol/totalLiqVol); //dont ask me why
                liq = lossRatio * lastLiq[i] + liquidityDensity * normalPdf(price, meanPrice, sd) * lastMarketVol;
            }   

            if (liq > maxLiquidity)
            {
                maxLiquidity = liq;
            }
            
            heatmapColumn.push_back(liq);
        }

        double marketVol = volume[index] - liquidatedVolume[index];
        marketVolume.push_back(marketVol);

        totalMarketVol += marketVol > 0 ? marketVol : 0;
        totalLiqVol += liquidatedVolume[index] > 0 ? liquidatedVolume[index] : 0;

        heatmap.push_back(heatmapColumn);
        
    }
}

double LiquidityHeatmap::normalPdf(double x, double mean, double sd){
    
    if (sd == 0)
    {
        return 0;
    }

    double var = sd * sd;
    double denom = sqrt(2 * PI * var);
    double num = exp(-pow(x - mean, 2) / (2 * var));
    return num/denom;
}