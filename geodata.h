#ifndef GEODATA_H
#define GEODATA_H

#include <fstream>
#include <chrono>

class GeoData
{
public:
    GeoData();
    friend std::ifstream &operator >> (std::ifstream &, GeoData &);

    std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point> period;
private:
    std::chrono::system_clock::time_point getTimePoint(const std::string &);
};

#endif // GEODATA_H
