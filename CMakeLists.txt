cmake_minimum_required(VERSION 3.10)
project(rChart)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")


# Add the src directory
add_subdirectory(include-raylib)

set(SOURCES
    src/main.cpp
    src/Chart.cpp
    src/API.cpp
    src/Indicator.cpp
    src/ATR.cpp
    src/Volume.cpp
    src/LiquidityHeatmap.cpp
)

set(HEADERS
    external/httplib.h
    external/json.hpp
    external/raygui.h
    external/csvfile.h
    include/Chart.hpp
    include/Candlestick.hpp
    include/Indicator.hpp
    include/API.hpp
    include/ATR.hpp
    include/Volume.hpp
    include/LiquidityHeatmap.hpp
    include/viridis.hpp
    include/DesiredStrategy.hpp
)

include_directories(${CMAKE_SOURCE_DIR}/onnxruntime-linux-x64-1.19.2/include)

# Add the main.cpp file
add_executable(rChart ${SOURCES} ${HEADERS})

# Link the raylib_cpp library to the executable
target_link_libraries(rChart raylib)

find_package(OpenSSL REQUIRED)
target_link_libraries(rChart OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(rChart ${CMAKE_SOURCE_DIR}/onnxruntime-linux-x64-1.19.2/lib/libonnxruntime.so)
