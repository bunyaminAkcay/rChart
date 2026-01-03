#include "../include/Chart.hpp"
#include "../include/API.hpp"
#include "../include/ATR.hpp"
#include "../include/StandardDeviation.hpp"
#include "../include/Volume.hpp"
#include "../include/LiquidityHeatmap.hpp"
#include "../include/BackTester.hpp"
#include "../include/LiqChange.hpp"

#include "../include/SupportResistance.hpp"
#include <algorithm>
#include <cfloat>
#include <raylib.h>
#include <vector>
#include <string>

#include <iostream>

int main() {

    API api;
    //BTC HBAR XRP SUI ADA XLM ETH LTC SOL
    std::string parity = "BTCUSDT";
    std::string internal = "2h";
    std::string title = "rChart " + parity + "-" + internal;
    
    //you can change the window size and fullscreen option
    int windowWidth = 1440, windowHeight = 900;
    bool fullscreen = false;

    bool predictionEnabled = false;

    int windowConfig = fullscreen ? 0x00000002 : 0x00000000;

    std::vector<Candlestick> candlestickData = api.getCandlestickData(parity, internal, 5);

    //Volume indicator
    Volume volume(candlestickData, PURPLE);
    volume.compute();
    
    //ATR indicator is used as input for the liquidityHeatmap indicator.
    ATR atr(14, candlestickData, RED);
    atr.compute();

    //Desired strategy gives perfect signals by consedering next candles
    //It can be useful to label data and then process for machine learning algorithms
    //some examples in classification folder try to classify by using labeled data and liquidity heatmap output 
    
    //DesiredStrategy desiredStrategy(candlestickData, atr.outputData, BLACK);
    //desiredStrategy.compute();

    //Liquidty heatmap is an indicator that try to simulate liquidity with market candlesticks and volume.
    //Market is chaotic and it depends so many parameter that we cant even count.
    //Liquidity heatmap assumes accumulated total liquidity can simulate as a normal distribution for candle n.
    //A volatility indicator (atr is used) determines accumulated liquidty's variance for candle n.
    //Total limit order volume determines accumulated liquidty's amplitude for candle n.
    //At candle n, some liquidity in the market is decreased due to market orders
    //And if liquidity is not liquidated yet remaning liquidity is moved to next candle.
    LiquidityHeatmap liquidityHeatmap(  candlestickData,
                                        volume.outputData,
                                        atr.outputData,
                                        5000,
                                        ORANGE,
                                        false);
    liquidityHeatmap.compute();
    

    SupportResistance supportResitance(liquidityHeatmap.heatmap, liquidityHeatmap.rangeBottom, liquidityHeatmap.boxSize, RED);

    supportResitance.compute();

    raylib::Window chartWindow = raylib::Window(windowWidth, windowHeight, "", windowConfig);
    
    //Chart is a class that draw candles, cursor and indicators.
    Chart chart(&chartWindow, title, candlestickData);
    chart.liquidityHeatmap = &liquidityHeatmap;
    chart.supportResistance = &supportResitance;

    LiqChange liqChange(5, candlestickData, liquidityHeatmap.liqChangeValues, GRAY);
    liqChange.compute();

    Indicator* atrRef = &atr;
    Indicator* volumeRef = &volume;
    Indicator* lhmRef = &liquidityHeatmap;
    //Indicator* dsRef = &desiredStrategy;
    Indicator* srRef = &supportResitance;
    
    Indicator* liqChangeRef = &liqChange;

    chart.insertIndicator(lhmRef);
    chart.insertIndicator(srRef);
    //chart.insertIndicator(atrRef);
    //chart.insertIndicator(volumeRef);
    //chart.insertIndicator(dsRef);
    chart.insertIndicator(liqChangeRef);

    //We can save heatmap and desiredStrategy as csv so we can apply machine learning algorithms.
    //chart.saveHeatmapToCsv(desiredStrategy.outputData, &atr, "liquidityHeatmap.csv");    
    //chart.saveSupportResistanceToCsv(supportResitance.heatmap, "srHeatmap.csv");
    
    bool gridSearch = false;
    if (gridSearch) {
            
        std::cout << "Grid search:" << std::endl;
        
        double minError = DBL_MAX;
        double minLR, minVM, minLD, minQ, minDf;
        
        double lossRatio = 0.9975;
        double ld = 1;
        double vm = 0.069;              
        double q = 1;     
        double df = 6;      
        
        //for (double vm = 0.065 ; vm <=0.075; vm+=0.001){
            //for (double df = 8; df < 12; df*=1.1) {
                for (double lossRatio = 1 ; lossRatio >= 0.995; lossRatio*=0.9999){
                    
                    double totalError = 0;
                    
                    liquidityHeatmap.lossRatio = lossRatio;
                    liquidityHeatmap.volatilityMultiplier = vm;
                    liquidityHeatmap.liquidityDensityCoefficient = ld;
                    liquidityHeatmap.q = q;
                    liquidityHeatmap.df = df;
                    liquidityHeatmap.recompute();

                    for (int i=1000; i< liquidityHeatmap.heatmap.size(); i++){ // do not calculate for first 1000 candle 
                        double error = abs(liquidityHeatmap.liquidatedVolume[i] - volume.nonTakerVolume[i]);
                        totalError += error;
                    }

                    if (totalError < minError) {    
                        minLR = lossRatio;
                        minVM = vm;
                        minLD = ld;
                        minQ = q;
                        minDf = df;
                        minError = totalError;
                        std::cout << "new min value!" << std::endl;
                    }

                    std::cout << "lossRatio " << lossRatio << ", volatility " << vm << ", liqDensity " << ld << ", q " << q << ", df " << df <<  " : " << totalError << std::endl;
                }
            //}
            
        //}

        liquidityHeatmap.lossRatio = minLR;
        liquidityHeatmap.volatilityMultiplier = minVM;
        liquidityHeatmap.liquidityDensityCoefficient = minLD;
        liquidityHeatmap.q = minQ;
        liquidityHeatmap.df = minDf;
        liquidityHeatmap.recompute();
    }

    double totalError = 0;
    double totalVolume = 0;
    for (int i=1000; i< liquidityHeatmap.heatmap.size(); i++){ // do not calculate for first 1000 candle 
        double error = abs(liquidityHeatmap.liquidatedVolume[i] - volume.nonTakerVolume[i]);
        totalError += error;
        totalVolume +=  volume.outputData[i];
    }

    std::cout << "lossRatio " << liquidityHeatmap.lossRatio <<
                ", volatility " << liquidityHeatmap.volatilityMultiplier <<
                ", liqDensity " << liquidityHeatmap.liquidityDensityCoefficient <<
                ", q " << liquidityHeatmap.q <<
                ", df " << liquidityHeatmap.df <<
                " : " << totalError <<
                " % " << (100*totalError/totalVolume) << std::endl;
    
    BackTester backTester(candlestickData);

    bool backtestGridSearch = false;

    if (backtestGridSearch) {
        double trustAtrValues[] = {0.5};
        double tpTrustCoefValues[] = {0.9};
        double reduceCoefValues[] = { 0.95 };
        double lowScoreLimitValues[] = { 1.5 };
        double minRewadValues[] = {0.1}; // no importance
        double minFinalReward[] = {1};
        double minAtrLimitValues[] = {2.6};
        double maxAtrLimitValues[] = {7.6};
        
        int combinationCount = 0;

        double maxProfit =  - DBL_MAX;

        // Grid search with nested for loops to test all parameter combinations
        for (double trustAtr : trustAtrValues) {
            for (double tpTrustCoef : tpTrustCoefValues) {
                for (double reduceCoef : reduceCoefValues) {
                    for (double lowScoreLimit : lowScoreLimitValues) {
                        for (double minReward : minRewadValues) {
                            for (double minFinalRewardVal : minFinalReward) {
                                for (double minAtrLimit : minAtrLimitValues) {
                                    for (double maxAtrLimit : maxAtrLimitValues) {
                                        combinationCount++;
                                        // Apply parameters
                                        liquidityHeatmap.trustAtr = trustAtr;
                                        liquidityHeatmap.tpTrustCoef = tpTrustCoef;
                                        liquidityHeatmap.reduceCoef = reduceCoef;
                                        liquidityHeatmap.lowScoreLimit = lowScoreLimit;
                                        liquidityHeatmap.minReward = minReward;
                                        liquidityHeatmap.minFinalReward = minFinalRewardVal;
                                        liquidityHeatmap.minAtrLimit = minAtrLimit;
                                        liquidityHeatmap.maxAtrLimit = maxAtrLimit;
                                        
                                        std::cout << "===== Testing Combination #" << combinationCount << " =====" << std::endl;
                                        std::cout << "trustAtr: " << trustAtr << std::endl;
                                        std::cout << "tpTrustCoef: " << tpTrustCoef << std::endl;
                                        std::cout << "reduceCoef: " << reduceCoef << std::endl;
                                        std::cout << "lowScoreLimit: " << lowScoreLimit << std::endl;
                                        std::cout << "minReward: " << minReward << std::endl;
                                        std::cout << "minFinalReward: " << minFinalRewardVal << std::endl;
                                        std::cout << "minAtrLimit: " << minAtrLimit << std::endl;
                                        std::cout << "maxAtrLimit: " << maxAtrLimit << std::endl;

                                        // Recompute and run backtest
                                        liquidityHeatmap.recompute();
                                        double profit = backTester.backtest(
                                            liquidityHeatmap.orderUpperPrices, 
                                            liquidityHeatmap.orderLowerPrices, 
                                            liquidityHeatmap.orderTypes
                                        );

                                        if (profit > maxProfit) {
                                            maxProfit = profit;
                                            std::cout << "BETTER PARAMETERS!!!" << std::endl;
                                        }
                                        
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    chart.run();

    return 0;
}