//
// Created by prtos on 13/06/16.
//

#ifndef RTC_AUXILIAIRES_H
#define RTC_AUXILIAIRES_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

/*!
 * \brief Cette fonction permet de séparer une chaine de caractère en un vecteur de chaines de caractère séparés par un délimiteur
 * Par exemple "toto,tata,tutu" séparé par une virgule donnerait un vecteur contenant "toto", "tata", "tutu"
 * \param[in] s: la chaine de caractère à séparer
 * \param[in] delim: le caractère délimitant
 * \return le vecteur de contenant le résultat
 */
std::vector<std::string> split(const std::string &s, char delim);

/*!
 * \brief Permet de lire un fichier au format gtfs
 * \param[in] nomFichier: chemin d'acces au fichier qui est suposé contennir plusieurs lignes et plusieurs colonnes
 * \param[out] resultats: vecteur 2D destiné à contenir le fichier, l'élement [i][j] représente la ième ligne et la jème colonne du fichier
 * \param[in] delimiteur: le caractère délimiteur des colonnes dans ce fichier.
 * \param[in] rm_entete: un booléen qui spécifie s'il faut supprimer ou pas la première ligne du fichier.
 * \pre Le fichier existe.
 * \exception logic_error s'il y a un problème lors de l'ouverture du fichier.
 */
void lireFichier(std::string nomFichier, std::vector<std::vector<std::string>>& resultats, char delimiteur, bool rm_entete);



/*!
 * \class Date
 * \brief Cette classe représente une date.
 */
class Date {
public:
	Date();
	Date(unsigned int an, unsigned int mois, unsigned int jour);
	bool operator== (const Date & other) const;
	bool operator< (const Date & other) const;
	bool operator> (const Date & other) const;
	friend std::ostream & operator<<(std::ostream & flux, const Date & p_date);

private:
	unsigned int m_an;
	unsigned int m_mois;
	unsigned int m_jour;
};


/*!
 * \class Heure
 * \brief Cette classe représente l'heure d'une journée.
 * Cependant pour les besoins du travail pratique nous permettont qu'elle puisse encoder un nombre d'heures supérieurs à 24
 */
class Heure {
public:
	Heure();
	Heure(unsigned int heure, unsigned int min, unsigned int sec);
	Heure add_secondes(unsigned int secs) const;
	bool operator== (const Heure & other) const;
	bool operator< (const Heure & other) const;
	bool operator> (const Heure & other) const;
	bool operator<= (const Heure & other) const;
	bool operator>= (const Heure & other) const;
	int operator- (const Heure & other) const;
	friend std::ostream & operator<<(std::ostream & flux, const Heure & p_heure);

private:
	unsigned int m_heure;
	unsigned int m_min;
	unsigned int m_sec;
};


#endif //RTC_AUXILIAIRES_H
