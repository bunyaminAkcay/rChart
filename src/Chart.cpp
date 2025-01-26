#include <cstdio>
#include <raylib.h>
#include <utility>
#include <vector>
#include <limits>

#define RAYGUI_IMPLEMENTATION

#include "../external/raygui.h"
#include "../include/Chart.hpp"
#include "../include/viridis.hpp"
#include "../external/csvfile.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <cassert>


Chart::Chart(raylib::Window* chartWindow, std::string title, std::vector<Candlestick> candlestickData)
{
    HideCursor();
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0x000000FF);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
    
    this->chartSizeX = chartWindow->GetWidth();
    this->chartSizeY = chartWindow->GetHeight();
    this->title = title;
    chartWindow->SetTitle(title);
    this->candlestickData = candlestickData;
    centerPrice = candlestickData[candlestickData.size() -1].closePrice;
    candlesticksSize = candlestickData.size();

    //find proper zoomY
    double maxPrice = candlestickData[0].highPrice, minPrice = candlestickData[0].lowPrice;
    for (int i = 1; i < 100; i++)
    {
        maxPrice = candlestickData[i].highPrice > maxPrice ? candlestickData[i].highPrice : maxPrice;
        minPrice = candlestickData[i].lowPrice < minPrice ? candlestickData[i].lowPrice : minPrice;
    }
    int h = 0.6 * std::max(maxPrice - centerPrice, centerPrice - minPrice);
    zoomY = double(this->chartSizeY)/h;
}

void Chart::run(){
    
    std::cout << "rChart INFO: Chart initialized." << std::endl;

    while (!chartWindow->ShouldClose())
    {
        controller();

        BeginDrawing();

        chartWindow->ClearBackground(RAYWHITE);
        
        drawIndicators();
        drawCandlesticks();

        drawGUI();

        drawCursor();
        EndDrawing();
    }

}

void Chart::drawGUI(){
    
    const float checkBoxesX = 15;
    float checkBoxesY = 65;
    const float checkBoxSize = 15;
    const float checkBoxSpace = 5;


    //checkboxs for visibility
    for (int i = 0; i < indicators.size(); i++){
        
        bool visibilityValue = indicators[i]->visibility;
        bool* visibility = &visibilityValue;
        
        GuiCheckBox((Rectangle){ checkBoxesX, checkBoxesY, checkBoxSize, checkBoxSize}, indicators[i]->name.c_str(), visibility);
        checkBoxesY += checkBoxSize + checkBoxSpace;

        if (*visibility != indicators[i]->visibility){
            indicators[i]->visibility = *visibility;
        }

        if (indicators[i]->name == this->supportResistance->name && indicators[i]->visibility == true) {
            this->liquidityHeatmap->visibility = false;
        }
        if (indicators[i]->name == this->liquidityHeatmap->name && indicators[i]->visibility == true) {
            this->supportResistance->visibility = false;
        }
            

    }

    

    //Slider
    double space = 50;
    double top = 50;
    double sliderSizeX = 800;
    double sliderSizeY = 10;
    
    float volatilityMultiplierValue = liquidityHeatmap->volatilityMultiplier;
    float* volatilityMultiplier = &volatilityMultiplierValue;
    
    GuiSlider((Rectangle){ float(chartSizeX - sliderSizeX - space), float(top), float(sliderSizeX), float(sliderSizeY) }, "Volatility Multiplier", std::to_string(*volatilityMultiplier).c_str(), volatilityMultiplier, 0, 50);
    top += sliderSizeY + space;


    float lossRatioValue = liquidityHeatmap->lossRatio;
    float* lossRatio = &lossRatioValue;
    
    GuiSlider((Rectangle){ float(chartSizeX - sliderSizeX - space), float(top), float(sliderSizeX), float(sliderSizeY) }, "Loss Ratio", std::to_string(*lossRatio).c_str(), lossRatio, 0.97, 1.03);
    top += sliderSizeY + space;
    

    float liquidityDensityCoefficientValue = liquidityHeatmap->liquidityDensityCoefficient;
    float* liquidityDensityCoefficient = &liquidityDensityCoefficientValue;
    
    GuiSlider((Rectangle){ float(chartSizeX - sliderSizeX - space), float(top), float(sliderSizeX), float(sliderSizeY) }, "Liquidity Density", std::to_string(*liquidityDensityCoefficient).c_str(), liquidityDensityCoefficient, 0.00000, 0.0005);
    top += sliderSizeY + space;
    
    


    if (*volatilityMultiplier != liquidityHeatmap->volatilityMultiplier || *lossRatio != liquidityHeatmap->lossRatio || *liquidityDensityCoefficient != liquidityHeatmap->liquidityDensityCoefficient)
    {
        liquidityHeatmap->volatilityMultiplier = *volatilityMultiplier;
        liquidityHeatmap->lossRatio = *lossRatio;
        liquidityHeatmap->liquidityDensityCoefficient = *liquidityDensityCoefficient;

        liquidityHeatmap->recompute();
        supportResistance->recompute();
        
    }
    
}


