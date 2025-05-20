#ifndef GEODATA_H
#define GEODATA_H

#include <fstream>
#include <chrono>
#include <vector>

class GeoData
{
public:
    GeoData();
    friend std::ifstream &operator >> (std::ifstream &, GeoData &);

    std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point> period;
    std::pair<long long int, long long int> getNanoSeconds() const;
private:
    std::chrono::system_clock::time_point getTimePoint(const std::string &);
    std::vector<std::string> splitLine(const std::string &line);
};

#endif // GEODATA_H
