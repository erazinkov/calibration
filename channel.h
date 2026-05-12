#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/types.h>
#include <optional>

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
        GAMMA    = FLAG_ACTIVE | FLAG_GAMMA,
        ALPHA    = FLAG_ACTIVE | FLAG_ALPHA,
        UNKNOWN  = FLAG_INACTIVE,
    };
    Channel(EChannelType type, std::optional<u_int8_t> index);
    std::optional<u_int8_t> index() const;

    EChannelType type() const;

private:
    std::optional<u_int8_t> _index;
    EChannelType _type;
};

#endif // CHANNEL_H