Chart::~Chart()
{
}

void Chart::drawCandlesticks(){

    Color borderColor = LIGHTGRAY;

    if (liquidityHeatmap->visibility == false && supportResistance->visibility == false) {
        borderColor = BLACK;
    }

    for (int i = 0; i < candlestickData.size(); i++)
    {
        Candlestick candlestick = candlestickData[i];
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
        int offsetY = chartSizeY * 0.5 - zoomY * ( std::max(candlestick.openPrice, candlestick.closePrice) - centerPrice);

        int bodyHeight = int(abs(candlestick.openPrice - candlestick.closePrice) * zoomY);
        bodyHeight = bodyHeight < 1 ? 1 : bodyHeight;
        int upperWickSize = int((candlestick.highPrice - std::max(candlestick.openPrice, candlestick.closePrice)) * zoomY);
        int lowerWickSize = int((std::min(candlestick.openPrice, candlestick.closePrice) - candlestick.lowPrice) * zoomY);

        DrawRectangle(offsetX, offsetY, int(candleWidth), bodyHeight, borderColor);
        if (bodyHeight > 2 and int(candleWidth) > 2){
            DrawRectangle(offsetX+1, offsetY+1, int(candleWidth)-2, bodyHeight-2, BLUE);
        }

        int withOver2 = int(candleWidth/2);

        if (upperWickSize > 0){
            DrawLine(offsetX + withOver2+1, offsetY - upperWickSize, offsetX + withOver2+1, offsetY - upperWickSize + upperWickSize, borderColor);
        }
        
        if (lowerWickSize > 0){
            DrawLine(offsetX + withOver2+1, offsetY + bodyHeight, offsetX + withOver2+1 , offsetY + bodyHeight + lowerWickSize, borderColor);
        }
    }
}

void Chart::controller(){
    int mouseWheelMove = GetMouseWheelMove();
        if (mouseWheelMove != 0) {
            if (mouseWheelMove > 0) {
                candleWidth *= zoomSpeed;
            } else if (candleWidth > minCandleWidth) {
                candleWidth *= oneOverZoomSpeed;
            }
        }

        if (IsKeyPressed(KEY_KP_ADD)) {  // "+" key
            
            zoomY *= zoomSpeed;
        }
        if (IsKeyPressed(KEY_KP_SUBTRACT)) {  // "-" key
            zoomY *= oneOverZoomSpeed;
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 newPos = GetMousePosition();
            Vector2 delta = { newPos.x - lastPressedLocation.x, newPos.y - lastPressedLocation.y };
            lastPressedLocation = newPos;
            centerPrice += delta.y / zoomY;
            chartOffsetX -= delta.x;
        } else {
            lastPressedLocation = GetMousePosition();
        }
}

