//
// Created by Leon on 21/09/2016.
//

#include <algorithm>
#include "voyage.h"

Voyage::Voyage(const std::vector<std::string> &ligne_gtfs, Ligne *p_ligne)
    try : m_id(ligne_gtfs[tripIdIndex]),
          m_service_id(ligne_gtfs[serviceIdIndex]),
          m_destination(ligne_gtfs[tripHeadsignIndex]),
          m_ligne(p_ligne)
{
    ;
} catch (std::length_error const& err) {
    std::cerr << "Error : " << err.what();
}

std::string Voyage::getId() const {
    return m_id;
}

const std::string& Voyage::getDestination() const {
    return m_destination;
}

Ligne* Voyage::getLigne() const {
    return m_ligne;
}

std::vector<Arret> Voyage::getArrets() const {
    return m_arrets;
}

std::string Voyage::getServiceId() const {
    return m_service_id;
}

Heure Voyage::getHeureDepart() const {
    if (m_arrets.empty()) return Heure(0,0,0);
    return m_arrets.front().getHeureArrivee();
}

Heure Voyage::getHeureFin() const {
    if (m_arrets.empty()) return Heure(0,0,0);
    return m_arrets.back().getHeureDepart();
}

Arret& Voyage::arretDeLaStation(unsigned int p_num_station) {
    for (auto& stop : m_arrets) {
        if (stop.getStationId() == p_num_station) {
            return stop;
        }
    }
    throw std::runtime_error("stop not found");
}

void Voyage::setDestination(const std::string &p_destination) {
    m_destination = p_destination;
}

void Voyage::setArrets(std::vector<Arret>& resultat) {
    Heure previous;
    std::vector<Arret> tmp = resultat;
    m_arrets = std::vector<Arret>();

    std::sort(tmp.begin(), tmp.end(), [] (Arret a1, Arret a2) { return a1.getNumeroSequence() < a2.getNumeroSequence(); });
    previous = tmp.front().getHeureDepart();
    std::transform(tmp.begin(), tmp.end(), std::back_inserter(m_arrets), [&previous] (Arret const& stop) {
        if (stop.getHeureArrivee() == previous) {
            Arret newStop = stop;
            previous = stop.getHeureDepart();
            newStop.setHeureArrivee(stop.getHeureArrivee().add_secondes(30));
            newStop.setHeureDepart(stop.getHeureDepart().add_secondes(30));
            return newStop;
        }
        previous = stop.getHeureDepart();
        return stop;
    });
    /*for (auto it = m_arrets.begin() + 1; it != m_arrets.end(); ++it) {
        if ((*it).getHeureDepart() == previous) {
            (*it).setHeureArrivee((*it).getHeureArrivee().add_secondes(30));
            (*it).setHeureDepart((*it).getHeureDepart().add_secondes(30));
        }
        previous = (*it).getHeureDepart();
    }*/
}

void Voyage::setId(std::string p_id) {
    m_id = p_id;
}

void Voyage::setLigne(Ligne *p_ligne) {
    m_ligne = p_ligne;
}

void Voyage::setServiceId(std::string p_service_id) {
    m_service_id = p_service_id;
}

bool Voyage::operator>(const Voyage &p_other) const {
    return getHeureDepart() > p_other.getHeureDepart();
}

bool Voyage::operator<(const Voyage &p_other) const {
    return getHeureDepart() < p_other.getHeureDepart();
}

std::ostream& operator<<(std::ostream &flux, const Voyage &p_voyage) {
    std::string dest = p_voyage.m_destination.substr(1, p_voyage.m_destination.size() - 2);
    std::string num = p_voyage.m_ligne->getNumero().substr(1, p_voyage.m_ligne->getNumero().size() - 2);

    flux << num << ": Vers " << dest << std::endl;
    for (auto& stop : p_voyage.m_arrets) {
        flux << stop;
    }
    return flux;
}