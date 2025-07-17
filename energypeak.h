#ifndef ENERGYPEAK_H
#define ENERGYPEAK_H

#include <iostream>

class EnergyPeak
{
public:
    enum class Id {
        FE847,
        FE1238,
        HYDROGEN,
        CARBON,
        OXYGEN,
        FE7631
    };
    EnergyPeak(EnergyPeak::Id id, double channel);
    EnergyPeak::Id id() const;

    double channel() const;

    double energy() const;

private:
    EnergyPeak::Id _id;
    double _channel;
    double _energy;
};

#endif // ENERGYPEAK_H