void Chart::drawCursor(){
    Vector2 pos = GetMousePosition();
    DrawLine(pos.x, 0, pos.x, chartSizeY, BLACK);
    DrawLine(0, pos.y, chartSizeX, pos.y, BLACK);
    DrawPixel(pos.x-1, pos.y-1, RED);
    double cursorPrice = getPriceFromY(pos.y);
    int textSize = 16;
    DrawText(std::to_string(cursorPrice).c_str(), chartSizeX - 80, pos.y - textSize, textSize, BLACK);
}

double Chart::getPriceFromY(int y){
    return (centerPrice - (y - chartSizeY * 0.5) / zoomY);
}

double Chart::getIndexFromX(int x){
    
    // f => int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
    // i = f^-1
    
    int i = - ( - (x - chartSizeX + chartOffsetX)/(int(candleWidth) + candlestickSpace) - candlesticksSize + 1) -1;
    return i;
}

void Chart::insertIndicator(Indicator* indicator){
    indicators.push_back(indicator);
    if (indicator->indicatorType == IndicatorType::liquidityheatmap)
    {
        LiquidityHeatmap* liquidityHeatmap = static_cast<LiquidityHeatmap*>(indicator);
        this->liquidityHeatmap = liquidityHeatmap;
    }
    
}

void Chart::drawLineIndicator(std::vector<double> data, int minY, int maxY, Color color){
    std::vector<int> drawableIndexNumbers;
    double maxValue = std::numeric_limits<double>::min();
    double minValue = std::numeric_limits<double>::max();
    
    for (int i = 0; i < data.size(); i++)
    {
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
        if (offsetX > 0 and offsetX < chartSizeX){
            drawableIndexNumbers.push_back(i);
            if (data[i] > maxValue){
                maxValue = data[i];
            }
                
            if (data[i] < minValue){
                minValue = data[i];
            }
        }
    }
    
    double ratio = (maxY - minY)/(maxValue - minValue);

    for(int i : drawableIndexNumbers){
        if (i == 0){
            continue;
        }
        int offsetX = chartSizeX - chartOffsetX - (data.size() -1.5 - i) * (int(candleWidth) + candlestickSpace);
        int offsetXpast = chartSizeX - chartOffsetX - (data.size() -1.5 - (i - 1)) * (int(candleWidth) + candlestickSpace);
        
        int offsetY = maxY - (data[i] - minValue) * ratio;
        int offsetYpast = maxY - (data[i-1] - minValue) * ratio;
        DrawLine(offsetXpast, offsetYpast, offsetX, offsetY, color);
    }
}

void Chart::drawHistogramIndicator(std::vector<double> data, int minY, int maxY, Color color, bool drawReverse){
    std::vector<int> drawableIndexNumbers;
    double maxValue = std::numeric_limits<double>::min();
    
    for (int i = 0; i < data.size(); i++){
        
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
        if (offsetX > 0 and offsetX < chartSizeX){
            drawableIndexNumbers.push_back(i);
            if (data[i] > maxValue){
                maxValue = data[i];
            }
        }
    }

    double ratio = (maxY - minY)/maxValue;

    for(int i : drawableIndexNumbers){
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);

        int height =  data[i] * ratio;
        if (!drawReverse){
            int offsetY = maxY - height;
            DrawRectangle(offsetX-1, offsetY-1, candleWidth+2, height+2, BLACK);
            DrawRectangle(offsetX, offsetY, candleWidth, height, color);
        }
        else
        {
            int offsetY = minY;
            DrawRectangle(offsetX-1, offsetY-1, candleWidth+2, height+2, BLACK);
            DrawRectangle(offsetX, offsetY, candleWidth, height, color);
        }
    }
}

