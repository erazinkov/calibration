#include "channelmap.h"

#include <algorithm>
#include <stdexcept>

ChannelMap::ChannelMap(std::map<u_int8_t, Channel> map)
{
    map_ = map;
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

//ChannelMap ChannelMap::mapTMP()
//{
//    std::map<u_int8_t, Channel> map
//    {
//        {0, Channel(Channel::GAMMA, 0)},
//        {1, Channel(Channel::GAMMA, 1)},
//        {2, Channel(Channel::GAMMA, 2)},
//        {3, Channel(Channel::GAMMA, 3)},
//        {4, Channel(Channel::GAMMA, 4)},
//        {5, Channel(Channel::GAMMA, 5)},
//        {6, Channel(Channel::GAMMA, 6)},
//        {7, Channel(Channel::GAMMA, 7)},
//        {8, Channel(Channel::ALPHA, 0)},
//        {9, Channel(Channel::ALPHA, 1)},
//        {10, Channel(Channel::ALPHA, 2)},
//        {11, Channel(Channel::ALPHA, 3)},
//        {12, Channel(Channel::ALPHA, 4)},
//        {13, Channel(Channel::ALPHA, 5)},
//        {14, Channel(Channel::ALPHA, 6)},
//        {15, Channel(Channel::ALPHA, 7)},
//    };

//    return ChannelMap(map);
//}

ChannelMap ChannelMap::mapTMP()
{
    std::map<u_int8_t, Channel> map
    {
        {0, Channel(Channel::ALPHA, 0)},
        {1, Channel(Channel::ALPHA, 1)},
        {2, Channel(Channel::ALPHA, 2)},
        {3, Channel(Channel::ALPHA, 3)},
        {4, Channel(Channel::ALPHA, 4)},
        {5, Channel(Channel::ALPHA, 5)},
        {6, Channel(Channel::ALPHA, 6)},
        {7, Channel(Channel::ALPHA, 7)},
        {8, Channel(Channel::ALPHA, 8)},
        {9, Channel(Channel::ALPHA, 9)},
        {10, Channel(Channel::ALPHA, 10)},
        {11, Channel(Channel::ALPHA, 11)},
        {12, Channel(Channel::ALPHA, 12)},
        {13, Channel(Channel::ALPHA, 13)},
        {14, Channel(Channel::UNKNOWN, 111)},
        {15, Channel(Channel::UNKNOWN, 111)},
        {16, Channel(Channel::UNKNOWN, 111)},
        {17, Channel(Channel::UNKNOWN, 111)},
        {18, Channel(Channel::UNKNOWN, 111)},
        {19, Channel(Channel::UNKNOWN, 111)},
        {20, Channel(Channel::GAMMA, 4)},
        {21, Channel(Channel::UNKNOWN, 111)},
        {22, Channel(Channel::UNKNOWN, 111)},
        {23, Channel(Channel::UNKNOWN, 111)},
        {24, Channel(Channel::UNKNOWN, 111)},
        {25, Channel(Channel::UNKNOWN, 111)},
        {26, Channel(Channel::UNKNOWN, 111)},
        {27, Channel(Channel::UNKNOWN, 111)},
        {28, Channel(Channel::UNKNOWN, 111)},
        {29, Channel(Channel::UNKNOWN, 111)},
        {30, Channel(Channel::UNKNOWN, 111)},
        {31, Channel(Channel::UNKNOWN, 111)},
    };

    return ChannelMap(map);
}

//ChannelMap ChannelMap::mapTMP()
//{
//    std::map<u_int8_t, Channel> map
//    {
//        {0, Channel(Channel::GAMMA, 0)},
//        {1, Channel(Channel::GAMMA, 1)},
//        {2, Channel(Channel::GAMMA, 2)},
//        {3, Channel(Channel::GAMMA, 3)},
//        {4, Channel(Channel::GAMMA, 4)},
//        {5, Channel(Channel::GAMMA, 5)},
//        {6, Channel(Channel::GAMMA, 6)},
//        {7, Channel(Channel::GAMMA, 7)},
//        {8, Channel(Channel::GAMMA, 8)},
//        {9, Channel(Channel::GAMMA, 9)},
//        {10, Channel(Channel::GAMMA, 10)},
//        {11, Channel(Channel::GAMMA, 11)},
//        {12, Channel(Channel::UNKNOWN, 111)},
//        {13, Channel(Channel::UNKNOWN, 111)},
//        {14, Channel(Channel::UNKNOWN, 111)},
//        {15, Channel(Channel::UNKNOWN, 111)},
//        {16, Channel(Channel::UNKNOWN, 111)},
//        {17, Channel(Channel::UNKNOWN, 111)},
//        {18, Channel(Channel::UNKNOWN, 111)},
//        {19, Channel(Channel::UNKNOWN, 111)},
//        {20, Channel(Channel::UNKNOWN, 111)},
//        {21, Channel(Channel::UNKNOWN, 111)},
//        {22, Channel(Channel::UNKNOWN, 111)},
//        {23, Channel(Channel::ALPHA, 0)},
//        {24, Channel(Channel::ALPHA, 1)},
//        {25, Channel(Channel::ALPHA, 2)},
//        {26, Channel(Channel::ALPHA, 3)},
//        {27, Channel(Channel::ALPHA, 4)},
//        {28, Channel(Channel::ALPHA, 5)},
//        {29, Channel(Channel::ALPHA, 6)},
//        {30, Channel(Channel::ALPHA, 7)},
//        {31, Channel(Channel::ALPHA, 8)},
//    };

//    return ChannelMap(map);
//}

std::vector<int> ChannelMap::getIdxsByType(Channel::EChannelType type) const
{
    std::vector<int> idxs;

    auto it = map_.begin();

    while ( (it = std::find_if(it, map_.end(), [&type](std::pair<u_int8_t, Channel> mapItem){return mapItem.second.type() == type;}) ) != map_.end())
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
    if (map.size() != map_.size())
    {
        return false;
    }
    for (size_t i{0}; i < map_.size(); ++i)
    {
        if (map_.at(static_cast<u_int8_t>(i)).type() == Channel::UNKNOWN)
        {
            continue;
        }
        if ( (map.at(i) & map_.at(static_cast<u_int8_t>(i)).type()) != map_.at(static_cast<u_int8_t>(i)).type() )
        {
            return false;
        }
    }
    return true;
}

const std::map<u_int8_t, Channel> &ChannelMap::map() const
{
    return map_;
}

std::optional<u_int8_t> ChannelMap::getIdxByHardwareIdx(u_int8_t &hardwareIndex)
{
    auto it{map_.find(hardwareIndex)};
    if (it != map_.end())
    {
        return it->second.index();
    }
    return std::nullopt;
}


