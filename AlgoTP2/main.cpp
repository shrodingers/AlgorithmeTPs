//
// Created by Leon on 16/10/2016.
//

#include <fstream>
#include "reseau.h"

int main() {
    Reseau res;
    std::ofstream os;
    std::ofstream os2;
    std::ofstream os3;
    std::vector<unsigned int> result;
    std::vector<unsigned int> result2;
    std::vector<std::vector<unsigned int> > compoConnexes;

    os.open("dijkstra.dot", std::ofstream::out | std::ofstream::trunc);
    os2.open("bellman.dot", std::ofstream::out | std::ofstream::trunc);
    os3.open("connexes.dot", std::ofstream::out | std::ofstream::trunc);
    res.ajouterSommet(1);
    res.ajouterSommet(42);
    res.ajouterSommet(23);
    res.ajouterSommet(55);
    res.ajouterSommet(19);
    res.ajouterSommet(15);
    res.ajouterSommet(13);
    res.ajouterSommet(7);
    res.ajouterArc(1, 42, 5);
    res.ajouterArc(1, 55, 12);
    res.ajouterArc(23, 42, 3);
    res.ajouterArc(55, 13, 6);
    res.ajouterArc(42, 55, 34);
    res.ajouterArc(42, 19, 15);
    res.ajouterArc(23, 7, 1);
    res.ajouterArc(55, 7, 4);
    res.ajouterArc(1, 15, 79);
    res.ajouterArc(55, 15, 13);
    res.ajouterArc(13, 15, 45);
    res.ajouterArc(13, 42, 13);
    res.ajouterArc(15, 7, 6);
    res.ajouterArc(23, 15, 3);
    res.ajouterArc(19, 7, 15);
    res.ajouterArc(23, 42, 10);
    res.ajouterArc(19, 7, 15);
    res.ajouterArc(42, 15, 42);
    res.ajouterArc(13, 23, 10);
    res.ajouterArc(19, 55, 5);
    res.ajouterArc(42, 7, 72);
    res.ajouterArc(13, 7, 14);
    res.ajouterArc(23, 19, 32);
    std::cout << res.dijkstra(42, 23, result) << std::endl;
    res.bellmanFord(42, 23, result2);
    //res.printGraph(os, res.getReverseGraph());
    res.print(os, result);
    res.print(os2, result2);
    res.getComposantesFortementConnexes(compoConnexes);
    res.printConnexes(os3, compoConnexes);
    return 0;
}