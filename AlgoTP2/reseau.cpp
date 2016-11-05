//
// Created by Leon on 16/10/2016.
//

#include <algorithm>
#include <unordered_set>
#include <list>
#include <functional>
#include <numeric>
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
    try {
        m_sommets.at(numOrigine)[numDest] = std::make_pair(cout, type);
        m_nbr_arcs += 1;
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

    //Fonction qui construit le chemin avec les prédecesseurs
    auto findWay = [this, &predecesseurs, &chemin, numDest, numOrigine] () {
        unsigned int current = numDest;
        while (current != numOrigine) {
            chemin.push_back(current);
            current = predecesseurs.at(current);
        }
        chemin.push_back(current);
        std::reverse(chemin.begin(), chemin.end());
        //Retrouve le coût total du plus court chemin. Utilise les fonction d'ordre supérieur de la STL pour cela.
        return std::accumulate(std::next(chemin.begin()), chemin.end(), std::make_pair(0, chemin.begin()), [this] (std::pair<unsigned int,
                std::vector<unsigned int>::iterator> const& prev, unsigned int const sommet) {
            return std::make_pair(prev.first + m_sommets[*(prev.second)][sommet].first, std::next(prev.second));
        }).first;
    };

    //initialisation du graphe
    for (auto& elem: tmpGraph) {
        distances[elem.first] = INFINI;
    }
    int prev = 0;
    distances[numOrigine] = 0;
    while (!tmpGraph.empty()) {
        prev = origin;
        origin = findMiniumum(tmpGraph, distances);

        //Cas particulier qui indique la découverte du plus court chemin
        if (origin == numDest) {
            return findWay();
        } else if (origin == INFINI) {
            throw std::logic_error("Le chemin entre les deux sommets n'existe pas");
        }
        for (auto& elem: tmpGraph[origin]) {
            unsigned int poids;

            poids = tmpGraph[origin][elem.first].first;
            if (distances[origin] + poids < distances[elem.first]) {
                distances[elem.first] = distances[origin] + poids;
                predecesseurs[elem.first] = origin;
            }
        }
        tmpGraph.erase(origin);
    }
    return findWay();
}

