#ifndef LIQUIDITY_HEATMAP
#define LIQUIDITY_HEATMAP

#include "Indicator.hpp"
#include "Candlestick.hpp"

#ifdef __linux__
#ifdef __x86_64__
#include "../onnxruntime-linux-x64-1.19.2/include/onnxruntime_cxx_api.h"
#endif
#endif


class LiquidityHeatmap : public Indicator
{   
    private:
        std::vector<Candlestick>& candlesticks;
        std::vector<double>& volume;
        std::vector<double>& volatilityValues;
        std::string modelPath;
        int resolution;

        const double rangeRatio = 1;
        double totalLiqVol = 0;
        double totalMarketVol = 0;
        bool compression = false;


        double normalPdf(double x, double mean, double sd);

#ifdef __linux__
#ifdef __x86_64__
        int predict(Ort::Session* session, std::vector<float>& input);
#endif
#endif

        std::vector<float> softmax(const std::vector<float>& logits);
        
    public:
        double boxSize;
        double rangeBottom, rangeTop;
        double maxLiquidity = 0;
        int rangeSizeHalf = 512;
        int normalization = false;

        double volatilityMultiplier = 17.85;//4.5;//18;
        double lossRatio = 0.9943;//0.9985;//0.9883;
        double liquidityDensityCoefficient = 0.00002;//0.0002;//0.000115;

        std::vector<std::vector<double>> heatmap;
        
        std::vector<double> liquidatedVolume;
        std::vector<double> marketVolume;

        std::vector<double> strategy;
        bool predictionEnabled;

        LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color, bool predictionEnabled, std::string modelPath);
        ~LiquidityHeatmap();
        void compute() override;
        void recompute();
};


#endif