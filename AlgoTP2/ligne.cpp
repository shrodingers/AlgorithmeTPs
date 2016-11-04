//
// Created by Leon on 21/09/2016.
//

#include <algorithm>
#include "ligne.h"

const std::list<std::function< int(std::string const&) > > Ligne::CategoryTester = {
        [] (std::string const& str) { return str == "97BF0D" ? CategorieBus::METRO_BUS : nullCategory; },
        [] (std::string const& str) { return str == "013888" ? CategorieBus::LEBUS : nullCategory; },
        [] (std::string const& str) { return str == "E04503" ? CategorieBus::EXPRESS : nullCategory; },
        [] (std::string const& str) { return str == "1A171B" || str == "003888" ? CategorieBus::COUCHE_TARD : nullCategory; }
};

const std::string Ligne::nameCorres[] = {
        "METRO_BUS",
        "LEBUS",
        "EXPRESS",
        "COUCHE_TARD"
};

const std::string Ligne::colorCorresp[]  = {
        "97BF0D", "013888", "E04503", "1A171B"
};

Ligne::Ligne(const std::vector<std::string> &ligne_gtfs)
  try  :   m_id(StringConverter::fromString<unsigned int>(ligne_gtfs[routeIdIndex])),
        m_numero(ligne_gtfs[routeShortNameIndex].substr(1, ligne_gtfs[routeShortNameIndex].size() - 2)),
        m_description(ligne_gtfs[routeDescIndex]),
        m_categorie(couleurToCategorie(ligne_gtfs[routeColorIndex]))
{
    ;
} catch (std::invalid_argument& err) {
    std::cout << "Error in construction :" << err.what() << std::endl;
}

std::string Ligne::categorieToString(CategorieBus c) {
    return (c <= CategorieBus::COUCHE_TARD && c >= CategorieBus::METRO_BUS) ? nameCorres[static_cast<unsigned int>(c)] : "";
}

CategorieBus Ligne::couleurToCategorie(std::string couleur) {
    int ret;

    for (auto& tester : CategoryTester) {
        if ((ret = tester(couleur)) != -1)
            return static_cast<CategorieBus>(ret);
    }
    return nullCategory;
}

CategorieBus Ligne::getCategorie() const {
    return m_categorie;
}

const std::string& Ligne::getDescription() const {
    return m_description;
}

std::pair<std::string, std::string> Ligne::getDestinations() const {
    std::string dest1, dest2 = "";

    for (auto& trip : m_voyages) {
        std::string tripDest(trip->getDestination());
        if (dest1.empty() && dest1 != "null") dest1 = tripDest;
        else if (dest2.empty() && tripDest != dest1 && dest2 != "null") dest2 = tripDest;
    }
    return std::make_pair(dest1, dest2);
}

unsigned int Ligne::getId() const {
    return m_id;
}

const std::string& Ligne::getNumero() const {
    return m_numero;
}

const std::vector<Voyage*>& Ligne::getVoyages() const {
    return m_voyages;
}

void Ligne::setCategorie(CategorieBus categorie) {
    m_categorie = categorie;
}

void Ligne::setDescription(const std::string &description) {
    m_description = description;
}

void Ligne::setId(unsigned int id) {
    m_id = id;
}

void Ligne::setNumero(const std::string &numero) {
    m_numero = numero;
}

void Ligne::setVoyages(const std::vector<Voyage *> &voyages) {
    m_voyages = voyages;
}

void Ligne::addVoyage(Voyage *ptr_voyage) {
    m_voyages.push_back(ptr_voyage);
}

std::ostream& operator <<(std::ostream& f, const Ligne& p_ligne) {
    std::string name = Ligne::nameCorres[p_ligne.m_categorie];
    auto dest = p_ligne.getDestinations();
    f << name << ' ' << p_ligne.m_numero << " : " << dest.first << (dest.second != "" ? " - "  : "") << dest.second << std::endl;
    return f;
}