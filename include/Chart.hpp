#include "../include-raylib/raylib-cpp.hpp"
#include "Candlestick.hpp"
#include "Indicator.hpp"
#include "LiquidityHeatmap.hpp"
#include "ATR.hpp"
#include <string>
#include <vector>

class Chart
{
    private:
        const int compressDataSize = 98; 
        std::string title;
        int chartSizeX;
        int chartSizeY;
        raylib::Window* chartWindow;
        std::vector<Candlestick> candlestickData;
        int candlesticksSize;
        const int candlestickSpace = 2;
        double candleWidth = 6.0;
        int chartOffsetX = 100;
        double centerPrice;
        double zoomY;
        int minCandleWidth = 2;
        double zoomSpeed = 1.15;
        double oneOverZoomSpeed = 1/zoomSpeed;
        Vector2 lastPressedLocation;
        std::vector<Indicator*> indicators;
        LiquidityHeatmap* liquidityHeatmap;

        void drawCandlesticks();
        void controller();
        void drawCursor();
        void drawGUI();
        double getPriceFromY(int y);
        double getIndexFromX(int x);
        void drawLineIndicator(std::vector<double> data, int minY, int maxY, Color color);
        void drawHistogramIndicator(std::vector<double> data, int minY, int maxY, Color color, bool drawReverse = false);
        void draw2DataHistogramIndicator(std::vector<double> data1, std::vector<double> data2, int minY, int maxY, Color color1, Color color2, bool drawReverse = false);
        void drawHeatmapIndicator(std::vector<std::vector<double>> data, double bottomPrice, double boxSize, double maxValue, Color color, bool withRange = false);
        void drawStrategy(std::vector<double> data, int minY, int maxY);
        void drawIndicators();

    public:
        Chart(raylib::Window* chartWindow, std::string title, std::vector<Candlestick> candlestickData);
        ~Chart();
        void run();
        void insertIndicator(Indicator* indicator);
        void saveHeatmapToCsv(std::vector<double>& desiredStrategyOutput, ATR* atr, std::string fileName, bool compression=false, bool normalization=false);
};
