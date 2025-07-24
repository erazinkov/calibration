#ifndef CHANNEL_mapH
#define CHANNEL_mapH

#include <vector>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <optional>

#include "channel.h"

class ChannelMap
{
public:
    static ChannelMap mapNAP();
    unsigned long numberOfChannels(Channel::EChannelType type) const;
    bool isCorrect(std::vector<u_int8_t> &) const;
    const std::vector<Channel> &map() const;
    std::optional<u_int8_t> getSoftwareIdxByHardwareIdx(u_int8_t hardwareIndex);
private:
    std::vector<Channel> _map;
    ChannelMap(std::vector<Channel> map);
};

#endif // CHANNEL_mapH
