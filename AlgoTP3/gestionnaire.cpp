//
// Created by Leon on 31/10/2016.
//

#include "auxiliaires.h"
#include "gestionnaire.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <unordered_set>

Gestionnaire::Gestionnaire(std::string chemin_dossier) {
    //parsing des différents fichiers de données de la RTC
    std::vector< std::vector<std::string> > linesRtc, stationsRtc, tripsRtc, stopsRtc,datesRtc;
    std::unordered_map<std::string, std::vector<Arret*> > VoyageArrets;

    try {
        lireFichier(chemin_dossier + "/routes.txt", linesRtc, ',', true);
        lireFichier(chemin_dossier + "/stops.txt", stationsRtc, ',', true);
        lireFichier(chemin_dossier + "/trips.txt", tripsRtc, ',', true);
        lireFichier(chemin_dossier + "/stop_times.txt", stopsRtc, ',', true);
        lireFichier(chemin_dossier + "/calendar_dates.txt", datesRtc, ',', true);
    } catch (std::logic_error& e) {
        std::cerr << e.what() << std::endl;
    }

    // Construction des containers de Ligne
    std::transform(linesRtc.begin(), linesRtc.end(), std::inserter(m_lignes, std::end(m_lignes)), [this] (std::vector<std::string> const& line) {
        Ligne * ligne = new Ligne(line);
        m_lignes_name[ligne->getNumero()] = ligne;
        return std::make_pair(line[Ligne::routeIdIndex], std::make_pair(ligne, std::vector<Voyage*>()));
    });

    // Construction du container de station indexé par id
    std::transform(stationsRtc.begin(), stationsRtc.end(), std::inserter(m_stations, std::end(m_stations)), [] (std::vector<std::string> const& line) {
        return std::make_pair(StringConverter::fromString<unsigned int>(line[Station::stationIdIndex]), new Station(line));
    });

    // Ajout des voyages selon la date
    for (auto& line : tripsRtc) {
        Voyage* trip = new Voyage(line, m_lignes[line[Voyage::routeIdIndex]].first);
        for (auto& dateLine : datesRtc) {
            Date date(StringConverter::fromString<unsigned int>(dateLine[1].substr(0, 4)),
                      StringConverter::fromString<unsigned int>(dateLine[1].substr(4, 2)),
                      StringConverter::fromString<unsigned int>(dateLine[1].substr(6)));
            if (trip->getServiceId() == dateLine[0]) {
                m_voyages_dates[date].push_back(trip);
            } else if (!m_voyages_dates.count(date)) {
                m_voyages_dates[date];
            }
        }
        m_lignes[line[Voyage::routeIdIndex]].second.push_back(trip);
        m_voyages[trip->getId()] = trip;
    }

    //construction des Arrets
    std::transform(stopsRtc.begin(), stopsRtc.end(), std::back_inserter(m_arrets), [&VoyageArrets] (std::vector<std::string> const& line) {
        Arret* stop = new Arret(line);
        VoyageArrets[line[Arret::tripIdIndex]].push_back(stop);
        return stop;
    });

    // Ajout des voyages aux lignes
    for (auto& elem : m_lignes) {
        elem.second.first->setVoyages(elem.second.second);
    }

    // Ajout des Arrêts aux voyages
    for (auto& trip : m_voyages) {
        std::list<Arret> currentStops;
        auto& VoyageArretsArray = VoyageArrets[trip.second->getId()];
        std::for_each(VoyageArretsArray.begin(), VoyageArretsArray.end(), [this, &trip, &currentStops] (Arret* stop) {
            currentStops.insert(std::lower_bound(currentStops.begin(), currentStops.end(), *stop), *stop);
            if (m_stations.count(stop->getStationId())) {
                m_stations[stop->getStationId()]->addVoyage(trip.second);
            }
        });
        std::vector<Arret> finalvec ({ std::make_move_iterator(std::begin(currentStops)),
                                       std::make_move_iterator(std::end(currentStops)) });
        if (!finalvec.empty()) trip.second->setArrets(finalvec);
    }
}

