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

Date::Date()
{
    auto now = std::chrono::system_clock::now();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);
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
    return *this < other && *this > other;
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

