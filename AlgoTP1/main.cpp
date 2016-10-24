//
// Created by Leon on 18/09/2016.
//

#include <algorithm>
#include <map>
#include <chrono>
#include <unordered_map>
#include "auxiliaires.h"
#include "arret.h"
#include "ligne.h"
#include "station.h"
#include "voyage.h"

int main(int argc, char **argv) {

    //Déclaration des structures de données
    std::vector< std::vector<std::string> > linesRtc;
    std::vector< std::vector<std::string> > stationsRtc;
    std::vector< std::vector<std::string> > tripsRtc;
    std::vector< std::vector<std::string> > stopsRtc;
    std::vector< std::vector<std::string> > datesRtc;
    std::unordered_map<std::string, std::pair<Ligne*, std::vector<Voyage*> > > lines;
    std::list<Ligne*> orderedLines;
    std::vector<Station*> stations;
    std::vector<Voyage*> trips;
    std::vector<Arret*> stops;
    std::unordered_map<std::string, std::vector<Arret*> > VoyageStops;

    //initialisation du timer pour le parsing
    auto now = std::chrono::system_clock::now();

    //parsing des différents fichiers de données de la RTC
    lireFichier("RTC/routes.txt", linesRtc, ',', true);
    lireFichier("RTC/stops.txt", stationsRtc, ',', true);
    lireFichier("RTC/trips.txt", tripsRtc, ',', true);
    lireFichier("RTC/stop_times.txt", stopsRtc, ',', true);
    lireFichier("RTC/calendar_dates.txt", datesRtc, ',', true);

    //Constructuion des Objets
    for (auto& line : linesRtc) {
        lines[line[Ligne::routeIdIndex]].first = new Ligne(line);
    }
    for (auto& line : stationsRtc) {
        stations.push_back(new Station(line));
    }
    for (auto& line : tripsRtc) {
        Voyage* trip = new Voyage(line, lines[line[Voyage::routeIdIndex]].first);
        trips.push_back(trip);
        lines[line[Voyage::routeIdIndex]].second.push_back(trip);
    }
    for (auto& line : stopsRtc) {
        Arret* stop = new Arret(line);
        stops.push_back(stop);
        VoyageStops[line[Arret::tripIdIndex]].push_back(stop);
    }
    for (auto& elem : lines) {
        elem.second.first->setVoyages(elem.second.second);
    }

    Date today;
    Heure time(22, 47, 52);
    Heure timePlus = time.add_secondes(3600);
    std::vector<Voyage*> currentTrips;

    // Ajout des Arrêts aux voyages
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

    //Foncteur de comparaison lexicographique pour le tri des noms des lignes de bus
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

    // Algorithme de tri par insertion 'A la main'
    const std::string* max = nullptr;
    const std::string* min = nullptr;

    for (auto& elem: lines) {
        if (orderedLines.empty()) {
            orderedLines.push_back(elem.second.first);
            max = &(elem.second.first->getNumero());
            min = &(elem.second.first->getNumero());
        } else if (min && lexCmp(elem.second.first->getNumero(), *min)) {
            orderedLines.push_front(elem.second.first);
            min = &(elem.second.first->getNumero());
        } else if (max && !lexCmp(elem.second.first->getNumero(), *max)) {
            orderedLines.push_back(elem.second.first);
            max = &(elem.second.first->getNumero());
        } else {
            for (std::list<Ligne*>::iterator it = orderedLines.begin(); it != orderedLines.end(); ++it) {
                if (lexCmp(elem.second.first->getNumero(), (*it)->getNumero())) {
                    orderedLines.insert(it, elem.second.first);
                    break;
                }
            }
        }
    };

    //Tri des stations
    std::sort(stations.begin(), stations.end(), [] (Station* st1, Station* st2) {
        return st1->getId() < st2->getId();
    });

    // Selection des voyages à afficher selon leur date et temps de début et de fin
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

    // Tri des voyages en fonction de leur heure de départ
    std::sort(currentTrips.begin(), currentTrips.end(), [&lexCmp] (Voyage* t1, Voyage* t2) {
        return t1->getHeureDepart() < t2->getHeureDepart();
    });

    //Fin du temps de création et chargement des informations
    auto then = std::chrono::system_clock::now();
    //Calcul de la durée
    auto duration = std::chrono::duration<double, std::ratio<1,1> >(then - now);

    std::cout << "Chargement des données terminé en " << duration.count() << "secondes" << std::endl;

    std::ofstream os;
    os.open("resultats.txt", std::ofstream::out | std::ofstream::trunc);
    if (!os.good())
    {
        std::cerr << "Error while creating file" << std::endl;
        return -1;
    }
    // Ecriture des informations dans le fichier
    os << "==================" << std::endl << "LIGNES DE LA RTC"
       << std::endl << "COMPTE : " << lines.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: orderedLines) os << *elem;
    os << "==================" << std::endl << "STATIONS DE LA RTC"
       << std::endl << "COMPTE : " << lines.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: stations) os << *elem;
    os << "==================" << std::endl << "VOYAGES DE LA JOURNEE DU " << today << std::endl <<
       time << " - " << timePlus << std::endl << "COMPTE" << " = " << currentTrips.size() << std::endl << "==================" << std::endl << std::endl;
    for (auto& elem: currentTrips) os << *elem;
    return 0;
}