//
// Created by Leon on 18/09/2016.
//

#include <algorithm>
#include "Enonce/auxiliaires.h"
#include "Enonce/arret.h"
#include "Enonce/ligne.h"
#include "Enonce/voyage.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        return -1;
    }

    std::vector< std::vector<std::string> > results;
    std::vector<Ligne*> lines;

    lireFichier(argv[1], results, ',', true);
    for (auto& line : results) {
        lines.push_back(new Ligne(line));
    }
    results.clear();
    lireFichier(argv[2], results, ',', true);
    int count = 0;
    for(auto& line : results) {
        if (count == 242)
            return 0;
        ++count;
        Voyage test(line, *std::find_if(lines.begin(), lines.end(),
                                        [&line](Ligne* toFind) { return StringConverter::fromString<unsigned int>(line[0]) == toFind->getId(); }));
        std::cout << test;
    }
}