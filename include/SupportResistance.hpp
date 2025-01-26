#ifndef SUPPORT_RESISTANCE
#define SUPPORT_RESISTANCE

#include "Indicator.hpp"

class SupportResistance : public Indicator {


    private:
        std::vector<std::vector<double>>& liquidityHeatmapRef;
        
    public:
        double rangeBottom, boxSize, maxLiquidity;
        std::vector<std::vector<double>> heatmap;
        SupportResistance(std::vector<std::vector<double>>& liquidityHeatmapRef, double rangeBottom, double boxSize, Color color);
        ~SupportResistance();
        void compute();
        void recompute();
};

#endif