bool Gestionnaire::date_est_prise_en_charge(const Date &date) {
    return m_voyages_dates.count(date) != 0;
}

bool Gestionnaire::bus_existe(std::string num_ligne) {
    return m_lignes_name.count(num_ligne) != 0;
}

bool Gestionnaire::station_existe(int station_id) {
    return m_stations.count(static_cast<unsigned int>(station_id)) != 0;
}

Ligne Gestionnaire::getLigne(std::string num_ligne) {
    return *m_lignes_name[num_ligne];
}

Station Gestionnaire::getStation(int station_id) {
    return *m_stations[station_id];
}

std::pair<std::string, std::string> Gestionnaire::get_bus_destinations(int station_id, std::string num_ligne) {
    Ligne* line = m_lignes_name[num_ligne];
    Station* station = m_stations[station_id];
    std::vector<Voyage*> passantStation = station->getVoyagesPassants();
    std::vector<Voyage*> voyagesLigne = line->getVoyages();

    // cherche l'existence d'un voyage passant par la station et dépendant de la ligne
    return std::accumulate(passantStation.begin(), passantStation.end(), false, [&voyagesLigne] (bool found, Voyage* trip) {
        return std::accumulate(voyagesLigne.begin(), voyagesLigne.end(), found, [&trip] (bool subFound, Voyage* lineTrip) {
            return subFound || trip == lineTrip;
        });
    }) ? line->getDestinations() : std::make_pair("", "");
}

std::vector<std::pair<double, Station*> > Gestionnaire::trouver_stations_environnantes(Coordonnees coord,
                                                                                      double rayon) {
    // remplis le vecteur si la distance entre les points est inférieiure au rayon désiré
    auto ret = std::accumulate(m_stations.begin(), m_stations.end(), std::vector<std::pair<double, Station*>>(),
                           [&coord, &rayon] (std::vector<std::pair<double, Station*>>& ret, std::pair<const unsigned int, Station*>& station) {
                               auto dist = station.second->getCoords() - coord;
                               if (dist <= rayon) ret.push_back(std::make_pair(dist, station.second));
                               return ret;
                           });
    std::sort(ret.begin(), ret.end(), [] (std::pair<double, Station*>const& elem1, std::pair<double, Station*>const& elem2) {
       return  elem1.first < elem2.first;
    });
    return ret;
}

