#include "mapdata.h"

MapData::MapData()
{
}

std::chrono::system_clock::time_point MapData::getTimePoint(const std::string &str)
{
    std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(std::atoll(str.c_str()))};
    return tp;
}

std::ifstream &operator >> (std::ifstream &stream, MapData &d)
{
    stream >> d.fileName;
    std::string lastModified;
    stream >> lastModified;
    d.lastModified = d.getTimePoint(lastModified);
    stream >> d.offset;
    return stream;
}
