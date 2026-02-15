#include "galaxy/GalaxySector.h"

namespace fresh
{
namespace galaxy
{

GalaxySector::GalaxySector() : x(0), y(0) {}

GalaxySector::GalaxySector(int x, int y) : x(x), y(y) {}

void GalaxySector::addAsteroid(const AsteroidData& asteroid)
{
    asteroids.push_back(asteroid);
}

void GalaxySector::addStation(const StationData& station)
{
    stations.push_back(station);
}

void GalaxySector::addShip(const ShipData& ship)
{
    ships.push_back(ship);
}

void GalaxySector::clear()
{
    asteroids.clear();
    stations.clear();
    ships.clear();
}

} // namespace galaxy
} // namespace fresh
