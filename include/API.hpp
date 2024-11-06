#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <vector>
#include <string>
#include "Candlestick.hpp"

/*
    Binance API
*/

class API
{   
    private:
        const std::string url = "https://api.binance.com";
        long intervalStringToTimestamp(std::string interval);
    public:
        API();
        ~API();
        std::vector<Candlestick> getCandlestickData(std::string symbol, std::string interval, int fetchCount=1);

};