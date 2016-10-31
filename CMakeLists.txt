cmake_minimum_required(VERSION 3.4)
project(AlgoTP1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(source_TP1 )

set(SOURCE_TP2
        AlgoTP2/gestionnaire.h AlgoTP2/reseau.h AlgoTP2/main.cpp AlgoTP2/reseau.cpp)

add_executable(AlgoTP2 ${SOURCE_TP2})