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
    std::vector< std::vector<std::string> > linesRtc;
    std::vector< std::vector<std::string> > stationsRtc;
    std::vector< std::vector<std::string> > tripsRtc;
    std::vector< std::vector<std::string> > stopsRtc;
    std::vector< std::vector<std::string> > datesRtc;
    std::map<std::string, Ligne*> lines;
    std::list<Ligne*> orderedLines;
    std::vector<Station*> stations;
    std::vector<Voyage*> trips;
    std::vector<Arret*> stops;
    std::map<std::string, std::vector<Arret*> > VoyageStops;

    auto now = std::chrono::system_clock::now();

    lireFichier("RTC/routes.txt", linesRtc, ',', true);
    lireFichier("RTC/stops.txt", stationsRtc, ',', true);
    lireFichier("RTC/trips.txt", tripsRtc, ',', true);
    lireFichier("RTC/stop_times.txt", stopsRtc, ',', true);
    lireFichier("RTC/calendar_dates.txt", datesRtc, ',', true);

    for (auto& line : linesRtc) {
        lines[line[Ligne::routeIdIndex]] = new Ligne(line);
    }
    for (auto& line : stationsRtc) {
        stations.push_back(new Station(line));
    }
    for (auto& line : tripsRtc) {
        trips.push_back(new Voyage(line, lines[line[Voyage::routeIdIndex]]));
    }
    for (auto& line : stopsRtc) {
        Arret* stop = new Arret(line);
        stops.push_back(stop);
        VoyageStops[line[Arret::tripIdIndex]].push_back(stop);
    }

    Date today;
    Heure time(22,47,52);
    Heure timePlus = time.add_secondes(3600);
    std::vector<Voyage*> currentTrips;

    std::cout << "loading Done, time : " << time << timePlus << std::endl;

    for (auto& trip : trips) {
        std::list<Arret> currentStops;
        auto& VoyageStopsArray = VoyageStops[trip->getId()];
        std::for_each(VoyageStopsArray.begin(), VoyageStopsArray.end(), [&trip, &time, &timePlus, &currentStops] (Arret* stop) {
            currentStops.insert(std::lower_bound(currentStops.begin(), currentStops.end(), *stop), *stop);
        });
        std::vector<Arret> finalvec ({ std::make_move_iterator(std::begin(currentStops)),
                                       std::make_move_iterator(std::end(currentStops)) });
        if (!finalvec.empty()) trip->setArrets(finalvec);
    }

    auto then = std::chrono::system_clock::now();
    auto duration = std::chrono::duration<double, std::ratio<1,1> >(then - now);
    std::cout << "Chargement des données terminé en " << duration.count() << "secondes" << std::endl;

    const auto& lexCmp = [] (std::string const& s1, std::string const& s2) {
        const auto& countNumbers = [] (std::string const& str) {
            return std::count_if(str.begin(), str.end(), [] (char c) {
                return std::isdigit(c);
            });
        };
        return countNumbers(s1) == countNumbers(s2)
               ? std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end())
               : countNumbers(s1) < countNumbers(s2);
    };
    const std::string* max = nullptr;
    const std::string* min = nullptr;

    for (auto& elem: lines) {
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
                if (lexCmp(elem.second->getNumero(), (*it)->getNumero())) {
                    orderedLines.insert(it, elem.second);
                    break;
                }
            }
        }
    };
    std::sort(stations.begin(), stations.end(), [] (Station* st1, Station* st2) {
        return st1->getId() < st2->getId();
    });

    std::copy_if(trips.begin(), trips.end(), std::back_inserter(currentTrips), [&time, &timePlus, &datesRtc, &today] (Voyage* elem) {
        if (elem->getHeureDepart() >= time && elem->getHeureFin() <= timePlus) {
            for (auto& line : datesRtc) {
                Date date(StringConverter::fromString<unsigned int>(line[1].substr(0, 4)),
                          StringConverter::fromString<unsigned int>(line[1].substr(4, 2)),
                          StringConverter::fromString<unsigned int>(line[1].substr(6)));
                if (today == date && elem->getServiceId() == line[0]) return true;
            }
        }
        return false;
    });

    std::sort(currentTrips.begin(), currentTrips.end(), [&lexCmp] (Voyage* t1, Voyage* t2) {
        return t1->getHeureDepart() < t2->getHeureDepart();
    });


    std::ofstream os;
    os.open("resultats.txt", std::ofstream::out | std::ofstream::trunc);
    if (!os.good())
    {
        std::cerr << "Error while creating file" << std::endl;
        return -1;
    }

    os << "==================" << std::endl << "LIGNES DE LA RTC"
       << std::endl << "COMPTE : " << lines.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: orderedLines) os << *elem;
    os << "==================" << std::endl << "STATIONS DE LA RTC"
       << std::endl << "COMPTE : " << lines.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: stations) os << *elem;
    os << "VOYAGES DE LA JOURNEE" << std::endl;
    for (auto& elem: currentTrips) os << *elem;
    return 0;
}