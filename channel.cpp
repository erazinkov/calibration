#include "channel.h"

Channel::Channel(EChannelType type, std::optional<u_int8_t> index)
    : index_{index}, type_{type}
{

}

std::optional<u_int8_t> Channel::index() const
{
    return index_;
}

Channel::EChannelType Channel::type() const
{
    return type_;
}
