#include "../include/BackTester.hpp"
#include "../include/Order.hpp"

#include <iostream>

BackTester::BackTester(std::vector<Candlestick>& candlesticks) : candlesticks(candlesticks){}

double BackTester::backtest(std::vector<double>& orderUpperPrices, std::vector<double>& orderLowerPrices, std::vector<double>& orderTypes){
    
    double profit = 0;
    int orderCount = 0;

    //start from 2000 index
    for (int i = 2000; i < candlesticks.size(); i++ ){

        if (!(orderTypes[i] == OrderType::Buy || orderTypes[i] == OrderType::Sell )) {
            continue;
        }

        double entryPrice = candlesticks[i].closePrice;

        for (int j = i; j < candlesticks.size(); j++){
            if (orderTypes[i] == OrderType::Buy) {
                
                // check sl 
                if (candlesticks[j].lowPrice < orderLowerPrices[i]) {
                    profit -= 1;
                    orderCount++;
                    break;;
                }
                
                //check tp
                if (candlesticks[j].highPrice > orderUpperPrices[i]) {
                    profit += (orderUpperPrices[i] - entryPrice) / (entryPrice - orderLowerPrices[i]);
                    orderCount++;
                    break;
                }

            }
            else if (orderTypes[i] == OrderType::Sell) {
            
                // check sl 
                if (candlesticks[j].highPrice > orderUpperPrices[i]) {
                    profit -= 1;
                    orderCount++;
                    break;
                }
                
                //check tp
                if (candlesticks[j].lowPrice < orderLowerPrices[i]) {
                    profit += (entryPrice - orderLowerPrices[i]) / (orderUpperPrices[i] - entryPrice);
                    orderCount++;
                    break;
                }

            }
        }
    }

    if (orderCount == 0) {
        return 0;
    }

    double avgProfit = profit/orderCount;

    std::cout << "Avg profit: " << avgProfit << ", Order count: " << orderCount << std::endl;

    return avgProfit;
}