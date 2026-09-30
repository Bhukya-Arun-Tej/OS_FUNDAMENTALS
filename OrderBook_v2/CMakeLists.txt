cmake_minimum_required(VERSION 3.20)

project(OrderBookV2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(orderbook
    main.cpp

    Orderbook.cpp
    Orderbook.hpp
    Order.hpp
    OrderLocation.hpp
    Trade.hpp
    TradeInfo.hpp
)
