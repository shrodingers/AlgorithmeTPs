//
// Created by Leon on 16/10/2016.
//

#include <algorithm>
#include <unordered_set>
#include <list>
#include <functional>
#include "reseau.h"

Reseau::Reseau() :
        m_nbr_arcs(0)
{
    ;
}

void Reseau::ajouterSommet(unsigned int numero) throw (std::logic_error) {
    try {
        m_sommets.emplace(numero, liste_arcs());
    } catch (...) {
        throw std::logic_error("Erreur lors de la création d'un sommet");
    }
}

void Reseau::enleverSommet(unsigned int numero) throw (std::logic_error) {
    try {
        m_sommets.erase(numero);
    } catch (...) {
        throw std::logic_error("Erreur lors de la déletion d'un sommet");
    }

}

void Reseau::ajouterArc(unsigned int numOrigine, unsigned int numDest, unsigned int cout, unsigned int type) throw (std::logic_error) {
    m_nbr_arcs += 1;
    try {
        m_sommets.at(numOrigine)[numDest] = std::make_pair(cout, type);
    } catch (...) {
        throw std::logic_error("Erreur lors de l'ajout d'un arc");
    }

}

void Reseau::enleverArc(unsigned int numOrigine, unsigned int numDest) throw (std::logic_error) {
    try {
        m_sommets[numOrigine].erase(numDest);
    } catch (...) {
        throw std::logic_error("Erreur lors de la délétion d'un arc");
    }

}

void Reseau::majCoutArc(unsigned int numOrigine, unsigned int numDest, unsigned int cout) throw (std::logic_error) {
    try {
        m_sommets[numOrigine][numDest].first = cout;
    } catch (...) {
        throw std::logic_error("Erreur lors de la mise à jour du coût d'un arc");
    }

}

int Reseau::nombreSommets() const {
    return static_cast<int>(m_sommets.size());
}

int Reseau::nombreArcs() const {
    return m_nbr_arcs;
}

bool Reseau::estVide() const {
    return m_sommets.empty();
}

bool Reseau::sommetExiste(unsigned int numero) const {
    try {
        m_sommets.at(numero);
    } catch (std::out_of_range) {
        return false;
    } catch (...) {
        throw std::logic_error("Erreur lors de l'accès à un sommet");
    }
    return true;
}

bool Reseau::arcExiste(unsigned int numOrigine, unsigned int numDest) const throw (std::logic_error) {
    try {
        m_sommets.at(numOrigine).at(numDest);
    } catch (std::out_of_range) {
        return false;
    } catch (...) {
        throw std::logic_error("Erreur lors de l'accès à un arc");
    }
    return true;
}

int Reseau::getCoutArc(unsigned int numOrigine, unsigned int numDestination) const throw (std::logic_error) {
    try {
        return m_sommets.at(numOrigine).at(numDestination).first;
    } catch(...) {
        throw std::logic_error("Erreur lors de l'accès au coût d'un arc");
    }
}

int Reseau::getTypeArc(unsigned int numOrigine, unsigned int numDestination) const throw (std::logic_error) {
    try {
        return m_sommets.at(numOrigine).at(numDestination).second;
    } catch(...) {
        throw std::logic_error("Erreur lors de l'accès au type d'un arc");
    }
}

unsigned int Reseau::findMiniumum(liste_sommets const &graph, std::unordered_map<unsigned int, unsigned int>& distances) {
    unsigned int minimum = INFINI;
    unsigned int sommet = INFINI;

    for (auto& elem : graph) {
        if (distances[elem.first] < minimum) {
            minimum = distances[elem.first];
            sommet = elem.first;
        }
    }
    return sommet;
}

int Reseau::dijkstra(unsigned int numOrigine, unsigned int numDest, std::vector<unsigned int> &chemin)  throw (std::logic_error) {
    std::unordered_map<unsigned int, unsigned int> distances;
    std::unordered_map<unsigned int, unsigned int> predecesseurs;
    liste_sommets tmpGraph = m_sommets;
    unsigned int origin = INFINI;
    auto findWay = [&predecesseurs, &chemin, numDest, numOrigine] () {
        unsigned int current = numDest;
        while (current != numOrigine) {
            chemin.push_back(current);
            current = predecesseurs.at(current);
        }
        chemin.push_back(current);
        std::reverse(chemin.begin(), chemin.end());
    };

    for (auto& elem: tmpGraph) {
        distances[elem.first] = INFINI;
    }
    distances[numOrigine] = 0;
    while (!tmpGraph.empty()) {
        origin = findMiniumum(tmpGraph, distances);
        if (origin == numDest) {
            findWay();
            return 0;
        } else if (origin == INFINI) {
            throw std::logic_error("Le chemin entre les deux sommets n'existe pas");
        }
        for (auto& elem: tmpGraph[origin]) {
            unsigned int poids;

            poids = tmpGraph[origin][elem.first].first;
            if (distances[elem.first] > distances[origin] + poids) {
                distances[elem.first] = distances[origin] + poids;
                predecesseurs[elem.first] = origin;
            }
        }
        tmpGraph.erase(origin);
    }
    findWay();
    return 0;
}

