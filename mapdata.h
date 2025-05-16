#ifndef MAPDATA_H
#define MAPDATA_H


#include <fstream>
#include <chrono>

class MapData
{
public:
    MapData();
    friend std::ifstream &operator >> (std::ifstream &, MapData &);

    std::string fileName{""};
    std::chrono::system_clock::time_point lastModified{std::chrono::system_clock::now()};
    long long int offset{0};

    long long int getNanoSeconds() const;

private:
    std::chrono::system_clock::time_point strToTimePoint(const std::string &);

};

#endif // MAPDATA_H
