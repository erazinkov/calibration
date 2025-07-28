#include "channelmap.h"

#include <algorithm>
#include <stdexcept>

ChannelMap::ChannelMap(std::map<u_int8_t, Channel> map)
{
    _map = map;
}

ChannelMap ChannelMap::mapNAP()
{

    std::map<u_int8_t, Channel> map
    {
        {0, Channel(Channel::GAMMA, 0)},
        {1, Channel(Channel::GAMMA, 1)},
        {2, Channel(Channel::GAMMA, 2)},
        {3, Channel(Channel::GAMMA, 3)},
        {4, Channel(Channel::GAMMA, 4)},
        {5, Channel(Channel::GAMMA, 5)},
        {6, Channel(Channel::UNKNOWN, 111)},
        {7, Channel(Channel::ALPHA, 0)},
        {8, Channel(Channel::ALPHA, 1)},
        {9, Channel(Channel::ALPHA, 2)},
        {10, Channel(Channel::ALPHA, 3)},
        {11, Channel(Channel::ALPHA, 4)},
        {12, Channel(Channel::ALPHA, 5)},
        {13, Channel(Channel::ALPHA, 6)},
        {14, Channel(Channel::ALPHA, 7)},
        {15, Channel(Channel::ALPHA, 8)},
    };

    return ChannelMap(map);
}

unsigned long ChannelMap::numberOfChannels(Channel::EChannelType type) const
{
    unsigned long number{};
    auto it = _map.begin();

    while ( (it = std::find_if(it, _map.end(), [&type](std::pair<u_int8_t, Channel> mapItem){return mapItem.second.type() == type;}) ) != _map.end())
    {
        ++number;
        ++it;
    }
    return number;
}

std::vector<int> ChannelMap::getIdxsByType(Channel::EChannelType type) const
{
    std::vector<int> idxs;

    auto it = _map.begin();

    while ( (it = std::find_if(it, _map.end(), [&type](std::pair<u_int8_t, Channel> mapItem){return mapItem.second.type() == type;}) ) != _map.end())
    {
        if ((*it).second.index().has_value())
        {
            idxs.push_back((*it).second.index().value());
        }
        ++it;
    }
    return idxs;
}

bool ChannelMap::isCorrect(std::vector<u_int8_t> &map) const {
    if (map.size() != _map.size())
    {
        return false;
    }

    for (size_t i{0}; i < _map.size(); ++i)
    {
        if (_map.at(static_cast<u_int8_t>(i)).type() == Channel::UNKNOWN)
        {
            continue;
        }
        if ( (map.at(i) & _map.at(static_cast<u_int8_t>(i)).type()) != _map.at(static_cast<u_int8_t>(i)).type() )
        {
            return false;
        }
    }
    return true;
}

const std::map<u_int8_t, Channel> &ChannelMap::map() const
{
    return _map;
}

std::optional<u_int8_t> ChannelMap::getIdxByHardwareIdx(u_int8_t &hardwareIndex)
{
    auto it{_map.find(hardwareIndex)};
    if (it != _map.end())
    {
        return it->second.index();
    }
    return std::nullopt;
}


