#ifndef LIQUIDITY_HEATMAP
#define LIQUIDITY_HEATMAP

#include "Indicator.hpp"
#include "Candlestick.hpp"
#include "Order.hpp"
#include <vector>



class LiquidityHeatmap : public Indicator
{   
    private:
        
        std::vector<Candlestick>& candlesticks;
        std::vector<double>& volume;
        std::vector<double>& volatilityValues;
        int resolution;

        const double rangeRatio = 2;
        double totalLiqVol = 0;
        double totalMarketVol = 0;
        bool compression = false;


        double normalPdf(double x, double mean, double sd);
        double cauchyPdf(double x, double x0, double gamma);
        double student_t_pdf(double t, double mu, double sigma, double df);
        std::vector<float> softmax(const std::vector<float>& logits);
        Order calculateStrategy(std::vector<double>& heatmapColumn, int candleIndex);
        Order calculateStrategy2(std::vector<double>& heatmapColumn, int candleIndex);
        
    public:

        bool withStrategy = false;
        
        double boxSize;
        double rangeBottom, rangeTop;
        double maxLiquidity = 0;
        int rangeSizeHalf = 512;
        int normalization = false;

        double volatilityMultiplier = 0.069;
        double lossRatio = 0.998;
        double liquidityDensityCoefficient = 1;//1;
        double q = 1;//0.9;
        double df = 4;//6;
        
        double colorDensity = 1;

        double oneOverAvgLiqBox = 100;

        std::vector<std::vector<double>> heatmap;
        
        std::vector<double> liquidatedVolume;
        std::vector<double> marketVolume;

        std::vector<double> liqChangeValues;

        double liqChangeValue = 0;


        //about strategy
        std::vector<double> liqChange;
        std::vector<double> orderUpperPrices;
        std::vector<double> orderLowerPrices;
        std::vector<double> orderTypes;

        double trustAtr = 0.7;
        double tpTrustCoef = 0.93;

        double reduceCoef = 0.7;
        double lowScoreLimit = 3;

        double minReward = 0.3;
        double minFinalReward = 0.7;

        double minAtrLimit = 2.5, maxAtrLimit = 7;
        //about strategy end

        LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color, bool withStrategy = false);
        ~LiquidityHeatmap();
        void compute() override;
        void recompute();
};


#endif