#include "../include/LiquidityHeatmap.hpp"
#include <climits>
#include <cmath>
#include <math.h>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <numeric>
#include <string>
#include <unordered_map>


LiquidityHeatmap::LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color, bool withStrategy)
    : Indicator(),
    candlesticks(candlesticks),
    volume(volume),
    volatilityValues(volatilityValues),
    resolution(resolution),
    rangeBottom(candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)),
    rangeTop(candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)),
    boxSize(((candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)) - (candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)))/double(resolution)),
    withStrategy(withStrategy)
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
    
    orderLowerPrices.clear();
    orderLowerPrices.shrink_to_fit();
    orderUpperPrices.clear();
    orderUpperPrices.shrink_to_fit();
    orderTypes.clear();
    orderTypes.shrink_to_fit();

    maxLiquidity = 0;
    compute();
}

void LiquidityHeatmap::compute(){

    double prevLowPrice = 0, prevHighPrice = 0;

    for (int index = 0; index < candlesticks.size(); index++)
    {
        std::vector<double> heatmapColumn;
        liquidatedVolume.push_back(0);
        

        Candlestick c = candlesticks[index];

        double meanPrice = (c.highPrice + c.lowPrice) * 0.5;

        std::vector<double> lastLiq;

        if (index == 0){
            lastLiq = std::vector<double>(resolution, 0);
        }
        else
        {
            lastLiq = heatmap[index-1];
        }
        
        liqChangeValue = 0;
        
        for (int i = 0; i < resolution; i++)
        {
            double price = rangeBottom + (i + 1) * boxSize;
            double liq;

            double sd = volatilityMultiplier * volatilityValues[index];
            double multiplierDueToDistance = (candlesticks[candlesticks.size() -1].closePrice);
            double liquidityDensity = liquidityDensityCoefficient * rangeRatio * multiplierDueToDistance / double(resolution);
            //std::cout << liquidityDensityCoefficient << " " << liquidityDensity << " " << c.takerBuyBaseVolume << std::endl;
            liq = liquidityDensity * student_t_pdf(price, meanPrice, sd, df) * c.nonTakerVolume;
            
            if (price > c.lowPrice && price < c.highPrice ){
                
                liquidatedVolume[index] += lastLiq[i] + q * liq;  //if new liq is inside of new candle, some of it liquidated and other part accumulated
                

                //about liq change
                if (price > prevHighPrice) {
                    liqChangeValue += lastLiq[i];
                }
                else if (price < prevLowPrice) {
                    liqChangeValue -= lastLiq[i];
                }
                // end liq change

                liq *= (1-q);
                
            }
            else {
                liq += lossRatio * lastLiq[i];
            }

            if (liq > maxLiquidity)
            {                                       
                maxLiquidity = liq;
            }
            
            heatmapColumn.push_back(liq);
        }
        
        liqChangeValues.push_back(liqChangeValue);
        prevHighPrice = c.highPrice;
        prevLowPrice = c.lowPrice;

        double marketVol = volume[index] - liquidatedVolume[index];
        marketVolume.push_back(marketVol);

        totalMarketVol += marketVol > 0 ? marketVol : 0;
        totalLiqVol += liquidatedVolume[index] > 0 ? liquidatedVolume[index] : 0;

        heatmap.push_back(heatmapColumn);
        
        //for stategy
        Order strategyOutput = calculateStrategy(heatmapColumn, index);

        orderTypes.push_back(strategyOutput.orderType);

        if (strategyOutput.orderType == OrderType::Buy || strategyOutput.orderType == OrderType::LowScoreLong || strategyOutput.orderType == OrderType::LowTpSlRatioLong) {
            liqChange.push_back(strategyOutput.trustScore);
            orderUpperPrices.push_back(strategyOutput.tpPrice);
            orderLowerPrices.push_back(strategyOutput.stopLossPrice);
        }
        else if (strategyOutput.orderType == OrderType::Sell || strategyOutput.orderType == OrderType::LowScoreShort || strategyOutput.orderType == OrderType::LowTpSlRatioShort) {
            liqChange.push_back(strategyOutput.trustScore);
            orderUpperPrices.push_back(strategyOutput.stopLossPrice);
            orderLowerPrices.push_back(strategyOutput.tpPrice);
        }
        else {// for no_info
            liqChange.push_back(0);
            orderUpperPrices.push_back(0);
            orderLowerPrices.push_back(0);
        }
    }

    //calculate avg liq for ploting
    double totalLiqInHeatmap = 0;
    for (int index = 0; index < candlesticks.size(); index++){
        std::vector<double> heatmapColumn = heatmap[index];
        for (int i = 0; i < resolution; i++){
            totalLiqInHeatmap += heatmapColumn[i];
        }
    }
    oneOverAvgLiqBox = (candlesticks.size() * resolution)/totalLiqInHeatmap;
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


double LiquidityHeatmap::cauchyPdf(double x, double x0, double gamma){
    
    if (gamma == 0)
    {
        return 0;
    }

    double denom = PI * gamma * (1 + pow((x - x0) / gamma, 2));
    return 1.0 / denom;
}

double LiquidityHeatmap::student_t_pdf(double t, double mu, double sigma, double df) {
    if (df <= 0) {
        throw std::invalid_argument("Serbestlik derecesi (df) pozitif olmalıdır.");
    }
    if(sigma == 0){
        return 0;
    }
    else if (sigma < 0) {
        throw std::invalid_argument("Ölçek parametresi (sigma) pozitif olmalıdır.");
    }
    double numerator = std::tgamma((df + 1.0) / 2.0);
    double denominator = std::sqrt(df * M_PI) * std::tgamma(df / 2.0) * sigma;
    double factor = std::pow(1.0 + (std::pow(t - mu, 2) / (df * sigma * sigma)), -((df + 1.0) / 2.0));
    return (numerator / denominator) * factor;
}

Order LiquidityHeatmap::calculateStrategy(std::vector<double>& heatmapColumn, int candleIndex){

    Candlestick c = candlesticks[candleIndex];
    double volatilityValue = volatilityValues[candleIndex];
    double hl2 = (c.highPrice + c.lowPrice) * 0.5;

    double upperMaxLimit = hl2 + volatilityValue * maxAtrLimit;
    double upperLowLimit = hl2 + volatilityValue * minAtrLimit;
    
    double lowerMaxLimit = hl2 - volatilityValue * maxAtrLimit;
    double lowerLowLimit = hl2 - volatilityValue * minAtrLimit;

    int minUpperIndex = INT_MAX;
    int maxLowerIndex = -INT_MAX;
    
    std::unordered_map<int, double> upperPriceLiqPairs; // i, accumulatedLiq
    std::unordered_map<int, double> lowerPriceLiqPairs; // i, accumulatedLiq
    
    double upperAccumulatedLiq = 0, lowerAccumulatedLiq = 0;

    double totalUpperAccumulatedLiq = 0, totalLowerAccumulatedLiq = 0;
    int totalUpperIndexCount = 0, totalLowerIndexCount = 0;

    //calculate average liq
    for (int i = 0; i < resolution; i++){
        double price = rangeBottom + (i + 1) * boxSize;
        if (price > lowerMaxLimit && price < upperMaxLimit){

            if (price > hl2) {
                if (price > upperLowLimit) {
                    totalUpperAccumulatedLiq += heatmapColumn[i];
                    totalUpperIndexCount++;
                }
            }
            else {
                if (price < lowerLowLimit) {
                    totalLowerAccumulatedLiq += heatmapColumn[i];
                    totalLowerIndexCount++;
                }
            }

        }
    }
    
    double upperAverageLiq = totalUpperAccumulatedLiq / totalUpperIndexCount;
    double lowerAverageLiq = totalLowerAccumulatedLiq / totalLowerIndexCount; 


    for (int i = 0; i < resolution; i++){

        double price = rangeBottom + (i + 1) * boxSize;
        if (price > lowerMaxLimit && price < upperMaxLimit){

            if (price > hl2) {
                upperAccumulatedLiq += heatmapColumn[i];
                if (price > upperLowLimit && heatmapColumn[i] > upperAverageLiq) {
                    upperPriceLiqPairs[i] = upperAccumulatedLiq;
                    if (i < minUpperIndex) {
                        minUpperIndex = i;
                    }
                }
            }
            else {
                lowerAccumulatedLiq += heatmapColumn[i];
                if (price < lowerLowLimit && heatmapColumn[i] > lowerAverageLiq) {
                    lowerPriceLiqPairs[i] = lowerAccumulatedLiq;
                    maxLowerIndex = i;
                }
            }

        }
    }

    //lowerPriceLiqPairs is accumulated from bottom to top. fix it with that for loop
    for (auto& pair : lowerPriceLiqPairs) {
        pair.second = lowerAccumulatedLiq - pair.second;
    }

    double bestLongScore = 0, bestShortScore = 0;

    Order shortOrder = Order();
    Order longOrder = Order();

    double longOrderSlAccumulatedLiq, shortOrderSlAccumulatedLiq;

    // complexity of this function O(n2).
    // Remember this function calls for every candle for drawing or backtesting
    for (auto& lowerPair : lowerPriceLiqPairs) {
        for (auto& upperPair : upperPriceLiqPairs) {
            
            double upperPairPrice = rangeBottom + (upperPair.first + 1) * boxSize;
            double lowerPairPrice = rangeBottom + (lowerPair.first + 1) * boxSize;

            double upperTargetDist = upperPairPrice - c.closePrice;
            double lowerTargetDist = c.closePrice - lowerPairPrice;

            double longScore = (upperPair.second/lowerPair.second) / (upperTargetDist / lowerTargetDist);
            double shortScore = 1 / longScore;

            if (longScore > bestLongScore && upperTargetDist/lowerTargetDist > minReward) {
                bestLongScore = longScore;
                longOrder = Order(OrderType::Buy, c.closePrice, lowerPairPrice, upperPairPrice);
                longOrderSlAccumulatedLiq = lowerPair.second;
            }

            if (shortScore > bestShortScore && lowerTargetDist/upperTargetDist > minReward) {
                bestShortScore = shortScore;
                shortOrder = Order(OrderType::Sell, c.closePrice, upperPairPrice, lowerPairPrice);
                shortOrderSlAccumulatedLiq = upperPair.second;
            }

        }
    }
    
    //check best score and determine order
    longOrder.trustScore = bestLongScore;
    shortOrder.trustScore = bestShortScore;

    //first pull down stop order by 95% of accumulated liq
    //then add 0.5 atr to sl
    
    double upperReducedAccumulatedLiq = 0, lowerReducedAccumulatedLiq = 0;
    
    double targetLongOrderSlAccumulatedLiq = longOrderSlAccumulatedLiq * reduceCoef;
    double targetShortOrderSlAccumulatedLiq = shortOrderSlAccumulatedLiq * reduceCoef;

    double targetLongSlCalculated = false;
    double targetShortSlCalculated = false;

    
    for (int i = 0; i < resolution; i++){

        double price = rangeBottom + (i + 1) * boxSize;
        if (price > lowerMaxLimit && price < upperMaxLimit){

            if (price > hl2 && !(targetShortSlCalculated)) {
                upperReducedAccumulatedLiq += heatmapColumn[i];
                if (price > upperLowLimit && upperReducedAccumulatedLiq > targetShortOrderSlAccumulatedLiq){
                    shortOrder.stopLossPrice = price + trustAtr * volatilityValue;
                    targetShortSlCalculated = true;
                }
            }
            else if (!targetLongSlCalculated) {
                lowerReducedAccumulatedLiq += heatmapColumn[i];
                if (price < lowerLowLimit && totalLowerAccumulatedLiq - lowerReducedAccumulatedLiq < targetLongOrderSlAccumulatedLiq){
                    longOrder.stopLossPrice = price - trustAtr * volatilityValue;
                    targetLongSlCalculated = true;
                }
            }

        }
    }
    
    //trustCoef
    longOrder.tpPrice = (longOrder.tpPrice - longOrder.entryPrice) * tpTrustCoef + longOrder.entryPrice; 
    shortOrder.tpPrice = shortOrder.entryPrice - (shortOrder.entryPrice - shortOrder.tpPrice) * tpTrustCoef;


    
    //filter low scores
    if (longOrder.trustScore < lowScoreLimit) {
        longOrder.orderType = OrderType::LowScoreLong;
    }

    if (shortOrder.trustScore < lowScoreLimit) {
        shortOrder.orderType = OrderType::LowScoreShort;
    }
    
    Order returnOrder = bestLongScore > bestShortScore ? longOrder : shortOrder;

    
    // filter low tp sl ratios
    if ((abs(returnOrder.tpPrice - returnOrder.entryPrice) / abs(returnOrder.stopLossPrice - returnOrder.entryPrice)) < minFinalReward){
        if (bestLongScore > bestShortScore) {
            returnOrder.orderType = OrderType::LowTpSlRatioLong;
        }
        else {
            returnOrder.orderType = OrderType::LowTpSlRatioShort;
        }
        
    }
    
    return returnOrder;
}

Order LiquidityHeatmap::calculateStrategy2(std::vector<double>& heatmapColumn, int candleIndex){

    int pairCount = 5;


    Candlestick c = candlesticks[candleIndex];
    double volatilityValue = volatilityValues[candleIndex];
    double hl2 = (c.highPrice + c.lowPrice) * 0.5;

    double upperMaxLimit = hl2 + volatilityValue * maxAtrLimit;
    double upperLowLimit = hl2 + volatilityValue * minAtrLimit;
    
    double lowerMaxLimit = hl2 - volatilityValue * maxAtrLimit;
    double lowerLowLimit = hl2 - volatilityValue * minAtrLimit;

    double lowerTotalLiq = 0;
    double upperTotalLiq = 0;
    
    //calculate total liq
    for (int i = 0; i < resolution; i++){
        double price = rangeBottom + (i + 1) * boxSize;
        if (price > lowerMaxLimit && price < upperMaxLimit){

            if (price > hl2) {
                upperTotalLiq += heatmapColumn[i];
            }
            else {
                lowerTotalLiq += heatmapColumn[i];
            }

        }
    }

    double upperTargetDist = upperTotalLiq/pairCount;
    double lowerTargetDist = lowerTotalLiq/pairCount;

    double upperTargetLiq = upperTargetDist; 
    double lowerTargetLiq = lowerTotalLiq - lowerTargetDist;
    
    double upperAccumulatedLiq = 0;
    double lowerAccumulatedLiq = 0;

    std::unordered_map<int, double> upperPriceLiqPairs; // i, accumulatedLiq
    std::unordered_map<int, double> lowerPriceLiqPairs; // i, accumulatedLiq

    for (int i = 0; i < resolution; i++){

        double price = rangeBottom + (i + 1) * boxSize;
        if (price > lowerMaxLimit && price < upperMaxLimit){

            if (price > hl2) {
                upperAccumulatedLiq += heatmapColumn[i];
                if (upperAccumulatedLiq > upperTargetLiq) {
                    //new target
                    upperTargetLiq += upperTargetDist;
                    if (price > upperLowLimit) {
                        upperPriceLiqPairs[i] = upperAccumulatedLiq;
                    }
                }
            }
            else {
                lowerAccumulatedLiq += heatmapColumn[i];
                if (lowerTotalLiq - lowerAccumulatedLiq < lowerTargetLiq) {
                    //new target
                    lowerTargetLiq -= lowerTargetDist;
                    if (price < lowerLowLimit ) {
                        lowerPriceLiqPairs[i] = lowerTotalLiq - lowerAccumulatedLiq;
                    }
                }
            }

        }
    }


    double bestLongScore = 0, bestShortScore = 0;

    Order shortOrder = Order();
    Order longOrder = Order();

    // complexity of this function O(n2).
    // Remember this function calls for every candle for drawing or backtesting
    for (auto& lowerPair : lowerPriceLiqPairs) {
        for (auto& upperPair : upperPriceLiqPairs) {
            
            double upperPairPrice = rangeBottom + (upperPair.first + 1) * boxSize;
            double lowerPairPrice = rangeBottom + (lowerPair.first + 1) * boxSize;

            double upperTargetDist = upperPairPrice - c.closePrice;
            double lowerTargetDist = c.closePrice - lowerPairPrice;

            double longScore = (upperPair.second/lowerPair.second) / (upperTargetDist / lowerTargetDist);
            double shortScore = 1 / longScore;

            if (longScore > bestLongScore) {
                bestLongScore = longScore;
                longOrder = Order(OrderType::Buy, c.closePrice, lowerPairPrice, upperPairPrice);
                longOrder.trustScore = longScore;
            }

            if (shortScore > bestShortScore) {
                bestShortScore = shortScore;
                shortOrder = Order(OrderType::Sell, c.closePrice, upperPairPrice, lowerPairPrice);
                shortOrder.trustScore = shortScore;
            }

        }
    }



    //filter low scores
    if (longOrder.trustScore < lowScoreLimit) {
        longOrder.orderType = OrderType::LowScoreLong;
    }

    if (shortOrder.trustScore < lowScoreLimit) {
        shortOrder.orderType = OrderType::LowScoreShort;
    }
    
    Order returnOrder = bestLongScore > bestShortScore ? longOrder : shortOrder;

    
    // filter low tp sl ratios
    if ((abs(returnOrder.tpPrice - returnOrder.entryPrice) / abs(returnOrder.stopLossPrice - returnOrder.entryPrice)) < minFinalReward){
        if (bestLongScore > bestShortScore) {
            returnOrder.orderType = OrderType::LowTpSlRatioLong;
        }
        else {
            returnOrder.orderType = OrderType::LowTpSlRatioShort;
        }
        
    }

    return returnOrder;
}