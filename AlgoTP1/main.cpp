//
// Created by Leon on 18/09/2016.
//

#include <algorithm>
#include <map>
#include <chrono>
#include "Enonce/auxiliaires.h"
#include "Enonce/arret.h"
#include "Enonce/ligne.h"
#include "Enonce/station.h"
#include "Enonce/voyage.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        return -1;
    }

    std::vector< std::vector<std::string> > linesRtc;
    std::vector< std::vector<std::string> > stationsRtc;
    std::vector< std::vector<std::string> > tripsRtc;
    std::vector< std::vector<std::string> > stopsRtc;
    std::map<std::string, Ligne*> lines;
    std::list<Ligne*> orderedLines;
    std::vector<Station*> stations;
    std::vector<Voyage*> trips;
    std::vector<Arret*> stops;

    auto now = std::chrono::system_clock::now();

    lireFichier("RTC/routes.txt", linesRtc, ',', true);
    lireFichier("RTC/stops.txt", stationsRtc, ',', true);
    lireFichier("RTC/trips.txt", tripsRtc, ',', true);
    lireFichier("RTC/stop_times.txt", stopsRtc, ',', true);

    for (auto& line : linesRtc) {
        lines[line[Ligne::routeIdIndex]] = new Ligne(line);
    }
    /*for (auto& line : stationsRtc) {
        stations.push_back(new Station(line));
    }*/
    for (auto& line : tripsRtc) {
        trips.push_back(new Voyage(line, lines[line[Voyage::routeIdIndex]]));
    }
    for (auto& line : stopsRtc) {
        stops.push_back(new Arret(line));
    }

    auto then = std::chrono::system_clock::now();
    auto duration = std::chrono::duration<double, std::ratio<1,1> >(then - now);
    std::cout << "Chargement des données terminé en " << duration.count() << "secondes" << std::endl;

    const auto& lexCmp = [] (std::string const& s1, std::string const& s2) {
        return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), [&s1, &s2] (char c1, char c2){
            return s1.length() < s2.length() || c1 < c2;
        });
    };

    for (auto& elem: lines) {
        const std::string* max = nullptr;
        const std::string* min = nullptr;

        if (orderedLines.empty()) {
            orderedLines.push_back(elem.second);
            max = &(elem.second->getNumero());
            min = &(elem.second->getNumero());
        } else if (min && lexCmp(elem.second->getNumero(), *min)) {
            orderedLines.push_front(elem.second);
            min = &(elem.second->getNumero());
        } else if (max && !lexCmp(elem.second->getNumero(), *max)) {
            orderedLines.push_back(elem.second);
            max = &(elem.second->getNumero());
        } else {
            for (std::list<Ligne*>::iterator it = orderedLines.begin(); it != orderedLines.end(); ++it) {
                if (!lexCmp(elem.second->getNumero(), (*it)->getNumero())) {
                    orderedLines.insert(it, elem.second);
                    break;
                }
            }
        }
    };

    std::ofstream os;
    os.open("resultats.txt", std::ofstream::out | std::ofstream::trunc);
    if (!os.good())
    {
        std::cerr << "Error while creating file" << std::endl;
        return -1;
    }

    os << "==================" << std::endl << "LIGNES DE LA RTC"
       << std::endl << "COMPTE : " << lines.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: orderedLines) {
        os << *elem;
    }
    return 0;
}