#include "channel.h"

Channel::Channel(EChannelType type, u_int8_t hardwareIndex, u_int8_t physicalIndex)
    : _hardwareIndex{hardwareIndex}, _physicalIndex{physicalIndex}, _type{type}
{

}

u_int8_t Channel::hardwareIndex() const
{
    return _hardwareIndex;
}

u_int8_t Channel::physicalIndex() const
{
    return _physicalIndex;
}

Channel::EChannelType Channel::type() const
{
    return _type;
}
