#include "Indicator.hpp"
#include "Candlestick.hpp"
#include "../include-raylib/raylib-cpp.hpp"


class Volume : public Indicator
{   
    private:
        std::vector<Candlestick>& candlesticks;
    public:
        Volume(std::vector<Candlestick>& candlesticks, raylib::Color color);
        ~Volume();
        void compute() override;
};