/*!
 * \file arret.h
 * \brief Fichier contenant la classe arret
 * \author Prudencio Tossou
 * \version 0.1
 */

#ifndef RTC_ARRET_H
#define RTC_ARRET_H

#include "auxiliaires.h"


/*!
* \class Arret
* \brief Classe représentant un arret
*
*  Un arret est une composante d'un voyage, c'est une opération spatio-temporelle
*  (ex: la ligne 800 effectue un arrêt à la station du desjardin à 11h32).
*  Il est important de ne confondre la station et l'arret.
*
*/
class Arret {

public:
	Arret(const std::vector<std::string>& ligne_gtfs);
	const Heure& getHeureArrivee() const;
	void setHeureArrivee(const Heure& p_heureArrivee);
	const Heure& getHeureDepart() const;
	void setHeureDepart(const Heure& p_heureDepart);
	unsigned int getNumeroSequence() const;
	void setNumeroSequence(unsigned int p_numeroSequence);
	unsigned int getStationId() const;
	void setStationId(unsigned int stationId);
	std::string getVoyageId() const;
	void setVoyageId(const std::string& voyageId);

	bool operator< (const Arret & p_other) const;
	bool operator> (const Arret & p_other) const;
	friend std::ostream & operator<<(std::ostream & flux, const Arret & p_arret);


private:
	unsigned int m_station_id;
	Heure m_heure_arrivee;
	Heure m_heure_depart;
	unsigned int m_numero_sequence;
	std::string m_voyage_id;
};


#endif //RTC_ARRET_H
