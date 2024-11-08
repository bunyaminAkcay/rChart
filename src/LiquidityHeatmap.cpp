#include "../include/LiquidityHeatmap.hpp"
#include "onnxruntime_cxx_api.h"
#include <math.h>
#include <iostream>
#include <cstddef>
#include <algorithm>
#include <numeric>

LiquidityHeatmap::LiquidityHeatmap(std::vector<Candlestick>& candlesticks, std::vector<double>& volume, std::vector<double>& volatilityValues, int resolution, raylib::Color color, bool predictionEnabled, std::string modelPath)
    : Indicator(),
    candlesticks(candlesticks),
    volume(volume),
    volatilityValues(volatilityValues),
    resolution(resolution),
    rangeBottom(candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)),
    rangeTop(candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)),
    boxSize(((candlesticks[candlesticks.size() -1].closePrice * (1 + 0.5 * rangeRatio)) - (candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio) < 0 ? 0 : candlesticks[candlesticks.size() -1].closePrice * (1 - 0.5 * rangeRatio)))/double(resolution)),
    predictionEnabled(predictionEnabled),
    modelPath(modelPath)
{
    this->name = "Liquidity Heatmap";
    this->color = color;
    this->indicatorType = IndicatorType::heatmap;

}

std::vector<float> LiquidityHeatmap::softmax(const std::vector<float>& logits) {
    std::vector<float> exp_logits(logits.size());
    float max_logit = *std::max_element(logits.begin(), logits.end());

    // Compute e^(logit - max_logit)
    std::transform(logits.begin(), logits.end(), exp_logits.begin(),
                   [max_logit](float logit) { return std::exp(logit - max_logit); });

    // Compute sum of exponentials
    float sum_exp_logits = std::accumulate(exp_logits.begin(), exp_logits.end(), 0.0f);

    // Compute softmax probabilities
    std::vector<float> probabilities(logits.size());
    std::transform(exp_logits.begin(), exp_logits.end(), probabilities.begin(),
                   [sum_exp_logits](float exp_logit) { return exp_logit / sum_exp_logits; });

    return probabilities;
}

#ifdef __linux__
#ifdef __x86_64__

int LiquidityHeatmap::predict(Ort::Session* session ,std::vector<float>& input){
    
    // Get model input/output information
    Ort::AllocatorWithDefaultOptions allocator;

    // Input information
    std::size_t num_input_nodes = session->GetInputCount();

    Ort::AllocatedStringPtr inputName = session->GetInputNameAllocated(0, allocator);
    
    Ort::TypeInfo input_type_info = session->GetInputTypeInfo(0);
    auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
    std::vector<int64_t> input_node_dims = input_tensor_info.GetShape();

    // Output information
    std::size_t num_output_nodes = session->GetOutputCount();
    Ort::AllocatedStringPtr outputName  = session->GetOutputNameAllocated(0, allocator);
    
    // Example: create input data
    std::vector<int64_t> input_shape = {1, int64_t(input.size())};
    
    // Create input tensor
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), input_shape.data(), input_shape.size());

    // Run the model
    std::vector<const char*> input_names = {inputName.get()};
    std::vector<const char*> output_names = {outputName.get()};
    auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names.data(), &input_tensor, 1, output_names.data(), 1);

    // Process the output
    float* output = output_tensors[0].GetTensorMutableData<float>();
    
    //std::cout << output[0] << " " << output[1] << " " << output[2] << std::endl;
    
    int prediction = 3;

    if (isnan(output[0]) || isnan(output[1]) || isnan(output[2]))
    {
        return prediction;
    }

    if (output[0] >= output[1] && output[0] >= output[2]){
        prediction = 2; 
    }
    else if (output[1] >= output[0] && output[1] >= output[2])
    {
        prediction = 1;
    }
    else if (output[2] >= output[0] && output[2] >= output[1])
    {
        prediction = 0;
    }

    //std::cout << prediction << std::endl;

    return prediction;
}
#endif
#endif

LiquidityHeatmap::~LiquidityHeatmap(){
}

void LiquidityHeatmap::recompute(){
    heatmap.clear();
    heatmap.shrink_to_fit();
    liquidatedVolume.clear();
    liquidatedVolume.shrink_to_fit();
    marketVolume.clear();
    marketVolume.shrink_to_fit();
    strategy.clear();
    strategy.shrink_to_fit();

    compute();
}

