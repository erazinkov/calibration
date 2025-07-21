#ifndef CHANNEL_mapH
#define CHANNEL_mapH

#include <vector>
#include <sys/types.h>
#include <bits/stdc++.h>

#include "channel.h"

class ChannelMap
{
public:
    static ChannelMap mapNAP();
    unsigned long numberOfChannels(Channel::EChannelType type) const;
    bool isCorrect(std::vector<u_int8_t> &) const;
    const std::vector<Channel> &map() const;

private:
    std::vector<Channel> _map;
    ChannelMap(std::vector<Channel> map);
};

#endif // CHANNEL_mapH
