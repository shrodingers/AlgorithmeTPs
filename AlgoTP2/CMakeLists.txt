cmake_minimum_required(VERSION 3.4)
project(AlgoTP1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        gestionnaire.cpp reseau.h auxilliaires.cpp arret.cpp ligne.cpp station.cpp Voyage.cpp coordonnees.cpp reseau.cpp)

add_executable(AlgoTP1 ${SOURCE_FILES})