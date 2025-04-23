#include "geodata.h"

#include <bits/stdc++.h>

GeoData::GeoData()
{
}

std::chrono::system_clock::time_point GeoData::getTimePoint(const std::string &str)
{
    auto posNs = str.find_last_of(".");
    if (posNs == str.npos)
    {
        posNs = str.length();
    }
    else
    {
        posNs += 1;
    }
    std::string strNs{str.substr(posNs, str.length())};
    const auto reqPre{9};
    const auto curPre{strNs.length()};
    for (size_t i{0}; i < reqPre - curPre; ++i)
    {
        strNs += '0';
    }
    std::tm tm = {};
    std::stringstream ss{str};
    ss >> std::get_time(&tm, "%Y-%m-%d  %H:%M:%S");
    std::chrono::system_clock::time_point tp{std::chrono::system_clock::from_time_t(std::mktime(&tm))};
    tp += std::chrono::nanoseconds(std::atoll(strNs.c_str()));
    return tp;
}

std::ifstream &operator >> (std::ifstream &stream, GeoData &d)
{
    std::string str[4];
    stream >> str[0] >> str[1]>> str[2] >> str[3];
    d.period = {
        d.getTimePoint(str[0] + str[1]),
        d.getTimePoint(str[2] + str[3])
    };
    return stream;
}
