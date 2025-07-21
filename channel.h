#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/types.h>

class Channel
{
public:
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
    Channel(EChannelType type, u_int8_t hardwareIndex, u_int8_t physicalIndex);
    u_int8_t hardwareIndex() const;
    u_int8_t physicalIndex() const;

    EChannelType type() const;

private:
    u_int8_t _hardwareIndex;
    u_int8_t _physicalIndex;
    EChannelType _type;
};

#endif // CHANNEL_H
