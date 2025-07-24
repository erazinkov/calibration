#include "channel.h"

Channel::Channel(EChannelType type, u_int8_t hardwareIndex, u_int8_t softwareIndex)
    : _hardwareIndex{hardwareIndex}, _softwareIndex{softwareIndex}, _type{type}
{

}

u_int8_t Channel::hardwareIndex() const
{
    return _hardwareIndex;
}

u_int8_t Channel::softwareIndex() const
{
    return _softwareIndex;
}

Channel::EChannelType Channel::type() const
{
    return _type;
}
