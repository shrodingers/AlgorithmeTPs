#include "station.h"

Station::Station(const std::vector<std::string>& ligne_gtfs) :
        m_id(StringConverter::fromString<unsigned int>(ligne_gtfs.at(0))),
        m_nom(ligne_gtfs.at(1)),
        m_description(ligne_gtfs.at(2).substr(1, ligne_gtfs.at(2).size() - 2)),
        m_coords(Coordonnees(stod(ligne_gtfs.at(3)), stod(ligne_gtfs.at(4))))
{
}

std::ostream & operator<<(std::ostream & flux, const Station & p_station)
{
    std::string name = p_station.m_nom.substr(1, p_station.m_nom.size() - 2);
    flux << p_station.getId() << " - " << name << std::endl;
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
    bool is_in_list = false;

    for (auto & v : m_voyages_passants)
    {
        is_in_list = false;
        for (auto & l : lignes_passantes)
            if (v->getLigne()->getId() == l->getId()) {
                is_in_list = true;
                break;
            }
        if (!is_in_list)
            lignes_passantes.push_back(v->getLigne());
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
