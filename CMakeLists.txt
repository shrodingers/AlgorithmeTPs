cmake_minimum_required(VERSION 3.4)
project(AlgoTP1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(source_TP1 )

set(SOURCE_TP2
        AlgoTP2/gestionnaire.h AlgoTP2/reseau.h AlgoTP2/main.cpp AlgoTP2/reseau.cpp AlgoTP2/gestionnaire.cpp
        AlgoTP2/auxilliaires.cpp AlgoTP2/arret.cpp AlgoTP2/ligne.cpp AlgoTP2/station.cpp AlgoTP2/Voyage.cpp AlgoTP2/coordonnees.cpp)

add_executable(AlgoTP2 ${SOURCE_TP2})