// Initialise le réseau en utilisant les stations comme sommets (regarder initialiser_reseau pour commentaires internes)
void Gestionnaire::initialiser_reseau_stations(Date date, Heure heure_depart, Heure heure_fin, Coordonnees depart,
                                         Coordonnees dest, double dist_de_marche, double dist_transfert) {
    auto voyages = m_voyages_dates[date];
    std::unordered_map<std::pair<std::string, unsigned int>, unsigned int, hashPair > indexes;

    auto calcTime = [] (Heure hDepart, Heure hArrivee) {
        return static_cast<unsigned int>(hArrivee - hDepart);
    };

    auto timeByFeet = [] (double distance) {
        return static_cast<unsigned int>((distance/vitesse_de_marche) * 3600);
    };

    auto ajouterTrajet = [this] (unsigned int idOrig, unsigned int idDest, unsigned int cout) {
        if (idOrig != idDest && !m_reseau.arcExiste(idOrig, idDest)) {
            m_reseau.ajouterArc(idOrig, idDest, cout);
        } else if (m_reseau.arcExiste(idOrig, idDest) && cout < m_reseau.getCoutArc(idOrig, idDest)) {
            m_reseau.majCoutArc(idOrig, idDest, cout);
        }
    };

    m_arretsInteret = std::accumulate(voyages.begin(), voyages.end(), std::vector<Arret>(), [&heure_depart, &heure_fin, &indexes] (std::vector<Arret>& prev, Voyage* elem) {
        unsigned int count = 0;
        for (auto& stop : elem->getArrets()) {
            if (stop.getHeureArrivee() >= heure_depart && stop.getHeureArrivee() <= heure_fin) {
                prev.push_back(stop);
            }
        }
        return prev;
    });

    std::unordered_set<std::pair<std::string, unsigned int>,  hashPair > resolved;

    std::function<void(unsigned int, Coordonnees, Heure, double, const Arret*)> trouverEnvirons =
            [this, &indexes, &resolved, &dest, &dist_de_marche, &dist_transfert, &calcTime, &ajouterTrajet, &timeByFeet, &trouverEnvirons]
                    (unsigned int origin, Coordonnees coord, Heure dpt_hour, double max_dist, const Arret* currentArret) {
                std::unordered_map<unsigned int, const Arret*> goodStops;
                bool nextFound = false;
                if (currentArret) {

                    if (!resolved.count(std::make_pair(currentArret->getVoyageId(), currentArret->getStationId()))) {
                        resolved.emplace(currentArret->getVoyageId(), currentArret->getStationId());
                    } else {
                        return;
                    }}
                if (!m_reseau.sommetExiste(origin)) m_reseau.ajouterSommet(origin);

                if (coord - dest < dist_de_marche) {
                    ajouterTrajet(origin, num_dest, timeByFeet(coord - dest));
                }
                for (auto const& stop : m_arretsInteret) {
                    auto distance = coord - m_stations[stop.getStationId()]->getCoords();

                    if (!nextFound && currentArret && currentArret->getVoyageId() == stop.getVoyageId() && currentArret->getNumeroSequence() < stop.getNumeroSequence()) {
                        ajouterTrajet(origin, stop.getStationId(), calcTime(dpt_hour, stop.getHeureArrivee()));
                        trouverEnvirons(stop.getStationId(), m_stations[stop.getStationId()]->getCoords(), stop.getHeureArrivee(), dist_transfert, &stop);
                        nextFound = true;

                    } else if (distance < max_dist && stop.getHeureDepart() > dpt_hour.add_secondes(timeByFeet(distance))) {
                        if (goodStops.count(stop.getStationId()) == 0 || goodStops[stop.getStationId()]->getHeureDepart() > stop.getHeureDepart()) {
                            goodStops[stop.getStationId()] = &stop;
                        }
                    }
                }
                for (auto& elem: goodStops) {
                    Station* to = m_stations[elem.second->getStationId()];

                    ajouterTrajet(origin, elem.second->getStationId(), calcTime(dpt_hour, elem.second->getHeureDepart()));
                    trouverEnvirons(elem.second->getStationId(), to->getCoords(), elem.second->getHeureArrivee(), dist_transfert, elem.second);
                }
            };
    std::sort(m_arretsInteret.begin(), m_arretsInteret.end(), [] (Arret const& a1, Arret const& a2) {
        return a1.getNumeroSequence() < a2.getNumeroSequence();
    });
    m_reseau = Reseau();
    m_reseau.ajouterSommet(num_depart);
    m_reseau.ajouterSommet(num_dest);
    trouverEnvirons(num_depart, depart, heure_depart, dist_de_marche, nullptr);
}

