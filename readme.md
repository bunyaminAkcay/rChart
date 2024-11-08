# rChart

rChart is a trading chart program that includes various indicators. It fetches candlestick data from the Binance API, processes the data, and draws the chart and indicators using the Raylib library. The liquidity heatmap indicator is an original and unique feature that estimates liquidity levels.

## Controls

You can navigate the chart using the mouse and zoom in or out with "+" and "-" keys on the touchpad.

## Liquidity Heatmap Indicator

The Liquidity Heatmap predicts potential liquidations at various price levels, a key factor in price action analysis. In many price action techniques, identifying areas of liquidity consolidation and gaps is essential for understanding market dynamics. rChart Liquidity Heatmap indicator achieves this by algorithmically analyzing price and volume history to highlight areas with potential liquidation activity.

![alt text](https://github.com/bunyaminAkcay/rChart/blob/master/images/screenshot.png?raw=true)

## How liquidity heatmap indicator works?

At any given time t, it is assumed that the total accumulated liquidity in the market can be modeled with a normal distribution. This model assumes that the variance of the normal distribution is correlated to a volatility indicator, while the amplitude of the distribution is correlated to the volume of limit order liquidity. At each moment t, some amount of liquidity is reduced by market orders, and the remaining liquidity is carried over to t+1, allowing it to accumulate in the market.

## Requirements

The cpp-httplib library is used to send HTTPS requests to the Binance API. For SSL support, the libssl and libcrypto libraries must be linked to the project. For more details, you can check the [cpp-httplib repository](https://github.com/yhirose/cpp-httplib).

## Build

Create a build folder:

```
mkdir build
```

run bash script to build and run:

```
source buildAndRun.sh
```

## Classification

The project is also aims to make predictions of buy and sell signals with liquidty heatmap data. rChart can save the heatmap as csv and it is possible to learn from this data. Some python files in classification folder aims to classify the data buy, sell and not trade signals. However, classification is not successful for now. After a classification, you can produce a onnx model than you can use in rChart. To use prediction in rChart from onnx file, only linux x86_64 is supported. You can install onnx runtime library for your system and then you can implement the code.