int Reseau::bellmanFord(unsigned int numOrigine, unsigned int numDest, std::vector<unsigned int> &chemin) throw (std::logic_error) {
    std::unordered_map<unsigned int, unsigned int> distances;
    std::unordered_map<unsigned int, unsigned int> predecesseurs;
    bool hasChanged = true;

    //Fonction qui construit le chemin avec les prédecesseurs
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
        //Retrouve le coût total du plus court chemin. Utilise les fonction d'ordre supérieur de la STL pour cela.
        return std::accumulate(std::next(chemin.begin()), chemin.end(), std::make_pair(0, chemin.begin()), [this] (std::pair<unsigned int, std::vector<unsigned int>::iterator> const& prev, unsigned int const sommet) {
            return std::make_pair(prev.first + m_sommets[*(prev.second)][sommet].first, std::next(prev.second));
        }).first;
    };

    for (auto& elem: m_sommets) distances[elem.first] = INFINI;
    distances[numOrigine] = 0;
    for (unsigned int i = 1; i != m_sommets.size() - 1; ++i) {
        // Cas particulier indiquant que l'algorithme a déjà résolu tous les arcs
        if (!hasChanged) {
            return findWay();
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
    return findWay();
}

int Reseau::getComposantesFortementConnexes(std::vector<std::vector<unsigned int> > &composantes) const {
    std::unordered_set<unsigned int> visited;
    std::unordered_set<unsigned int> assignedList;
    std::unordered_map<unsigned int, std::vector<unsigned int> > assigned;
    std::list<unsigned int> visitStack;
    liste_sommets reversed = getReverseGraph();

    //Fonction récursive de visite en profondeur
    std::function<void(unsigned int)> visit = [this, &visit, &visited, &visitStack] (unsigned int sommet) {
        if (visited.find(sommet) == visited.end()) {
            visited.insert(sommet);
            for (auto& elem: m_sommets.at(sommet)) {
                visit(elem.first);
            }
            visitStack.push_front(sommet);
        }
    };

    // Fonction récursive de visite du graphe inverse en profondeur avec assignantion des composantes fortement connexes
    std::function<void(unsigned int, unsigned int)> assign = [&assigned, &assign, &reversed, &assignedList] (unsigned int sommet, unsigned int root) {
        if (assignedList.find(sommet) == assignedList.end()) {
            assigned[root].push_back(sommet);
            assignedList.insert(sommet);
            for (auto &elem: reversed.at(sommet)) {
                assign(elem.first, root);
            }
        }
    };

    //visite du graphe en profondeur
    for (auto& elem: m_sommets) {
        visit(elem.first);
    }

    // visite du graphe inverse en profondeur
    for (auto& elem: visitStack) {
        assign(elem, elem);
    }

    // Récupération des composantes fortement connexes
    for (auto& elem: assigned) {
        composantes.push_back(elem.second);
    }


    //Commentaire : Retourne la somme du coût des composantes fortement connexes, gardé sous la main au cas où
    /*
    // calcule le coût total de toutes les composante fortement connexes
    return std::accumulate(composantes.begin(), composantes.end(), 0, [this] (unsigned int const cout, std::vector<unsigned int> const& elem) {
        // calcule le coût total d'une composante fortement connexe
        auto total =  std::accumulate(elem.begin(), elem.end(), cout, [this, &elem] (unsigned int const cout_composante, unsigned int const sommet) {
            // calcule le coût total de tous les arcs liant un sommet de la composante aux autres sommets de la même composante
            return std::accumulate(elem.begin(), elem.end(), cout_composante, [this, sommet] (unsigned int const cout_arc, unsigned int const sommet_composante) {
                // ajoute au coût total la valeur de l'arc entre deux sommets de la composante fortement connexe
                return cout_arc + ((sommet != sommet_composante &&
                                    m_sommets.at(sommet).find(sommet_composante) != m_sommets.at(sommet).end())
                                   ? m_sommets.at(sommet).at(sommet_composante).first : 0);
            });
        });
        return total;
    });*/

    //Retourne la taille du vecteur contenat les composantes fortement connexes, la taille d'un vecteur étant dépendante de l'implémentation,
    // le cast est ici nécessaire, sachant que la limite de composantes fortements connexes pouvant être trouvée ici sera numeric_limits<int>::max
    return static_cast<unsigned int>(composantes.size());
}

bool Reseau::estFortementConnexe() const {
    std::vector<std::vector<unsigned int> > composantes;

    getComposantesFortementConnexes(composantes);
    return composantes.size() == 1 && composantes[0].size() == m_sommets.size();
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

/// Les fonctions suivantes en print* son utilisées pour sérialiser un graphe au format dot dans un fichier et ainsi pouvoir en avoir un rendu graphique
/// Elles sont commentées pour ne pas modifier l'interface publique
/*
//Sérialise un graphe
void Reseau::printGraph(std::ostream &os, liste_sommets const &graph) {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: graph) {
        for (auto& arc: sommet.second) {
            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\"]" << std::endl;
        }
    }
    os << "}";
}

// sérialise le graphe d'un réseau
void Reseau::print(std::ostream &os) const {
    os << "digraph myGraph {" << std::endl;
    for (auto& sommet: m_sommets) {
        for (auto& arc: sommet.second) {
            os << sommet.first << " -> " << arc.first << " [label=\"" << arc.second.first << "\"]" << std::endl;
        }
    }
    os << "}";
}


//sérialise le graphe d'un réseau et colorise le chemin des noeuds en rouge
void Reseau::print(std::ostream &os, std::vector<unsigned int> const &chemin) const {
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

//sérialise le graphe d'un réseau et colorise les arcs des composantes fortement connexes en bleu
void Reseau::printConnexes(std::ostream &os, std::vector<std::vector<unsigned int> > const &compo) const {
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
*/