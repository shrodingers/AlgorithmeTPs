#include "Enonce/station.h"

Station::Station(const std::vector<std::string>& ligne_gtfs)
{
	m_id = stoi(ligne_gtfs.at(0));
	m_nom = ligne_gtfs.at(1);
	m_description = ligne_gtfs.at(2);
	//m_coords = Coordonnees(stod(ligne_gtfs.at(3)), stod(ligne_gtfs.at(4)));
}

std::ostream & operator<<(std::ostream & flux, const Station & p_station)
{
	flux << p_station.getId() << " : " << p_station.getNom() << " : " << p_station.getDescription() << " :" << p_station.getCoords();
	return (flux);
}

const Coordonnees& Station::getCoords() const
{
	return (m_coords);
}

void Station::setCoords(const Coordonnees& coords)
{
	m_coords = coords;
}

const std::string& Station::getDescription() const
{
	return (m_description);
}

void Station::setDescription(const std::string& description)
{
	m_description = description;
}

std::vector<Ligne*> Station::getLignesPassantes() const
{
	std::vector<Ligne*> lignes_passantes;

	for (std::vector<Voyage *>::const_iterator i = m_voyages_passants.begin(); i != m_voyages_passants.end(); ++i)
	{
		lignes_passantes.push_back((*i)->getLigne());
	}
	return (lignes_passantes);
}

const std::string& Station::getNom() const
{
	return (m_nom);
}

void Station::setNom(const std::string& nom)
{
	m_nom = nom;
}

unsigned int Station::getId() const
{
	return (m_id);
}

void Station::setId(unsigned int stationId)
{
	m_id = stationId;
}

const std::vector<Voyage*> & Station::getVoyagesPassants() const
{
	return (m_voyages_passants);
}

void Station::addVoyage(Voyage* ptr_voyage)
{
	m_voyages_passants.push_back(ptr_voyage);
}

double Station::distance(const Station& p_station) const
{
	return (m_coords - p_station.getCoords());
}
