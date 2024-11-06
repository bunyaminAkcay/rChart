#ifndef INDICATOR
#define INDICATOR

#include "../include-raylib/raylib-cpp.hpp"
#include <string>
#include <vector>
#include <limits>

enum IndicatorType { line, histogram, heatmap, strategy };

class Indicator
{
    public:
    
        std::string name;
        Color color;
        std::vector<double> outputData;
        IndicatorType indicatorType;

        Indicator();
        virtual ~Indicator() {};
        virtual void compute() = 0;
};

#endif