void Gestionnaire::initialiser_reseau(Date date, Heure heure_depart, Heure heure_fin, Coordonnees depart,
                                         Coordonnees dest, double dist_de_marche, double dist_transfert) {
    auto voyages = m_voyages_dates[date];
    std::unordered_map<std::pair<std::string, unsigned int>, unsigned int, hashPair > indexes;
    //calcule le temps entre deux arrets
    auto calcTime = [] (Heure hDepart, Heure hArrivee) {
        return static_cast<unsigned int>(hArrivee - hDepart);
    };
    // calcule le temps pour faire une distance à pieds
    auto timeByFeet = [] (double distance) {
        return static_cast<unsigned int>((distance/vitesse_de_marche) * 3600);
    };
    // Ajoute un trajet possiblme (Arc du réseau)
    auto ajouterTrajet = [this] (unsigned int idOrig, unsigned int idDest, unsigned int cout, MoyenDeplacement type = MoyenDeplacement::BUS) {
        if (idOrig != idDest && !m_reseau.arcExiste(idOrig, idDest)) {
            m_reseau.ajouterArc(idOrig, idDest, cout, static_cast<unsigned int>(type));
        }
    };
    // Sélectionne tous les arrets répondant aux critères
    m_arretsInteret = std::accumulate(voyages.begin(), voyages.end(), std::vector<Arret>(), [&heure_depart, &heure_fin, &indexes] (std::vector<Arret>& prev, Voyage* elem) {
        for (auto& stop : elem->getArrets()) {
            if (stop.getHeureArrivee() >= heure_depart && stop.getHeureArrivee() <= heure_fin) {
                prev.push_back(stop);
            }
        }
        return prev;
    });
    // Garde le compte des arrets résolu (un arrêt est une entité unique quand définie par un voyage et une station)
    std::unordered_set<std::pair<std::string, unsigned int>,  hashPair > resolved;
    // Trouve et ajoute tous les arcs environnant un arret donnée, appelée récursivement depuis le point de départ => construit le graphe entier
    std::function<void(unsigned int, Coordonnees, Heure, double, const Arret*)> trouverEnvirons =
            [this, &indexes, &resolved, &dest, &dist_de_marche, &dist_transfert, &calcTime, &ajouterTrajet, &timeByFeet, &trouverEnvirons]
                    (unsigned int origin, Coordonnees coord, Heure dpt_hour, double max_dist, const Arret* currentArret) {
                std::unordered_map<unsigned int, const Arret*> goodStops;
                bool nextFound = false;
                if (currentArret) {
                    // Evite de repasser sur un arrêt déjà résolu
                    if (!resolved.count(std::make_pair(currentArret->getVoyageId(), currentArret->getStationId()))) {
                        resolved.emplace(currentArret->getVoyageId(), currentArret->getStationId());
                    } else {
                        return;
                    }}
                if (!m_reseau.sommetExiste(origin)) m_reseau.ajouterSommet(origin);
                // Teste la distance avec la fin
                if (coord - dest < dist_de_marche) {
                    ajouterTrajet(origin, num_dest, timeByFeet(coord - dest), MoyenDeplacement::PIEDS);
                }
                auto getIndex = [&indexes] (Arret const& arr) {
                    return indexes[std::make_pair(arr.getVoyageId(), arr.getStationId())] + 2;
                };
                for (auto const& stop : m_arretsInteret) {
                    auto distance = coord - m_stations[stop.getStationId()]->getCoords();
                    // Teste si l'arret est sur le meme voyage, et qu'il est situé juste après dans la séquence. Dans ce cas, le trajet entre les deux se fait par BUS
                    if (!nextFound && currentArret && currentArret->getVoyageId() == stop.getVoyageId() && currentArret->getNumeroSequence() < stop.getNumeroSequence()) {
                        ajouterTrajet(origin, getIndex(stop), calcTime(dpt_hour, stop.getHeureArrivee()), MoyenDeplacement::BUS);
                        trouverEnvirons(getIndex(stop), m_stations[stop.getStationId()]->getCoords(), stop.getHeureArrivee(), dist_transfert, &stop);
                        nextFound = true;
                        //Cherche si l'arret est dans le rayon défini par la distance max de transfert,
                        // et si il est possible de le rejoindre avant son départ depuis l'arret en résolution.
                        // Ajoute l'arret joignable le plus rapidement si multiplicité
                    } else if (distance < max_dist && stop.getHeureDepart() > dpt_hour.add_secondes(timeByFeet(distance))) {
                        if (goodStops.count(getIndex(stop)) == 0 || goodStops[getIndex(stop)]->getHeureDepart() > stop.getHeureDepart()) {
                            goodStops[getIndex(stop)] = &stop;
                        }
                    }
                }
                // Pour tout les Arrets satisfaisant trouvés, ajouter le trajet entre l'arr^t courant et celui ci.
                // Résoudre ensuite chacun de ces arrets.
                for (auto& elem: goodStops) {
                    Station* to = m_stations[elem.second->getStationId()];

                    ajouterTrajet(origin, getIndex(*elem.second), calcTime(dpt_hour, elem.second->getHeureDepart()), MoyenDeplacement::PIEDS);
                    trouverEnvirons(getIndex(*elem.second), to->getCoords(), elem.second->getHeureArrivee(), dist_transfert, elem.second);
                }
            };
    //Trie les arrets par numéro de séquence
    std::sort(m_arretsInteret.begin(), m_arretsInteret.end(), [] (Arret const& a1, Arret const& a2) {
        return a1.getNumeroSequence() < a2.getNumeroSequence();
    });
    unsigned int count = 0;
    for (auto& stop : m_arretsInteret) {
        // initialiser les indexes des arrets, un arret n'ayant pas d'ID,
        // c'est cet index qui permet de nommer le sommet du graphe et de retrouver l'arret
        indexes[std::make_pair(stop.getVoyageId(), stop.getStationId())] = count++;
    }
    m_reseau = Reseau();
    // Initialise le réseau avec les sommets de départ et d'arrivée
    m_reseau.ajouterSommet(num_depart);
    m_reseau.ajouterSommet(num_dest);
    // Trouve récursivement tous les arrets joignables depuis le point de départ dans le temps imparti
    trouverEnvirons(num_depart, depart, heure_depart, dist_de_marche, nullptr);
}

