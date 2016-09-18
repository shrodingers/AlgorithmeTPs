//
// Created by Leon on 18/09/2016.
//

#include <string>
#include <stdexcept>
#include <limits>
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