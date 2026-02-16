#ifndef CHANNEL_mapH
#define CHANNEL_mapH

#include <map>
#include <vector>
//#include <sys/types.h>
//#include <bits/stdc++.h>

#include "channel.h"

class ChannelMap
{
public:
    static ChannelMap mapNAP();
    static ChannelMap mapTMP();

    std::vector<int> getIdxsByType(Channel::EChannelType type) const;

    bool isCorrect(std::vector<u_int8_t> &) const;
    const std::map<u_int8_t, Channel> &map() const;
    std::optional<u_int8_t> getIdxByHardwareIdx(u_int8_t &hardwareIndex);

private:
    std::map<u_int8_t, Channel> map_;
    ChannelMap(std::map<u_int8_t, Channel> map);
};

#endif // CHANNEL_mapH