std::vector<Heure> Gestionnaire::trouver_horaire(Date date, Heure heure, std::string numero_ligne, int station_id,
                                                 std::string destination) {
    auto voyages = m_voyages_dates[date];
    std::vector<Heure> arretsHoraires;
    // Enève de la liste les voyages ne correspondant à la ligne et à la destination
    voyages.erase(std::remove_if(voyages.begin(), voyages.end(), [&numero_ligne, &destination] (Voyage* trip) {
       return trip->getLigne()->getNumero() != numero_ligne || trip->getDestination() != destination;
    }), voyages.end());
    for (auto& trip : voyages) {
        //sauvegarde les horaires des arrets passant par la station pour les voyages sélectionnés
        Arret& stop = trip->arretDeLaStation(static_cast<unsigned int>(station_id));
        if (stop.getHeureArrivee() >= heure) {
            arretsHoraires.push_back(stop.getHeureArrivee());
        }
    }
    std::sort(arretsHoraires.begin(), arretsHoraires.end(), [] (Heure const& h1, Heure const& h2) {
        return h1 < h2;
    });
    return arretsHoraires;
}


bool Gestionnaire::reseau_est_fortement_connexe(Date date, Heure heure_debut, bool considerer_transfert) {
    initialiser_reseau_stations(date, heure_debut, Heure(32, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
    return m_reseau.estFortementConnexe();
}

void Gestionnaire::composantes_fortement_connexes(Date date, Heure heure_debut,
                                                  std::vector<std::vector<unsigned int> > &composantes,
                                                  bool considerer_transfert) {
    initialiser_reseau_stations(date, heure_debut, Heure(32, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
    m_reseau.getComposantesFortementConnexes(composantes);
}

std::vector< unsigned int > Gestionnaire::plus_court_chemin(Date date, Heure heure_depart, Coordonnees depart,
                                                            Coordonnees destination) {
    std::vector<unsigned int> chemin;

    std::cout << "initialisation du reseau..." << std::endl;
    initialiser_reseau(date, heure_depart, heure_depart.add_secondes(interval_planification_en_secondes), depart, destination, distance_max_initiale, distance_max_transfert);
    std::cout << "Calcul de l'itinéraire" << std::endl;
    try {
        m_reseau.dijkstra(num_depart, num_dest, chemin);
    } catch (std::logic_error const& err) {
        return std::vector<unsigned int>();
    }
    unsigned int tmp = 0;
    // Retrouve les informations des stations selon les arrets composant le réseau
    std::transform(chemin.begin(), chemin.end(), chemin.begin(), [this, &tmp] (unsigned int sommet) {
        if (sommet < 2) {
            return sommet;
      } else {
          return m_arretsInteret[sommet - 2].getStationId();
      }
    });
    return chemin;
}
