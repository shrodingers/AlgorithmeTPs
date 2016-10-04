//
// Created by Leon on 18/09/2016.
//

#include <iomanip>
#include <math.h>
#include "Enonce/coordonnees.h"

int main(int argc, char **argv) {
    
    Coordonnees c(78.554, 15.4562);
    Coordonnees c2(48.4656, -76.485);

    std::cout << c.getLatitude() << " -- " << c.getLongitude() << std::endl;

    std::cout << Coordonnees::is_valide_coord(78.1542, 48.5614) << std::endl;

    c.setLatitude(49.4521);
    c.setLongitude(16.455);

    std::cout << c.getLatitude() << " -- " << c.getLongitude() << std::endl;

    std::cout << c << std::endl;
    std::cout << c2 << std::endl;

    double dist = c - c2;

    std::cout << std::setprecision(10) << dist << std::endl;
}
