//
// Created by Leon on 18/09/2016.
//

#include <string>
#include <stdexcept>
#include <limits>
#include <tuple>
#include <chrono>
#include "Enonce/auxiliaires.h"

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> ret;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        ret.push_back(item);
    }
    return ret;
}

static inline void trimFirst(std::ifstream& is) {

}

void lireFichier(std::string nomFichier, std::vector<std::vector<std::string>>& resultats, char delimiteur, bool rm_entete)
{
    std::ifstream is;
    //std::vector< std::vector< std::string > > result;
    std::string buffer;

    try {
        is.open(nomFichier);
    } catch (...) {
        throw new std::logic_error("Unable to open file");
    }

    /*
     * trim first line
     */
    if (rm_entete) is.ignore(std::numeric_limits<std::streamsize>::max(), is.widen('\n'));

    while (std::getline(is, buffer)) {
        resultats.push_back(split(buffer, delimiteur));
    }
}

static inline tm& getLocalTime() {
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    return *localtime(&tt);
}

Date::Date()
{
    tm local_tm = getLocalTime();
    m_an = reinterpret_cast<unsigned int>(local_tm.tm_year);
    m_mois = reinterpret_cast<unsigned int>(local_tm.tm_mon);
    m_jour = reinterpret_cast<unsigned int>(local_tm.tm_mday);
}

Date::Date(unsigned int an, unsigned int mois, unsigned int jour)
        : m_an(an), m_mois(mois), m_jour(jour)
{
    ;
}

bool Date::operator==(const Date &other) const {
    return !(*this < other) && !(*this > other);
}

bool Date::operator<(const Date &other) const {
    return std::tie(m_an, m_mois, m_jour) < std::tie(other.m_an, other.m_mois, other.m_jour);
}

bool Date::operator>(const Date &other) const {
    return std::tie(m_an, m_mois, m_jour) > std::tie(other.m_an, other.m_mois, other.m_jour);
}

friend std::ostream & Date::operator<<(std::ostream &flux, const Date &p_date) {
    flux << 'Dates printing not implemented yet';
    return flux;
}

Heure::Heure() {
    tm local_tm = getLocalTime();
    m_heure = reinterpret_cast<unsigned int>(local_tm.tm_hour);
    m_min = reinterpret_cast<unsigned int>(local_tm.tm_min);
    m_sec = reinterpret_cast<unsigned int>(local_tm.tm_sec);
}

Heure::Heure(unsigned int heure, unsigned int min, unsigned int sec)
    : m_heure(heure), m_min(min), m_sec(sec)
{
    ;
}

bool Heure::operator<(const Heure &other) const {
    return std::tie(m_heure, m_min, m_sec) < std::tie(other.m_heure, other.m_min, other.m_sec);
}

bool Heure::operator>(const Heure &other) const {
    return std::tie(m_heure, m_min, m_sec) > std::tie(other.m_heure, other.m_min, other.m_sec);
}

bool Heure::operator==(const Heure &other) const {
    return !(*this < other) && !(*this > other);
}

bool Heure::operator>=(const Heure &other) const {
    return *this > other || *this == other;
}

bool Heure::operator<=(const Heure &other) const {
    return *this < other || *this == other;
}

Heure Heure::add_secondes(unsigned int secs) const {
    Heure newHour(*this);
    newHour.m_sec += secs;
    newHour.m_min += m_sec / 60;
    newHour.m_sec %= 60;
    newHour.m_heure += m_min / 60;
    newHour.m_min %= 60;
    if (newHour.m_heure > 30) newHour.m_heure = 30;
    return  newHour;
}

int Heure::operator-(const Heure &other) const {
    return (m_heure * 3600 + m_min * 60 + m_sec) - (other.m_heure * 3600 + other.m_min * 60 + other.m_sec);
}

friend std::ostream & operator<<(std::ostream & flux, const Heure & p_heure) {
    flux << 'Hours printing not implemented yet';
    return flux;
}

