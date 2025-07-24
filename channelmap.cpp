#include "channelmap.h"

#include <algorithm>

ChannelMap::ChannelMap(std::vector<Channel> map)
{
    _map = map;
}

ChannelMap ChannelMap::mapNAP()
{


    std::vector<Channel> map
    {
        Channel(Channel::GAMMA, 0, 0),
        Channel(Channel::GAMMA, 1, 1),
        Channel(Channel::GAMMA, 2, 2),
        Channel(Channel::GAMMA, 3, 3),
        Channel(Channel::GAMMA, 4, 4),
        Channel(Channel::GAMMA, 5, 5),
        Channel(Channel::UNKNOWN, 111, 111),
        Channel(Channel::ALPHA, 0, 0),
        Channel(Channel::ALPHA, 1, 1),
        Channel(Channel::ALPHA, 2, 2),
        Channel(Channel::ALPHA, 3, 3),
        Channel(Channel::ALPHA, 4, 4),
        Channel(Channel::ALPHA, 5, 5),
        Channel(Channel::ALPHA, 6, 6),
        Channel(Channel::ALPHA, 7, 7),
        Channel(Channel::ALPHA, 8, 8),
    };
    return ChannelMap(map);
}

unsigned long ChannelMap::numberOfChannels(Channel::EChannelType type) const
{
    unsigned long number{};
    auto it = _map.begin();

    while ( (it = std::find_if(it, _map.end(), [&type](Channel mapItem){return mapItem.type() == type;}) ) != _map.end())
    {
        ++number;
        ++it;
    }
    return number;
}

bool ChannelMap::isCorrect(std::vector<u_int8_t> &map) const {
//    if (map.size() != _map.size())
//    {
//        return false;
//    }

//    for (size_t i{0}; i < _map.size(); ++i)
//    {
//        if (_map[i].first == UNKNOWN)
//        {
//            continue;
//        }
//        if ( (map.at(i) & _map.at(i).first) != _map.at(i).first )
//        {
//            return false;
//        }
//    }
    return true;
}

const std::vector<Channel> &ChannelMap::map() const
{
    return _map;
}

std::optional<u_int8_t> ChannelMap::getSoftwareIdxByHardwareIdx(u_int8_t hardwareIndex)
{


    auto it{std::find_if(_map.begin(), _map.end(), [&hardwareIndex](Channel channel){
            return channel.hardwareIndex() == hardwareIndex;
        })};

    if (it != _map.end())
    {
        return it.base()->softwareIndex();
    }

    return std::nullopt;
}


