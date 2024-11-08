#include "../include/API.hpp"
#include "../external/httplib.h"
#include "../external/json.hpp"
#include <iostream>
#include <string>
#include <chrono>

using json = nlohmann::json;

API::~API()
{
}

API::API()
{
}

long API::intervalStringToTimestamp(std::string interval){
    
    const long seconds = 1000;
    const long minute = 60 * seconds;
    const long hour = 60 * minute;
    const long day = 24 * hour;
    const long week = 7 * day;

    char timeUnit = interval.back();

    int timeValue = std::stoi(interval.substr(0, interval.size() - 1));

    long timeUnitValue;

    if (timeUnit == 'm')
    {
        timeUnitValue = minute;
    }
    else if (timeUnit == 'h')
    {
        timeUnitValue = hour;
    }
    else if (timeUnit == 'd')
    {
        timeUnitValue = day;
    }
    else if (timeUnit == 'w')
    {
        timeUnitValue = week;
    }
    else{
        printf("\nUnkown timeUnit\n");
        exit(-1);
    }

    return timeUnitValue * timeValue;
}

std::vector<Candlestick> API::getCandlestickData(std::string symbol, std::string interval, int fetchCount){
    
    std::vector<Candlestick> candlesticks;

    httplib::Client cli(url);

    auto now = std::chrono::system_clock::now();

    auto duration = now.time_since_epoch();

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    long intervalValue = intervalStringToTimestamp(interval);
    
    // you can change the p value to fetch older data
    int p = 0;
    long date = milliseconds - long(fetchCount) * 1000 * intervalValue - p*( long(fetchCount) * intervalValue * 1000);

    for (int i = 0; i < fetchCount; i++)
    {
        long startTime = date;

        date += 1000 * intervalValue;
        
        auto res = cli.Get("/api/v3/klines?symbol=" + symbol + "&interval=" + interval + "&limit=1000" + "&startTime=" + std::to_string(startTime) + "&endTime=" + std::to_string(date) );
    
        json candlesticksJson = json::parse(res->body);

        auto trimDoubleQuote = [](std::string str){
            str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
            return str;
        };

        for (json::iterator it = candlesticksJson.begin(); it != candlesticksJson.end(); ++it) {
            json candlestickJson = *it;
            Candlestick candlestick;
            candlestick.klineOpenTime = std::stoll((*it)[0].dump());
            candlestick.openPrice = std::stod(trimDoubleQuote((*it)[1].dump()));
            candlestick.highPrice = std::stod(trimDoubleQuote((*it)[2].dump()));
            candlestick.lowPrice = std::stod(trimDoubleQuote((*it)[3].dump()));
            candlestick.closePrice = std::stod(trimDoubleQuote((*it)[4].dump()));
            candlestick.volume = std::stod(trimDoubleQuote((*it)[5].dump()));
            candlestick.klineCloseTime = std::stoll((*it)[6].dump());
            candlestick.quoteAssetVolume = std::stod(trimDoubleQuote((*it)[7].dump()));
            
            candlesticks.push_back(candlestick);
        }
    }
    printf("rChart INFO: %ld candles is fetched from Binance API.\n", candlesticks.size());
    return candlesticks;
}