cmake_minimum_required(VERSION 3.6)
project(AlgoTP1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        Enonce/arret.h
        Enonce/auxiliaires.h
        Enonce/coordonnees.h
        Enonce/ligne.h
        Enonce/station.h
        Enonce/voyage.h
    arret.h main.cpp auxilliaires.cpp arret.cpp ligne.cpp Voyage.cpp)

add_executable(AlgoTP1 ${SOURCE_FILES})