int Reseau::bellmanFord(unsigned int numOrigine, unsigned int numDest, std::vector<unsigned int> &chemin) throw (std::logic_error) {
    std::unordered_map<unsigned int, unsigned int> distances;
    std::unordered_map<unsigned int, unsigned int> predecesseurs;
    bool hasChanged = true;

    auto findWay = [this, &predecesseurs, &distances, &chemin, numDest, numOrigine] () {
        unsigned int current = numDest;

        for (auto& sommet: m_sommets) {
            for (auto& arc: sommet.second) {
                unsigned int nouvDistance = distances[sommet.first] + arc.second.first;
                if (nouvDistance < distances[arc.first]) {
                    throw std::logic_error("Le graphe possède un cycle de poids négatif accessible");
                }
            }
        }
        while (current != numOrigine) {
            chemin.push_back(current);
            current = predecesseurs.at(current);
        }
        chemin.push_back(current);
        std::reverse(chemin.begin(), chemin.end());
    };

    for (auto& elem: m_sommets) distances[elem.first] = INFINI;
    distances[numOrigine] = 0;
    for (unsigned int i = 1; i != m_sommets.size() - 1; ++i) {
        if (!hasChanged) {
            findWay();
            return 0;
        }
        hasChanged = false;
        for (auto& sommet: m_sommets) {
            for (auto& arc: sommet.second) {
                unsigned int nouvDistance = distances[sommet.first] + arc.second.first;
                if (nouvDistance < distances[arc.first]) {
                    distances[arc.first] = nouvDistance;
                    predecesseurs[arc.first] = sommet.first;
                    hasChanged = true;
                }
            }
        }
    }
    findWay();
    return 0;
}

int Reseau::getComposantesFortementConnexes(std::vector<std::vector<unsigned int> > &composantes) const {
    std::unordered_set<unsigned int> visited;
    std::unordered_set<unsigned int> assignedList;
    std::unordered_map<unsigned int, std::vector<unsigned int> > assigned;
    std::list<unsigned int> visitStack;
    liste_sommets reversed = getReverseGraph();

    std::function<void(unsigned int)> visit = [this, &visit, &visited, &visitStack] (unsigned int sommet) {
        if (visited.find(sommet) == visited.end()) {
            visited.insert(sommet);
            for (auto& elem: m_sommets.at(sommet)) {
                visit(elem.first);
            }
            visitStack.push_front(sommet);
        }
    };

    std::function<void(unsigned int, unsigned int)> assign = [&assigned, &assign, &reversed, &assignedList] (unsigned int sommet, unsigned int root) {
        if (assignedList.find(sommet) == assignedList.end()) {
            assigned[root].push_back(sommet);
            assignedList.insert(sommet);
            for (auto &elem: reversed.at(sommet)) {
                assign(elem.first, root);
            }
        }
    };

    for (auto& elem: m_sommets) {
        visit(elem.first);
    }
    for (auto& elem: visitStack) {
        assign(elem, elem);
    }

    for (auto& elem: assigned) {
        composantes.push_back(elem.second);
    }
    return 0;
}



liste_sommets Reseau::getReverseGraph() const {
    liste_sommets reversed;

    for (auto& sommet: m_sommets) {
        for (auto& arc: sommet.second) {
            reversed[arc.first][sommet.first] = arc.second;
        }
        if (reversed.find(sommet.first) == reversed.end()) {
            reversed[sommet.first];
        }
    }
    return reversed;
}

void Reseau::printGraph(std::ostream &os, liste_sommets const &graph) {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: graph) {
        for (auto& arc: sommet.second) {
            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\"]" << std::endl;
        }
    }
    os << "}";
}

void Reseau::print(std::ostream &os) {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: m_sommets) {
        for (auto& arc: sommet.second) {
            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\"]" << std::endl;
        }
    }
    os << "}";
}

void Reseau::print(std::ostream &os, std::vector<unsigned int> const &chemin) {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: m_sommets) {
        for (auto& arc: sommet.second) {
            unsigned int save = 0;

            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\" ";
            for (auto& node: chemin) {
                if (save && save == sommet.first && node == arc.first) {
                    os << "color=red";
                }
                save = node;
            }
            os << "]" << std::endl;
        }
    }
    os << "}";
}

void Reseau::printConnexes(std::ostream &os, std::vector<std::vector<unsigned int> > const &compo) {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: m_sommets) {
        for (auto& arc: sommet.second) {
            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\" ";
            for (auto &group : compo) {
                unsigned int save = 0;
                if (std::find(group.begin(), group.end(), sommet.first) != group.end() &&
                    std::find(group.begin(), group.end(), arc.first) != group.end())
                    os << "color=blue";
            }
            os << "]" << std::endl;
        }
    }
    os << "}";
}