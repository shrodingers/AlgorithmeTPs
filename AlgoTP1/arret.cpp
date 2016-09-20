//
// Created by Leon on 18/09/2016.
//

#include "Enonce/arret.h"

Arret::Arret(const std::vector<std::string> &ligne_gtfs)
        :   m_voyage_id(ligne_gtfs[tripIdIndex]),
            m_station_id(StringConverter::fromString<unsigned int>(ligne_gtfs[stopIdIndex])),
            m_numero_sequence(StringConverter::fromString<unsigned int>(ligne_gtfs[stopSequenceIndex])),
            m_heure_depart(0, 0, 0),
            m_heure_arrivee(0, 0, 0)
{
    std::vector<std::string> hourComp = split(ligne_gtfs[departureTimeIndex], ':');
    m_heure_depart = Heure(StringConverter::fromString<unsigned int>(hourComp[hourIndex]),
                           StringConverter::fromString<unsigned int>(hourComp[minuteIndex]),
                           StringConverter::fromString<unsigned int>(hourComp[secondIndex]));
    hourComp = split(ligne_gtfs[arrivalTimeIndex], ':');
    m_heure_arrivee = Heure(StringConverter::fromString<unsigned int>(hourComp[hourIndex]),
                            StringConverter::fromString<unsigned int>(hourComp[minuteIndex]),
                            StringConverter::fromString<unsigned int>(hourComp[secondIndex]));
}

const Heure& Arret::getHeureArrivee() const {
    return m_heure_arrivee;
}

const Heure& Arret::getHeureDepart() const {
    return m_heure_depart;
}

unsigned int Arret::getNumeroSequence() const {
    return m_numero_sequence;
}

unsigned int Arret::getStationId() const {
    return m_station_id;
}

std::string Arret::getVoyageId() const {
    return m_voyage_id;
}

void Arret::setHeureArrivee(const Heure &p_heureArrivee) {
    m_heure_arrivee = p_heureArrivee;
}

void Arret::setHeureDepart(const Heure &p_heureDepart) {
    m_heure_depart = p_heureDepart;
}

void Arret::setNumeroSequence(unsigned int p_numeroSequence) {
    m_numero_sequence = p_numeroSequence;
}

void Arret::setStationId(unsigned int stationId) {
    m_station_id = stationId;
}

void Arret::setVoyageId(const std::string &voyageId) {
    m_voyage_id = voyageId;
}

bool Arret::operator>(const Arret &p_other) const {
    if (m_voyage_id == p_other.m_voyage_id) {
        return m_numero_sequence > p_other.m_numero_sequence;
    }
    return false;
}

bool Arret::operator<(const Arret &p_other) const {
    if (m_voyage_id == p_other.m_voyage_id) {
        return m_numero_sequence < p_other.m_numero_sequence;
    }
    return false;
}

std::ostream & operator<<(std::ostream & flux, const Arret & p_arret) {
    flux << p_arret.m_voyage_id << ',' << p_arret.m_heure_arrivee
         << ',' << p_arret.m_heure_depart << ',' << p_arret.m_station_id
         << ',' << p_arret.m_numero_sequence << ",0,0" << std::endl;
    return flux;
}