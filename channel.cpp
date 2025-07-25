#include "channel.h"

Channel::Channel(EChannelType type, std::optional<u_int8_t> index)
    : _index{index}, _type{type}
{

}

std::optional<u_int8_t> Channel::index() const
{
    return _index;
}

Channel::EChannelType Channel::type() const
{
    return _type;
}
