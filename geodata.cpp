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

std::vector<std::string> GeoData::splitLine(const std::string &line)
{
    std::stringstream ss(line);
    std::string str;

    std::vector<std::string> strs;
    while (ss >> str)
    {
        strs.push_back(str);
    }
    return strs;
}

std::ifstream &operator >> (std::ifstream &stream, GeoData &d)
{
    std::string line;
    getline(stream, line);

    std::string first{""},
                second{""};
    auto strs{d.splitLine(line)};
    if (strs.size() == 4)
    {
        first = strs.at(0) + strs.at(1);
        second = strs.at(2) + strs.at(3);
    }

    d.period = {
        d.getTimePoint(first),
        d.getTimePoint(second)
    };
    return stream;
}
