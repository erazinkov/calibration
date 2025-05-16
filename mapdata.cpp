#include "mapdata.h"

MapData::MapData()
{
}

std::chrono::system_clock::time_point MapData::strToTimePoint(const std::string &str)
{
    std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(std::atoll(str.c_str()))};
    return tp;
}

long long int MapData::getNanoSeconds() const
{
    auto ns{
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::time_point_cast<std::chrono::nanoseconds>(this->lastModified)
        .time_since_epoch())
        .count()
    };
    return ns;
}

std::ifstream &operator >> (std::ifstream &stream, MapData &d)
{
    stream >> d.fileName;
    std::string lastModified;
    stream >> lastModified;
    d.lastModified = d.strToTimePoint(lastModified);
    stream >> d.offset;
    return stream;
}
