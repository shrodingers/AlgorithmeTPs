//
// Created by Leon on 18/09/2016.
//

#include "Enonce/auxiliaires.h"
#include "Enonce/arret.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    std::vector< std::vector<std::string> > results;

    lireFichier(argv[1], results, ',', true);
    for(auto& line : results) {
        Arret test(line);
        std::cout << test;
    }
}