#ifndef CHANNELMAP_H
#define CHANNELMAP_H

#include <vector>
#include <sys/types.h>
#include <bits/stdc++.h>


enum EChannelFlag {
    FLAG_INACTIVE   = 0b0000'0000,
    FLAG_ACTIVE     = 0b0000'0001,
    FLAG_ALPHA      = 0b0000'0010,
    FLAG_GAMMA      = 0b0000'0100,
};

enum EChannelType {
    GAMMA   = FLAG_ACTIVE | FLAG_GAMMA,
    ALPHA   = FLAG_ACTIVE | FLAG_ALPHA,
    SELF    = FLAG_INACTIVE,
    UNKNOWN = FLAG_INACTIVE,
};

class ChannelMap
{
public:
    static ChannelMap mapNAP();
    unsigned long numberOfChannelsAlpha() const;
    unsigned long numberOfChannelsGamma() const;
    u_int8_t numberByChannel(unsigned long) const;
    u_int8_t typeByChannel(unsigned long) const;
    bool isCorrect(std::vector<u_int8_t> &) const;
    const std::vector<std::pair<u_int8_t, u_int8_t> > &map() const;

private:
    std::vector<std::pair<u_int8_t, u_int8_t>> map_;
    unsigned long numberOfChannels(EChannelType type) const;
    ChannelMap(std::vector<std::pair<u_int8_t, u_int8_t>> map);
};

#endif // CHANNELMAP_H
