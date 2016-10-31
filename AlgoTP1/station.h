//
// Created by prtos on 29/04/16.
//

#ifndef RTC_STATION_H
#define RTC_STATION_H

#include <string>
#include <vector>
#include <iostream>
#include "coordonnees.h"
#include "ligne.h"
#include "voyage.h"

class Voyage;
class Ligne;

/*!
 * \class Station
 * \brief Classe représentant une station. Une station est un emplacement physique où un bus effectue des arrêts.
 * \note Sa déclaration nécessite la déclaration partielle des classe Ligne et Voyage, en raison de références circulaires avec ces classes.
 */
class Station {

private:
    unsigned int m_id;
    std::string m_nom;
    std::string m_description;
    Coordonnees m_coords;
    std::vector<Voyage* > m_voyages_passants;

public:
    Station(const std::vector<std::string>& ligne_gtfs);
    friend std::ostream& operator<<(std::ostream& flux, const Station& p_station);
	const Coordonnees& getCoords() const;
	void setCoords(const Coordonnees& coords);
	const std::string& getDescription() const;
	void setDescription(const std::string& description);
	std::vector<Ligne*> getLignesPassantes() const;
	const std::string& getNom() const;
	void setNom(const std::string& nom);
	unsigned int getId() const;
	void setId(unsigned int stationId);
	const std::vector<Voyage*> & getVoyagesPassants() const;
	void addVoyage(Voyage* ptr_voyage);
	double distance(const Station& p_station) const;
};


#endif //RTC_STATION_H
