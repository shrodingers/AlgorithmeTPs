//
// Created by Leon on 31/10/2016.
//

#include "auxiliaires.h"
#include "gestionnaire.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>
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
    std::transform(linesRtc.begin(), linesRtc.end(), std::inserter(m_lignes, std::end(m_lignes)), [this] (std::vector<std::string> const& line) {
        Ligne * ligne = new Ligne(line);
        m_lignes_name[ligne->getNumero()] = ligne;
        return std::make_pair(line[Ligne::routeIdIndex], std::make_pair(ligne, std::vector<Voyage*>()));
    });
    std::transform(stationsRtc.begin(), stationsRtc.end(), std::inserter(m_stations, std::end(m_stations)), [] (std::vector<std::string> const& line) {
        return std::make_pair(StringConverter::fromString<unsigned int>(line[Station::stationIdIndex]), new Station(line));
    });
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

    std::transform(stopsRtc.begin(), stopsRtc.end(), std::back_inserter(m_arrets), [&VoyageArrets] (std::vector<std::string> const& line) {
        Arret* stop = new Arret(line);
        VoyageArrets[line[Arret::tripIdIndex]].push_back(stop);
        return stop;
    });

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

    return std::accumulate(passantStation.begin(), passantStation.end(), false, [&voyagesLigne] (bool found, Voyage* trip) {
        return std::accumulate(voyagesLigne.begin(), voyagesLigne.end(), found, [&trip] (bool found, Voyage* lineTrip) {
            return trip == lineTrip;
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
    auto calcTime = [] (Heure hDepart, Heure hArrivee) {
        return static_cast<unsigned int>(hArrivee - hDepart);
    };
    auto timeByFeet = [] (double distance) {
        return static_cast<unsigned int>((distance/vitesse_de_marche) * 3600);
    };
    auto ajouterTrajet = [this] (unsigned int idOrig, unsigned int idDest, unsigned int cout, MoyenDeplacement type = MoyenDeplacement::BUS) {
        if (idOrig != idDest && !m_reseau.arcExiste(idOrig, idDest)) {
            m_reseau.ajouterArc(idOrig, idDest, cout, static_cast<unsigned int>(type));
        }
    };
    m_arretsInteret = std::accumulate(voyages.begin(), voyages.end(), std::vector<Arret>(), [&heure_depart, &heure_fin, &indexes] (std::vector<Arret>& prev, Voyage* elem) {
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
                    ajouterTrajet(origin, num_dest, timeByFeet(coord - dest), MoyenDeplacement::PIEDS);
                }
                auto getIndex = [&indexes] (Arret const& arr) {
                    return indexes[std::make_pair(arr.getVoyageId(), arr.getStationId())] + 2;
                };
                for (auto const& stop : m_arretsInteret) {
                    auto distance = coord - m_stations[stop.getStationId()]->getCoords();
                    if (!nextFound && currentArret && currentArret->getVoyageId() == stop.getVoyageId() && currentArret->getNumeroSequence() < stop.getNumeroSequence()) {
                        ajouterTrajet(origin, getIndex(stop), calcTime(dpt_hour, stop.getHeureArrivee()), MoyenDeplacement::BUS);
                        trouverEnvirons(getIndex(stop), m_stations[stop.getStationId()]->getCoords(), stop.getHeureArrivee(), dist_transfert, &stop);
                        nextFound = true;
                    } else if (distance < max_dist && stop.getHeureDepart() > dpt_hour.add_secondes(timeByFeet(distance))) {
                        if (goodStops.count(getIndex(stop)) == 0 || goodStops[getIndex(stop)]->getHeureDepart() > stop.getHeureDepart()) {
                            goodStops[getIndex(stop)] = &stop;
                        }
                    }
                }
                for (auto& elem: goodStops) {
                    Station* to = m_stations[elem.second->getStationId()];

                    ajouterTrajet(origin, getIndex(*elem.second), calcTime(dpt_hour, elem.second->getHeureDepart()), MoyenDeplacement::PIEDS);
                    trouverEnvirons(getIndex(*elem.second), to->getCoords(), elem.second->getHeureArrivee(), dist_transfert, elem.second);
                }
            };

    std::sort(m_arretsInteret.begin(), m_arretsInteret.end(), [] (Arret const& a1, Arret const& a2) {
        return a1.getNumeroSequence() < a2.getNumeroSequence();
    });
    unsigned int count = 0;
    for (auto& stop : m_arretsInteret) {
        indexes[std::make_pair(stop.getVoyageId(), stop.getStationId())] = count++;
    }
    m_reseau = Reseau();
    m_reseau.ajouterSommet(num_depart);
    m_reseau.ajouterSommet(num_dest);
    trouverEnvirons(num_depart, depart, heure_depart, dist_de_marche, nullptr);
}

std::vector<Heure> Gestionnaire::trouver_horaire(Date date, Heure heure, std::string numero_ligne, int station_id,
                                                 std::string destination) {
    auto voyages = m_voyages_dates[date];
    std::vector<Heure> arretsHoraires;
    voyages.erase(std::remove_if(voyages.begin(), voyages.end(), [&numero_ligne, &destination] (Voyage* trip) {
       return trip->getLigne()->getNumero() != numero_ligne || trip->getDestination() != destination;
    }), voyages.end());
    for (auto& trip : voyages) {
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
    initialiser_reseau_stations(date, heure_debut, Heure(25, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
    return m_reseau.estFortementConnexe();
}

void Gestionnaire::composantes_fortement_connexes(Date date, Heure heure_debut,
                                                  std::vector<std::vector<unsigned int> > &composantes,
                                                  bool considerer_transfert) {
    initialiser_reseau_stations(date, heure_debut, Heure(25, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
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
    std::transform(chemin.begin(), chemin.end(), chemin.begin(), [this, &tmp] (unsigned int sommet) {
      /*if (tmp) {
          std::cout << "cout : " << m_reseau.getCoutArc(tmp, sommet) << "type : " << m_reseau.getTypeArc(tmp, sommet) << std::endl;
      }*/
        if (sommet < 2) {
            //tmp = sommet;
            return sommet;
      } else {
          /*std::cout << "index : " << sommet << " station : " << m_arretsInteret[sommet - 2].getStationId() << "ligne : " << m_voyages[m_arretsInteret[sommet - 2].getVoyageId()]->getLigne()->getNumero() << std::endl;
          tmp = sommet;*/
          return m_arretsInteret[sommet - 2].getStationId();
      }
    });
    return chemin;
}

void Gestionnaire::init() {
    Date today;
    Heure start(20, 47, 52);
    Heure end(21, 42, 10);
    initialiser_reseau(today, start, end, Coordonnees(31.77583, 149.63495), Coordonnees(46.778808, -71.270014), 0, 0);
    std::ofstream os;
    os.open("reseau.dot", std::ofstream::out | std::ofstream::trunc);
    m_reseau.print(os);
}

Reseau const& Gestionnaire::getReseau() {
    return m_reseau;
}