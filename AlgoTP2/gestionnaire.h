//
// Created by prtos on 29/05/16.
//

#ifndef RTC_GESTIONNAIRE_H
#define RTC_GESTIONNAIRE_H

#include <vector>
#include <string>
#include "station.h"
#include "voyage.h"
#include "reseau.h"
#include "auxiliaires.h"

const double vitesse_de_marche = 5.0; /*! vitesse moyenne de marche d'un humain selon wikipedia */

const int interval_planification_en_secondes = 60 * 60 * 3; // l'interval de temps en dedans duquel on voir s'il existe un itinéraire

const double distance_max_initiale = 0.5; // Cette distance est relativement longue mais prend juste 6 min à parcourir.

const double distance_max_transfert = 0.2; // Cette distance est relativement courte et prend juste 2.5 min à parcourir.

const unsigned int num_depart = 0; //numéro donnée pour la station phantome de départ

const unsigned int num_dest = 1; //numéro donnée pour la station phantome de destination

enum class MoyenDeplacement {BUS=0, PIEDS};


/*!
 * \class Gestionnaire
 * \brief classe intégrante les données avec les autres classes pour rendre concret le
 */
class Gestionnaire  {

public:

	Gestionnaire(std::string chemin_dossier);

	bool date_est_prise_en_charge(const Date& date);

	bool bus_existe(std::string num_ligne);

	bool station_existe(int station_id);

	Ligne getLigne(std::string num_ligne);

	Station getStation(int station_id);

	std::pair<std::string, std::string> get_bus_destinations(int station_id, std::string num_ligne);

	std::vector<std::pair<double, Station*>> trouver_stations_environnantes(Coordonnees coord, double rayon);

	std::vector<Heure> trouver_horaire(Date date, Heure heure, std::string numero_ligne, int station_id, std::string destination);

	bool reseau_est_fortement_connexe(Date date, Heure heure_debut, bool considerer_transfert=true);

	void composantes_fortement_connexes(Date date, Heure heure_debut,
			std::vector< std::vector<unsigned int> >& composantes, bool considerer_transfert=true);

	std::vector< unsigned int > plus_court_chemin(Date date, Heure heure_depart, Coordonnees depart, Coordonnees destination);


private:
	Reseau m_reseau;

	void initialiser_reseau(Date date, Heure heure_depart, Heure heure_fin, Coordonnees depart, Coordonnees dest,
			double dist_de_marche=distance_max_initiale, double dist_transfert=distance_max_transfert);

	/** À compléter */



};

#endif //RTC_GESTIONNAIRE_H
