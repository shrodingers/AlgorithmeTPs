
#include <math.h>
#include "Enonce/coordonnees.h"

Coordonnees::Coordonnees(double latitude, double longitude)
{
	if (!Coordonnees::is_valide_coord(latitude, longitude))
		return ;
	m_latitude = latitude;
	m_longitude = longitude;
}

double Coordonnees::getLatitude() const
{
	return (m_latitude);
}

void Coordonnees::setLatitude(double latitude)
{
	m_latitude = latitude;
}

double Coordonnees::getLongitude() const
{
	return (m_longitude);
}

void Coordonnees::setLongitude(double longitude)
{
	m_longitude = longitude;
}

bool Coordonnees::is_valide_coord(double p_latitude, double p_longitude)
{
	if (p_latitude < -90 || p_latitude > 90)
		return (false);
	if (p_longitude < -180 || p_longitude > 180)
		return (false);
	return (true);
}

double Coordonnees::operator-(const Coordonnees & other) const
{
	double rayon = 6371;
	double pi = 3.141592;

	return (2.0 * rayon * asin(sqrt(
		pow(sin(((other.getLatitude() * pi / 180.0) - (this->getLatitude()  * pi / 180.0)) / 2.0), 2.0) 
		+ cos((this->getLatitude() * pi / 180.0)) 
		* cos((other.getLatitude() * pi / 180.0)) 
		* pow(sin(((other.getLongitude() * pi / 180.0) - (this->getLongitude() * pi / 180.0)) / 2.0), 2.0))));
}

std::ostream & operator<<(std::ostream & flux, const Coordonnees & p_coord)
{
	flux << p_coord.getLatitude() << " : " << p_coord.getLongitude();
	return (flux);
}