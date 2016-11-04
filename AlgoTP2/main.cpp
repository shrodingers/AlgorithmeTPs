//
// Created by Leon on 16/10/2016.
//

#include <fstream>
#include "reseau.h"
#include "gestionnaire.h"

int main() {
    std::unordered_map<std::string, std::pair<std::string, std::function<void(Gestionnaire*)> > > options;
    auto askDate = [] () {
        unsigned int year, month, day;
        std::string buff;
        std::cout << "Entrez la date qui vous intéresse!" << std::endl;
        try {
            std::cout << "annee[defaut=2016]";
            std::getline(std::cin, buff);
            year = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            year = 2016;
        }
        try {
            std::cout << "mois[defaut=10]";
            std::getline(std::cin, buff);
            month = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            month = 10;
        }
        try {
            std::cout << "jour[defaut=5]";
            std::getline(std::cin, buff);
            day = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            day = 5;
        }
        return Date(year, month, day);
    };

    auto askTime = [] (unsigned int def_hours, unsigned int def_min, unsigned int def_sec) {
        unsigned int hour, minute, second;
        std::string buff;
        std::cout << "Entrez le début de l'horaire !" << std::endl;
        try {
            std::cout << "heure[defaut=" << def_hours << "]";
            std::getline(std::cin, buff);
            hour = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            hour = def_hours;
        }
        try {
            std::cout << "minute[defaut=" << def_min << "]";
            std::getline(std::cin, buff);
            minute = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            minute = def_min;
        }
        try {
            std::cout << "secont[defaut=" << def_sec<< "]";
            std::getline(std::cin, buff);
            second = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            second = def_sec;
        }
        return Heure(hour, minute, second);
    };

    auto proxy = [] (Gestionnaire* gest) {
        double lat, longitude, rayon;
        std::string buff;
        std::cout << "entrez vos coordonnées GPS" << std::endl;
        std::cout << "Latitude : ";
        try {
            std::getline(std::cin, buff);
            lat = StringConverter::fromString<double>(buff);
            std::cout << "Longitude : ";
            std::getline(std::cin, buff);
            longitude = StringConverter::fromString<double>(buff);
            std::cout << "Entrez un rayon (en Km) pour la recherche : ";
            std::getline(std::cin, buff);
            rayon = StringConverter::fromString<double>(buff);
        } catch (std::runtime_error const& e) {
            std::cerr << "Entrez des données valides, s'il vous plait ! :)" << std::endl;
            return;
        }
        if (!Coordonnees::is_valide_coord(lat, longitude)) {
            std::cerr << "Entrez des données valides, s'il vous plait ! :)" << std::endl;
            return;
        }
        auto res = gest->trouver_stations_environnantes(Coordonnees(lat, longitude), rayon);
        for (auto& elem : res) {
           std::cout << "À une distance de " << elem.first << "km" << std::endl;
            std::cout << elem.second->getId() << elem.second->getDescription() << std::endl;
        }
    };
    auto horaires = [&askDate, &askTime] (Gestionnaire* gest) {
        Heure time;
        Date date;
        unsigned int numeroStation;
        std::string numeroLigne;
        std::string buff;
        date = askDate();
        time = askTime(20, 0, 0);
        std::cout << "Entrez le numéro du bus: ";
        std::getline(std::cin, numeroLigne);
        if (!gest->bus_existe(numeroLigne)) {
            std::cerr << "Entrez des données valides, s'il vous plait ! => La ligne n'est pas valide :)" << std::endl;
            return;
        }
        std::cout << "Entrez le numéro de la station: ";
        std::getline(std::cin, buff);
        try {
            numeroStation = StringConverter::fromString<unsigned int>(buff);
        } catch (...) {
            std::cerr << "Entrez des données valides, s'il vous plait ! => La station n'est pas valide :)" << std::endl;
            return;
        }
        if (!gest->station_existe(numeroStation)) {
            std::cerr << "Entrez des données valides, s'il vous plait ! => La station n'est pas valide :)" << std::endl;
            return;
        }
        Ligne realLine = gest->getLigne(numeroLigne);
        Station realStation = gest->getStation(numeroStation);
        std::vector<Heure> heures;
        try {
            heures = gest->trouver_horaire(date, time, numeroLigne, numeroStation,
                                                   realLine.getDestinations().first);
        } catch (...) {
            heures = gest->trouver_horaire(date, time, numeroLigne, numeroStation,
                                                   realLine.getDestinations().second);
        }
        std::cout << Ligne::categorieToString(realLine.getCategorie()) << " " << realLine.getNumero() << " - " << realLine.getDescription() << std::endl;
        std::cout << realStation.getId() << " - " << realStation.getDescription() << std::endl << std::endl;
        for (auto& elem : heures) {
            std::cout << elem << std::endl;
        }
    };

    //std::vector<std::vector<unsigned int> > compos;
    Gestionnaire gest("D:\\work\\AlgorithmeTPs\\AlgoTP1\\RTC");
    horaires(&gest);
    /*if (gest.reseau_est_fortement_connexe(Date(), Heure(19, 15, 0), true))
        std::cout << "connexe" << std::endl;
    else
        std::cout << "pas connexe" << std::endl;
    gest.composantes_fortement_connexes(Date(), Heure(24, 15, 0), compos, true);
    for (auto& elem: compos) {
        for (auto& sub: elem) {
            std::cout << sub << '-';
        }
        std::cout << std::endl;
    }
    gest.getReseau().print(os);*/
    /*auto chemin = gest.plus_court_chemin(Date(2016, 10, 5), Heure(20, 0, 0), Coordonnees(46.778808, -71.270014), Coordonnees(46.760074, -71.319867));
    for (auto& etape : chemin) {
        std::cout << etape << " - ";
        if (gest.station_existe(etape)) std::cout << gest.getStation(etape).getDescription();
        std::cout << std::endl;
    }
    gest.getReseau().print(os);*/
    return 0;
}