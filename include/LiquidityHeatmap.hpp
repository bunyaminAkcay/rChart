#ifndef LIQUIDITY_HEATMAP
#define LIQUIDITY_HEATMAP

#include "Indicator.hpp"
#include "Candlestick.hpp"

class LiquidityHeatmap : public Indicator
{   
    private:
        std::vector<Candlestick>& candlesticks;
        std::vector<double>& volume;
        std::vector<double>& volatilityValues;
        int resolution;

        const double rangeRatio = 1;
        double totalLiqVol = 0;
        double totalMarketVol = 0;
        bool compression = false;


        double normalPdf(double x, double mean, double sd);

        std::vector<float> softmax(const std::vector<float>& logits);
        
    public:
        double boxSize;
        double rangeBottom, rangeTop;
        double maxLiquidity = 0;
        int rangeSizeHalf = 512;
        int normalization = false;

        double volatilityMultiplier = 13.83;//17.85;//4.5;//18;
        double lossRatio = 0.99;//0.9985;//0.9883;
        double liquidityDensityCoefficient = 0.000068;//0.0002;//0.000115;

        std::vector<std::vector<double>> heatmap;
        
        std::vector<double> liquidatedVolume;
        std::vector<double> marketVolume;

        LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color);
        ~LiquidityHeatmap();
        void compute() override;
        void recompute();
};


#endif