void LiquidityHeatmap::compute(){

    int compressDataIndexes[] = {0, 7, 21, 42, 77, 133, 224, 371}; 
    int compressDataMultipliers[] = {1, 2, 3, 5, 8, 13, 21};
    const int compressDataSize = 98;

    
    

    #ifdef __linux__
    #ifdef __x86_64__

    Ort::Session* sessionPointer;

    if (predictionEnabled) {
        // Load the ONNX model
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
        // Create ONNX Runtime session options
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        Ort::Session session(env, modelPath.c_str(), session_options);
        sessionPointer = &session;
    }
    
    #endif
    #endif
    
    double totalVolume = 0;

    for (int k = 0; k < volume.size(); k++)
    {
        totalVolume += volume[k];
    }
    
    double averageVolume = totalVolume / volume.size();
    
    totalLiqVol = 50 * averageVolume;
    totalMarketVol = 50 * averageVolume;

    for (int index = 0; index < candlesticks.size(); index++)
    {
        std::vector<double> heatmapColumn;
        liquidatedVolume.push_back(0);
        

        Candlestick c = candlesticks[index];

        double meanPrice = (c.closePrice + c.openPrice) * 0.5;

        std::vector<double> lastLiq;

        if (index == 0){
            lastLiq = std::vector<double>(resolution, 0);
        }
        else
        {
            lastLiq = heatmap[index-1];
        }
        
        for (int i = 0; i < resolution; i++)
        {
            double price = rangeBottom + (i + 1) * boxSize;
            double liq;

            if (price > c.lowPrice && price < c.highPrice ){

                liquidatedVolume[index] += lastLiq[i];
                liq = 0;
            }
            else{
                double sd = volatilityMultiplier * volatilityValues[index];
                double lqVol = i == 0 ? 0 : liquidatedVolume[index-1];
                double vol = i == 0 ? 0 : volume[index-1];
                double lastMarketVol = abs(vol - lqVol);
                double liquidityDensity = (price * liquidityDensityCoefficient) * (totalMarketVol/totalLiqVol); //dont ask me why
                liq = lossRatio * lastLiq[i] + liquidityDensity * normalPdf(price, meanPrice, sd) * lastMarketVol;
            }   

            if (liq > maxLiquidity)
            {
                maxLiquidity = liq;
            }
            
            heatmapColumn.push_back(liq);
        }

        double marketVol = volume[index] - liquidatedVolume[index];
        marketVolume.push_back(marketVol);

        totalMarketVol += marketVol > 0 ? marketVol : 0;
        totalLiqVol += liquidatedVolume[index] > 0 ? liquidatedVolume[index] : 0;

        heatmap.push_back(heatmapColumn);



        if (predictionEnabled)
        {   
            #ifdef __linux__
            #ifdef __x86_64__
            int closePriceIndex = (candlesticks[index].closePrice - rangeBottom)/boxSize;

            std::vector<float> dataForPrediction;

            if (rangeSizeHalf + closePriceIndex < heatmapColumn.size() && closePriceIndex - rangeSizeHalf > 0)
            {
                
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

                        compressedData[compressDataIndex] += heatmapColumn[k + closePriceIndex];
                    }

                    double maxValue = 0;

                    for (int i = 0; i < compressDataSize; i++)
                    {
                        maxValue = maxValue < compressedData[i] ? compressedData[i] : maxValue; 
                    }

                    for (int i = 0; i < compressDataSize; i++)
                    {
                        compressedData[i] = compressedData[i]/maxValue;
                        dataForPrediction.push_back(compressedData[i]);
                    }

                    /* compression end */
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
                            maxValue = maxValue < heatmapColumn[k] ? heatmapColumn[k] : maxValue; 
                        }
                    }

                    for (int k = -rangeSizeHalf + closePriceIndex; k <= rangeSizeHalf + closePriceIndex; k++){
                        if (k == closePriceIndex){
                            continue;
                        }
                        dataForPrediction.push_back(heatmapColumn[k]/maxValue);
                    }
                }
                
                dataForPrediction.push_back(volatilityValues[index]/candlesticks[index].closePrice);

                int prediction = predict(sessionPointer, dataForPrediction);

                if (prediction == 0)
                {
                    strategy.push_back(0);
                }
                else if (prediction == 2){
                    strategy.push_back(1);
                }
                else if (prediction == 1){
                    strategy.push_back(2);
                }
                else if (prediction == 3){
                    strategy.push_back(3);
                }
                else
                {
                    std::cout << "Unknown output" << std::endl;
                    exit(-1);
                }


            }
            else
            {
                strategy.push_back(3);
            }
            
            #else
                printf("rChart Warn: Prediction with onnx model is only possible for x86 x64 now. You can install it and implement yourself.")
            #endif
            #else
                printf("rChart Warn: Prediction with onnx model is only possible for linux now. You can install it and implement yourself.")
            #endif
        }
        
    }

    //print accuraccy
    
}

double LiquidityHeatmap::normalPdf(double x, double mean, double sd){
    
    if (sd == 0)
    {
        return 0;
    }

    double var = sd * sd;
    double denom = sqrt(2 * PI * var);
    double num = exp(-pow(x - mean, 2) / (2 * var));
    return num/denom;
}