void Chart::draw2DataHistogramIndicator(std::vector<double> data1, std::vector<double> data2, int minY, int maxY, Color color1, Color color2, bool drawReverse){
    std::vector<int> drawableIndexNumbers;
    double maxValue = std::numeric_limits<double>::min();
    
    for (int i = 0; i < data1.size(); i++){
        
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
        if (offsetX > 0 and offsetX < chartSizeX){
            drawableIndexNumbers.push_back(i);
            if (data1[i] + data2[i] > maxValue){
                maxValue = data1[i] + data2[i];
            }
        }
    }

    double ratio = (maxY - minY)/maxValue;

    for(int i : drawableIndexNumbers){
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);

        int height1 =  data1[i] * ratio;
        int height2 =  data2[i] * ratio;
        
        if (!drawReverse){
            DrawRectangle(offsetX -1, maxY - height1 - height2 -1, candleWidth +2 , height1 + height2 +2, BLACK);
            DrawRectangle(offsetX, maxY - height1, candleWidth, height1, color1);
            DrawRectangle(offsetX, maxY - height1 - height2, candleWidth, height2, color2);
        }
        else
        {
            DrawRectangle(offsetX -1, minY -1, candleWidth +2, height1 + height2 + 2, BLACK);
            DrawRectangle(offsetX, minY, candleWidth, height1, color1);
            DrawRectangle(offsetX, minY + height1, candleWidth, height2, color1);
        }
    }
}


void Chart::drawHeatmapIndicator(std::vector<std::vector<double>> data, double bottomPrice, double boxSize, double maxValue, Color color, bool withRange){

    int rangeSizeHalf = liquidityHeatmap->rangeSizeHalf; 
    double lastUpperRange = 0;
    double lastLowerRange = 0;

    Color rangeColor = ORANGE;

    for (int index = 0; index < data.size(); index++){

        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - index) * (int(candleWidth) + candlestickSpace);
        if (offsetX > 0 and offsetX < chartSizeX){
            

            for (int i = 0; i < data[index].size(); i++)
            {
                double low = -(bottomPrice + i * boxSize - centerPrice) * zoomY + chartSizeY * 0.5; 
                double high = low - boxSize * zoomY;
                if (low > 0 && high < chartSizeY)
                {
                    int size = int(low) - int(high);

                    int a = data[index][i]/maxValue * 255;
                    //assert( a >= 0 || a <= 255);
                    DrawRectangle(int(offsetX), int(high), int(candleWidth + candlestickSpace), size, raylib::Color(255*viridis[a][0], 255*viridis[a][1], 255*viridis[a][2]));
                }
            }
            
            if (withRange)
            {
                int w = int(candleWidth + candlestickSpace);
                double c = candlestickData[index].closePrice;

                double upperRange = -(c + boxSize * rangeSizeHalf - centerPrice) * zoomY + chartSizeY * 0.5;
                double lowerRange = -(c - boxSize * rangeSizeHalf - centerPrice) * zoomY + chartSizeY * 0.5;

                if (index != 0)
                {
                    DrawLine(int(offsetX - w), int(lastUpperRange), int(offsetX), int(upperRange), rangeColor);
                    DrawLine(int(offsetX - w), int(lastLowerRange), int(offsetX), int(lowerRange), rangeColor);
                }
                
                lastUpperRange = upperRange;
                lastLowerRange = lowerRange;

            }
        }
    }

    
    //paint right part of chart

    int remainingOffsetX = chartSizeX - chartOffsetX + (int(candleWidth) + candlestickSpace);
    int index = data.size() -1;

    if (remainingOffsetX > 0 && remainingOffsetX < chartSizeX)
    {
        for (int i = 0; i < data[index].size(); i++)
        {
            double low = -(bottomPrice + i * boxSize - centerPrice) * zoomY + chartSizeY * 0.5;; 
            double high = low - boxSize * zoomY;
            
            if (low > 0 && high < chartSizeY)
            {
                int size = int(low) - int(high);

                int a = data[index][i]/maxValue * 255;
                DrawRectangle(int(remainingOffsetX), int(high), chartSizeX - remainingOffsetX, size, raylib::Color(255*viridis[a][0], 255*viridis[a][1], 255*viridis[a][2]));
            }
        }
    }
    
}

