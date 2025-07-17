#include "energypeak.h"

EnergyPeak::EnergyPeak(EnergyPeak::Id id, double channel) :
    _id(id), _channel(channel)
{
    switch (_id) {
    case EnergyPeak::Id::FE847:
        _energy = 847.0;
        break;
    case EnergyPeak::Id::FE1238:
        _energy = 1238.0;
        break;
    case EnergyPeak::Id::HYDROGEN:
        _energy = 2238.0;
        break;
    case EnergyPeak::Id::CARBON:
        _energy = 4438.0;
        break;
    case EnergyPeak::Id::OXYGEN:
        _energy = 6129.0;
        break;
    case EnergyPeak::Id::FE7631:
        _energy = 7631.0;
        break;
    }
}

EnergyPeak::Id EnergyPeak::id() const
{
    return _id;
}

double EnergyPeak::channel() const
{
    return _channel;
}

double EnergyPeak::energy() const
{
    return _energy;
}
