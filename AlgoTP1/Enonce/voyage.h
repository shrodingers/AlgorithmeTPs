//
// Created by prtos on 29/04/16.
//

#ifndef RTC_VOYAGE_H
#define RTC_VOYAGE_H

#include <string>
#include "arret.h"
#include "ligne.h"
#include "station.h"
#include "auxiliaires.h"

class Ligne;
class Station;

/*!
 * \class Voyage
 * \brief Classe permettant de décrire un voyage
 * \note Sa déclaration nécessite la déclaration partielle des classe Ligne et Station, en raison de références circulaires avec ces classes.
 */
class Voyage {

public:
	Voyage(const std::vector<std::string>& ligne_gtfs, Ligne* p_ligne);
	Arret & arretDeLaStation(unsigned int p_num_station);
	std::vector<Arret> getArrets() const;
	const std::string& getDestination() const;
	void setDestination(const std::string& p_destination);
	std::string getId() const;
	void setId(std::string p_id);
	Ligne* getLigne() const;
	void setLigne(Ligne* p_ligne);
	std::string getServiceId() const;
	void setServiceId(std::string p_service_id);
	Heure getHeureDepart() const;
	Heure getHeureFin() const;
	void setArrets(std::vector<Arret>& resultat);
	bool operator< (const Voyage & p_other) const;
	bool operator> (const Voyage & p_other) const;
	friend std::ostream & operator<<(std::ostream & flux, const Voyage & p_voyage);

private:
	std::string m_id;
	Ligne* m_ligne;
	std::string m_service_id;
	std::string m_destination;
	std::vector<Arret> m_arrets;
};


#endif //RTC_VOYAGE_H