void Chart::drawStrategy(std::vector<double> data, int minY, int maxY){
    std::vector<int> drawableIndexNumbers;
    
    for (int i = 0; i < data.size(); i++)
    {
        int offsetX = chartSizeX - chartOffsetX - (candlestickData.size() -1 - i) * (int(candleWidth) + candlestickSpace);
        if (offsetX > 0 and offsetX < chartSizeX){
            drawableIndexNumbers.push_back(i);
        }
    }
    
    for(int i : drawableIndexNumbers){
        if (i == 0){
            continue;
        }
        int offsetX = chartSizeX - chartOffsetX - (data.size() -1.5 - i) * (int(candleWidth) + candlestickSpace);
        int width = (int(candleWidth) + candlestickSpace);
        
        Color color;
        if (data[i] == 0)
        {
            color = GREEN;
        }
        else if (data[i] == 1)
        {
            color = RED;
        }
        else if (data[i] == 2)
        {
            color = BLACK;
        }
        else if (data[i] == 3)
        {
            color = GRAY;
        }
        

        DrawRectangle(offsetX, maxY, width, maxY - minY, color);
    }
}

void Chart::drawIndicators(){

    int lineIndicatorSize = 100, histogramIndicatorSize = 100, strategySize = 20;
    int space = 10;
    int bottomHeight = chartSizeY - space;

    int indexOnMouse = getIndexFromX(GetMousePosition().x);
    bool indexOnMouseValid = indexOnMouse >= 0 && indexOnMouse < candlesticksSize;

    for (int i = 0; i < indicators.size(); i++)
    {
        if (!indicators[i]->visibility) {
            continue;
        }

        if (IndicatorType::line == indicators[i]->indicatorType)
        {
            drawLineIndicator(indicators[i]->outputData, bottomHeight - lineIndicatorSize, bottomHeight, indicators[i]->color);
            bottomHeight -= lineIndicatorSize + space;

            if (indexOnMouseValid)
            {
                DrawText(std::to_string(indicators[i]->outputData[indexOnMouse]).c_str(), 20, bottomHeight + space, 16, indicators[i]->color);
            }
        }
        else if(IndicatorType::histogram == indicators[i]->indicatorType)
        {
            int fontSize = 16;

            drawHistogramIndicator(indicators[i]->outputData, bottomHeight - histogramIndicatorSize, bottomHeight, indicators[i]->color);
            bottomHeight -= histogramIndicatorSize + space;

            if (indexOnMouseValid)
            {
                DrawText(indicators[i]->name.c_str(), 20, bottomHeight + space, fontSize, indicators[i]->color);
                DrawText(std::to_string(indicators[i]->outputData[indexOnMouse]).c_str(), 20, bottomHeight + fontSize +  space, fontSize, indicators[i]->color);
            }
        }
        else if(IndicatorType::liquidityheatmap == indicators[i]->indicatorType)
        {
            Color color1 = ORANGE;
            Color color2 = RED;
            int fontSize = 16;

            LiquidityHeatmap* liquidityHeatmap = static_cast<LiquidityHeatmap*>(indicators[i]);
            
            drawHeatmapIndicator(liquidityHeatmap->heatmap, liquidityHeatmap->rangeBottom, liquidityHeatmap->boxSize, liquidityHeatmap->maxLiquidity, liquidityHeatmap->color, true);

            draw2DataHistogramIndicator( liquidityHeatmap->marketVolume, liquidityHeatmap->liquidatedVolume, bottomHeight - histogramIndicatorSize, bottomHeight, color1, color2);
            bottomHeight -= histogramIndicatorSize + space;

            if (indexOnMouseValid)
            {
                DrawText(liquidityHeatmap->name.c_str(), 20, bottomHeight + space, fontSize, color1);
                DrawText(std::to_string(liquidityHeatmap->liquidatedVolume[indexOnMouse]).c_str(), 20, bottomHeight + space + fontSize, fontSize, color2);
                DrawText(std::to_string(liquidityHeatmap->marketVolume[indexOnMouse]).c_str(), 20, bottomHeight + space + 2 * fontSize, fontSize, color1);
            }



        }
        else if(IndicatorType::supportresistance == indicators[i]->indicatorType){
            SupportResistance* supportResistance = static_cast<SupportResistance*>(indicators[i]);
            drawHeatmapIndicator(supportResistance->heatmap, supportResistance->rangeBottom, supportResistance->boxSize, supportResistance->maxLiquidity, supportResistance->color, true);
        }
        else if(IndicatorType::strategy == indicators[i]->indicatorType){

            drawStrategy(indicators[i]->outputData, space + bottomHeight - strategySize, bottomHeight);
            bottomHeight -= space + strategySize + space;

        }
    }

    int fontSize = 24;
    DrawText(title.c_str(), 16 + 1, fontSize + 1, fontSize, BLACK);
    DrawText(title.c_str(), 16 - 1, fontSize - 1 , fontSize, WHITE);

}
void Chart::saveSupportResistanceToCsv(std::vector<double>& desiredStrategyOutput, std::vector<std::vector<double>>& heatmapRef, std::string fileName, int columnSize){
    //csv:
    // label , --- 8 support power ---, --- 8 support distance ---
    // support and resistance are sorted from largest to smallest
    const int srLevelCount = 8;

    int liquidityHeatmapSize = heatmapRef.size();
    int rangeSizeHalf = liquidityHeatmap->rangeSizeHalf;

    std::cout << liquidityHeatmapSize << " " << rangeSizeHalf << std::endl;
    try {
        csvfile csv(fileName, ",");

        //start from index 1000
        //assume first 1000 candle does not reliable
        for (int i = 1000; i < candlesticksSize; i++){
            if (desiredStrategyOutput[i] == 3)// 3 == NoInfo
                continue;

            int output;
            if (desiredStrategyOutput[i] == 0){
                output = 1;
            }
            else if (desiredStrategyOutput[i] == 1){
                output = -1;
            }
            else{
                output = 0;
            }

            int closePriceIndex = (candlestickData[i].closePrice - liquidityHeatmap->rangeBottom)/liquidityHeatmap->boxSize;
            
            //close price can be so close to edge of the range.
            if ((rangeSizeHalf + closePriceIndex < liquidityHeatmapSize && closePriceIndex - rangeSizeHalf > 0) == false) {
                continue;
            }
            
            csv << output;

            // search max 8 value in column
            // index, value paires
            std::vector<std::pair<int, double>> maxValues;
            
            for(int lc = 0; lc < srLevelCount; lc++){
                
                double maxValue = std::numeric_limits<double>::lowest();
                int maxValueIndex = 0;
                int maxValueIndexP = 0;

                for (int p = -rangeSizeHalf; p < rangeSizeHalf; p++) {

                    int index = closePriceIndex + p;

                    bool indexNotUsed = true;
                    
                    for (std::pair<int, double> pair : maxValues){
                        if (pair.first == index){
                            indexNotUsed = false;
                            break;
                        }
                    }

                    if (indexNotUsed == false) {
                        continue;
                    }

                    if ( heatmapRef[i][index] > maxValue){
                        maxValue = heatmapRef[i][index];
                        maxValueIndex = index;
                        maxValueIndexP = p;
                    }
                }
                std::pair<int, double> pair(maxValueIndex, maxValue);
                maxValues.push_back(pair);

                csv << maxValueIndexP << pair.second;
            }
            csv << endrow;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception was thrown: " << e.what() << std::endl;
        
    }

}

void Chart::saveHeatmapToCsv(std::vector<double>& desiredStrategyOutput, ATR* atr, std::string fileName, bool compression, bool normalization){
    
    int rangeSizeHalf = liquidityHeatmap->rangeSizeHalf; 

    int compressDataIndexes[] = {0, 7, 21, 42, 77, 133, 224, 371}; 
    int compressDataMultipliers[] = {1, 2, 3, 5, 8, 13, 21};
    try
    {
        csvfile csv(fileName, ",");
        // Header
        //csv << "X" << "VALUE"        << endrow;
        // Data
        int liquidityHeatmapSize = liquidityHeatmap->heatmap.size();

        //start from index 1000
        for (int i = 1000; i < candlesticksSize; i++)
        {
            int closePriceIndex = (candlestickData[i].closePrice - liquidityHeatmap->rangeBottom)/liquidityHeatmap->boxSize;

            if (desiredStrategyOutput[i] == 3)// 3 == NoInfo
            {
                continue;
            }

            int output;
            if (desiredStrategyOutput[i] == 0)
            {
                output = 1;
            }
            else if (desiredStrategyOutput[i] == 1){
                output = -1;
            }
            else
            {
                output = 0;
            }
            
            if (rangeSizeHalf + closePriceIndex < liquidityHeatmapSize && closePriceIndex - rangeSizeHalf > 0)
            {
                csv << output;
                

                //compression is broke. fix in future
                //it is hard coded for rangeSizeHalf=371. You can use this value if you need
                //Actually no need for compression. If you want to compress data to reduce overfitting,
                //you can do it in machine learning side. It is unnecessery
                if (compression)
                {
                    /* compression */

                    double compressedData[compressDataSize];

                    // make sure compressedData is a zero array
                    for (int i = 0; i < compressDataSize; i++)
                    {
                        compressedData[i] = 0;
                    }
                    

                    for (int k = -rangeSizeHalf; k <= rangeSizeHalf; k++)
                    {
                        if (k == 0)
                        {
                            continue;
                        }

                        int absk = abs(k);
                        
                        int compressDataIndex = 0;
                        int baseIndex;
                        for (baseIndex = 0; baseIndex < 7; baseIndex++)
                        {
                            if (compressDataIndexes[baseIndex+1] < absk){
                                compressDataIndex +=7;
                            }
                            else
                            {
                                break;
                            }
                        }

                        compressDataIndex += (absk - compressDataIndexes[baseIndex] -1) / compressDataMultipliers[baseIndex];

                        if (k > 0)
                        {
                            compressDataIndex += 49;
                        }
                        else
                        {
                            compressDataIndex = 48-compressDataIndex;
                        }

                        compressedData[compressDataIndex] += liquidityHeatmap->heatmap[i][k + closePriceIndex];
                    }

                    /* compression end */

                    // normalize compression and save csv

                    double maxValue = 0;

                    for (int i = 0; i < compressDataSize; i++)
                    {
                        maxValue = maxValue < compressedData[i] ? compressedData[i] : maxValue; 
                    }

                    for (int i = 0; i < compressDataSize; i++)
                    {
                        compressedData[i] = compressedData[i]/maxValue;
                        csv << compressedData[i];
                    }
                }
                else
                {
                    double maxValue = 1;

                    if (normalization)
                    {
                        maxValue = 0;
                        for (int k = -rangeSizeHalf + closePriceIndex; k <= rangeSizeHalf + closePriceIndex; k++)
                        {
                            if (k == closePriceIndex){
                                continue;
                            }
                            maxValue = maxValue < liquidityHeatmap->heatmap[i][k] ? liquidityHeatmap->heatmap[i][k] : maxValue; 
                        }
                    }

                    for (int k = -rangeSizeHalf + closePriceIndex; k <= rangeSizeHalf + closePriceIndex; k++){
                        if (k == closePriceIndex){
                            continue;
                        }
                        csv << liquidityHeatmap->heatmap[i][k]/maxValue;
                    }
                }
                
                csv << (atr->outputData[i]/candlestickData[i].closePrice) << endrow;
                
            }
        }

    }
    catch (const std::exception& e)
    {
        std::cout << "Exception was thrown: " << e.what() << std::endl;
    }
}