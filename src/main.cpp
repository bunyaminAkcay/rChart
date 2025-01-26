#include "../include/Chart.hpp"
#include "../include/API.hpp"
#include "../include/ATR.hpp"
#include "../include/Volume.hpp"
#include "../include/LiquidityHeatmap.hpp"
#include "../include/DesiredStrategy.hpp"
#include "../include/SupportResistance.hpp"
#include <raylib.h>
#include <vector>
#include <string>

int main() {

    API api;
    
    //LTC LINK BTC ETH BNB SOL AVAX BCH ETC AAVE
    std::string parity = "BTCUSDT";
    std::string internal = "1h";
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
    DesiredStrategy desiredStrategy(candlestickData, atr.outputData, BLACK);
    desiredStrategy.compute();

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
                                        2000,
                                        ORANGE);
    liquidityHeatmap.compute();
    

    SupportResistance supportResitance(liquidityHeatmap.heatmap, liquidityHeatmap.rangeBottom, liquidityHeatmap.boxSize, RED);

    supportResitance.compute();

    raylib::Window chartWindow = raylib::Window(windowWidth, windowHeight, "", windowConfig);
    
    //Chart is a class that draw candles, cursor and indicators.
    Chart chart(&chartWindow, title, candlestickData);
    chart.liquidityHeatmap = &liquidityHeatmap;
    chart.supportResistance = &supportResitance;
    

    Indicator* atrRef = &atr;
    Indicator* volumeRef = &volume;
    Indicator* lhmRef = &liquidityHeatmap;
    Indicator* dsRef = &desiredStrategy;
    Indicator* srRef = &supportResitance;

    chart.insertIndicator(lhmRef);
    chart.insertIndicator(srRef);
    chart.insertIndicator(atrRef);
    chart.insertIndicator(volumeRef);
    //chart.insertIndicator(dsRef);

    //We can save heatmap and desiredStrategy as csv so we can apply machine learning algorithms.
    //chart.saveHeatmapToCsv(desiredStrategy.outputData, &atr, "liquidityHeatmap.csv", false, false);    
    chart.saveSupportResistanceToCsv(desiredStrategy.outputData, supportResitance.heatmap, "srHeatmap.csv");

    chart.run();
    return 0;
}