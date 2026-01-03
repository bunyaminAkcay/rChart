#ifndef CANDLESTICK_STRUCT
#define CANDLESTICK_STRUCT

struct Candlestick
{
    long long klineOpenTime;
    double openPrice;
    double highPrice;
    double lowPrice;
    double closePrice;
    double volume;
    long long klineCloseTime;
    double quoteAssetVolume;
    long long numberOfTrades;
    double takerBuyBaseVolume;
    double nonTakerVolume;
};

#endif