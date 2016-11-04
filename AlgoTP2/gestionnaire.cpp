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
    std::transform(linesRtc.begin(), linesRtc.end(), std::inserter(m_lignes, std::end(m_lignes)), [] (std::vector<std::string> const& line) {
        return std::make_pair(line[Ligne::routeIdIndex], std::make_pair(new Ligne(line), std::vector<Voyage*>()));
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
    return m_lignes.count(num_ligne) != 0;
}

bool Gestionnaire::station_existe(int station_id) {
    return m_stations.count(static_cast<unsigned int>(station_id)) != 0;
}

Ligne Gestionnaire::getLigne(std::string num_ligne) {
    return *m_lignes[num_ligne].first;
}

Station Gestionnaire::getStation(int station_id) {
    return *m_stations[station_id];
}

std::pair<std::string, std::string> Gestionnaire::get_bus_destinations(int station_id, std::string num_ligne) {
    Ligne* line = m_lignes[num_ligne].first;
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

void Gestionnaire::initialiser_reseauAlt(Date date, Heure heure_depart, Heure heure_fin, Coordonnees depart,
                                         Coordonnees dest, double dist_de_marche, double dist_transfert) {
    auto voyages = m_voyages_dates[date];
    std::unordered_map<std::pair<std::string, unsigned int>, unsigned int, hashPair > indexes;
    auto calcTime = [] (Heure hDepart, Heure hArrivee) {
        if (hArrivee < hDepart)
            std::cout << hArrivee << " - " << hDepart << std::endl;
        return static_cast<unsigned int>(hArrivee - hDepart);
    };
    auto timeByFeet = [] (double distance) {
        if (distance > 1)
            std::cout << distance << std::endl;
        //std::cout << "feets" << static_cast<unsigned int>(std::round((vitesse_de_marche / distance) * 3600)) << std::endl;
        return static_cast<unsigned int>((distance/vitesse_de_marche) * 3600);
    };
    auto ajouterTrajet = [this] (unsigned int idOrig, unsigned int idDest, unsigned int cout) {
        /*if (cout == 0)
            std::cout << "cout nll";*/
        if (idOrig != idDest && !m_reseau.arcExiste(idOrig, idDest)) {
            m_reseau.ajouterArc(idOrig, idDest, cout);
        } else if (m_reseau.arcExiste(idOrig, idDest) && cout < m_reseau.getCoutArc(idOrig, idDest)) {
            m_reseau.majCoutArc(idOrig, idDest, cout);
        }
    };
    std::cout << "arrets init" << std::endl;
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
                auto getIndex = [&indexes] (Arret const& arr) {
                    return indexes[std::make_pair(arr.getVoyageId(), arr.getStationId())] + 2;
                };
                for (auto const& stop : m_arretsInteret) {
                    auto distance = coord - m_stations[stop.getStationId()]->getCoords();
                    if (!nextFound && currentArret && currentArret->getVoyageId() == stop.getVoyageId() && currentArret->getNumeroSequence() < stop.getNumeroSequence()) {
                        ajouterTrajet(origin, getIndex(stop), calcTime(dpt_hour, stop.getHeureArrivee()));
                        trouverEnvirons(getIndex(stop), m_stations[stop.getStationId()]->getCoords(), stop.getHeureArrivee(), dist_transfert, &stop);
                        nextFound = true;
                    } else if (distance < max_dist && stop.getHeureDepart() > dpt_hour.add_secondes(timeByFeet(distance))) {
                        if (goodStops.count(stop.getStationId()) == 0 || goodStops[stop.getStationId()]->getHeureDepart() > stop.getHeureDepart()) {
                            goodStops[stop.getStationId()] = &stop;
                        }
                    }
                }
                for (auto& elem: goodStops) {
                    Station* to = m_stations[elem.second->getStationId()];

                    ajouterTrajet(origin, getIndex(*elem.second), calcTime(dpt_hour, elem.second->getHeureDepart()));
                    trouverEnvirons(getIndex(*elem.second), to->getCoords(), elem.second->getHeureArrivee(), dist_transfert, elem.second);
                }
            };

    std::cout << "arrets sort" << std::endl;
    std::sort(m_arretsInteret.begin(), m_arretsInteret.end(), [] (Arret const& a1, Arret const& a2) {
        return a1.getNumeroSequence() < a2.getNumeroSequence();
    });
    unsigned int count = 0;
    for (auto& stop : m_arretsInteret) {
        indexes[std::make_pair(stop.getVoyageId(), stop.getStationId())] = count++;
    }
    std::cout << "Construction" << std::endl;
    m_reseau = Reseau();
    m_reseau.ajouterSommet(num_depart);
    m_reseau.ajouterSommet(num_dest);
    trouverEnvirons(num_depart, depart, heure_depart, dist_de_marche, nullptr);
    //trouverEnvirons(num_dest, dest, heure_depart, dist_de_marche, nullptr);
}

void Gestionnaire::initialiser_reseau(Date date, Heure heure_depart, Heure heure_fin, Coordonnees depart,
                                      Coordonnees dest, double dist_de_marche, double dist_transfert) {
    auto voyages = m_voyages_dates[date];
    std::vector<Arret*> m_arretsInteret;
    auto ajouterTrajet = [this] (unsigned int idOrig, unsigned int idDest, unsigned int cout) {
        if (cout == 0)
            std::cout << "cout nll";
        if (idOrig != idDest && !m_reseau.arcExiste(idOrig, idDest)) {
            m_reseau.ajouterArc(idOrig, idDest, cout);
        } else if (m_reseau.arcExiste(idOrig, idDest) && cout < m_reseau.getCoutArc(idOrig, idDest)) {
            m_reseau.majCoutArc(idOrig, idDest, cout);
        }
    };
    auto calcTime = [] (Heure hDepart, Heure hArrivee) {
        if (hArrivee < hDepart)
            std::cout << hArrivee << " - " << hDepart << std::endl;
        return static_cast<unsigned int>(hArrivee - hDepart);
    };
    auto timeByFeet = [] (double distance) {
        if (distance > 1)
            std::cout << distance << std::endl;
        //std::cout << "feets" << static_cast<unsigned int>(std::round((vitesse_de_marche / distance) * 3600)) << std::endl;
        return static_cast<unsigned int>((distance/vitesse_de_marche) * 3600);
    };
    auto addArretsEnvironants = [this, &calcTime, &timeByFeet, &ajouterTrajet, &dist_transfert] (unsigned int origin, Coordonnees coord, Heure dpt_hour,
                                                                                                 std::vector<std::pair<double , Station*> > environs, bool isDest) {
        for (auto i = environs.begin(); i != environs.end(); ++i) {
            std::vector<Arret> arretsPassant;
            std::vector<Voyage*> voyagesPassants = i->second->getVoyagesPassants();

            std::transform(voyagesPassants.begin(), voyagesPassants.end(),
                           std::back_inserter(arretsPassant), [&i](Voyage *voyagePassant) {
                        return voyagePassant->arretDeLaStation(i->second->getId());
                    });
            std::sort(arretsPassant.begin(), arretsPassant.end(), [](Arret const &a1, Arret const &a2) {
                return a1.getHeureDepart() < a2.getHeureDepart();
            });
            for (auto &arretPassant : arretsPassant) {
                if (dpt_hour < arretPassant.getHeureDepart() &&
                    calcTime(dpt_hour, arretPassant.getHeureDepart()) > timeByFeet(i->first)) {
                    if (!m_reseau.sommetExiste(i->second->getId())) m_reseau.ajouterSommet(i->second->getId());
                    if (isDest)
                        ajouterTrajet(i->second->getId(), origin, timeByFeet(i->first));
                    else
                        ajouterTrajet(origin, i->second->getId(), calcTime(dpt_hour, arretPassant.getHeureDepart()));
                    break;
                }
            }
        }
    };

    /*std::copy_if(m_arrets.begin(), m_arrets.end(), std::back_inserter(arretsInteret), [&heure_depart, &heure_fin] (Arret*elem) {
        return elem->getHeureArrivee() >= heure_depart && elem->getHeureDepart() <= heure_fin;
    });

    std::sort(arretsInteret.begin(), arretsInteret.end())*/

    m_reseau = Reseau();
    m_reseau.ajouterSommet(num_depart);
    m_reseau.ajouterSommet(num_dest);

    addArretsEnvironants(num_depart, depart, heure_depart,  trouver_stations_environnantes(depart, dist_de_marche), false);
    addArretsEnvironants(num_dest, dest, heure_depart,  trouver_stations_environnantes(dest, dist_de_marche), true);
    /*std::vector<std::pair<double , Station*> > environs = trouver_stations_environnantes(m_stations[stop.getStationId()]->getCoords(), dist_transfert);
    if (depart - m_stations[stop.getStationId()]->getCoords() <= dist_de_marche && calcTime(heure_depart, stop.getHeureDepart()) > timeByFeet(depart - m_stations[stop.getStationId()]->getCoords()))
        ajouterTrajet(num_depart, stop.getStationId(),
                      calcTime(heure_depart, stop.getHeureDepart()));*/
    int count = 0;
    for (auto &elem: voyages) {
        auto stops = elem->getArrets();
        Arret *precedentArret = nullptr;
        for (auto &stop : stops) {
            if (stop.getHeureArrivee() >= heure_depart && stop.getHeureDepart() <= heure_fin) {
                if (precedentArret && stop.getStationId() == precedentArret->getStationId()) {
                    std::cout << "same" << std::endl;
                    continue;
                }
                if (!m_reseau.sommetExiste(stop.getStationId())) m_reseau.ajouterSommet(stop.getStationId());
                if (precedentArret) {
                    if (precedentArret->getHeureDepart() > stop.getHeureArrivee())
                        std::cout << precedentArret->getHeureDepart() << " - " << stop.getHeureArrivee() << std::endl;
                    ajouterTrajet(precedentArret->getStationId(), stop.getStationId(),
                                  calcTime(precedentArret->getHeureArrivee(), stop.getHeureArrivee()));
                }
                /*if (dest - m_stations[stop.getStationId()]->getCoords() <= dist_de_marche)
                    ajouterTrajet(stop.getStationId(), num_dest,
                                  timeByFeet(m_stations[stop.getStationId()]->getCoords() - dest));*/
                addArretsEnvironants(stop.getStationId(), m_stations[stop.getStationId()]->getCoords(), stop.getHeureArrivee(),
                                     trouver_stations_environnantes(m_stations[stop.getStationId()]->getCoords(), dist_transfert), false);
                /*std::vector<std::pair<double , Station*> > environs = trouver_stations_environnantes(m_stations[stop.getStationId()]->getCoords(), dist_transfert);
                for (auto i = environs.begin(); i != environs.end(); ++i) {
                    std::vector<Arret> arretsPassant;
                    std::transform(i->second->getVoyagesPassants().begin(), i->second->getVoyagesPassants().end(),
                                   std::back_inserter(arretsPassant), [&i](Voyage *voyagePassant) {
                                return voyagePassant->arretDeLaStation(i->second->getId());
                            });
                    std::sort(arretsPassant.begin(), arretsPassant.end(), [](Arret const &a1, Arret const &a2) {
                        return a1.getHeureDepart() < a2.getHeureDepart();
                    });
                    for (auto &arretPassant : arretsPassant) {
                        if (stop.getHeureArrivee() < arretPassant.getHeureDepart() &&
                            calcTime(stop.getHeureArrivee(), arretPassant.getHeureDepart()) > timeByFeet(i->first)) {
                            if (!m_reseau.sommetExiste(i->second->getId())) m_reseau.ajouterSommet(i->second->getId());
                            if (stop.getStationId() != i->second->getId() && !m_reseau.arcExiste(stop.getStationId(), i->second->getId())) {
                                m_reseau.ajouterArc(stop.getStationId(), i->second->getId(), calcTime(stop.getHeureArrivee(), arretPassant.getHeureDepart()));
                            }
                            ajouterTrajet(stop.getStationId(), i->second->getId(),
                                          calcTime(stop.getHeureArrivee(), arretPassant.getHeureDepart()));
                            break;
                        }
                    }
                }*/
                precedentArret = &stop;
            }
        }
    }
}

std::vector<Heure> Gestionnaire::trouver_horaire(Date date, Heure heure, std::string numero_ligne, int station_id,
                                                 std::string destination) {
    auto voyages = m_voyages_dates[date];
    std::vector<Heure> arretsHoraires;
    std::remove_if(voyages.begin(), voyages.end(), [&numero_ligne, &destination] (Voyage* trip) {
       return trip->getLigne()->getNumero() == numero_ligne && trip->getDestination() == destination;
    });
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
    initialiser_reseau(date, heure_debut, Heure(25, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
    return m_reseau.estFortementConnexe();
}

void Gestionnaire::composantes_fortement_connexes(Date date, Heure heure_debut,
                                                  std::vector<std::vector<unsigned int> > &composantes,
                                                  bool considerer_transfert) {
    initialiser_reseau(date, heure_debut, Heure(25, 0, 0), Coordonnees(0, 0), Coordonnees(0, 0), 0, considerer_transfert ? distance_max_transfert : 0);
    m_reseau.getComposantesFortementConnexes(composantes);
}

std::vector< unsigned int > Gestionnaire::plus_court_chemin(Date date, Heure heure_depart, Coordonnees depart,
                                                            Coordonnees destination) {
    std::vector<unsigned int> chemin;
    std::cout << "initialisation du reseau..." << std::endl;
    initialiser_reseauAlt(date, heure_depart, heure_depart.add_secondes(interval_planification_en_secondes), depart, destination, distance_max_initiale, distance_max_transfert);
    std::cout << "Calcul de l'itinéraire" << std::endl;
    try {
        m_reseau.dijkstra(num_depart, num_dest, chemin);
    } catch (std::logic_error const& err) {
        return std::vector<unsigned int>();
    }
    std::transform(chemin.begin(), chemin.end(), chemin.begin(), [this] (unsigned int sommet) {
      if (sommet < 2) {
          return sommet;
      } else {
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