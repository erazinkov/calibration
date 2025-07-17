#include "piecewiselinearfunction.h"

#include <algorithm>

PiecewiseLinearFunction::PiecewiseLinearFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    for (size_t i{0}; i < energyPeaks.size() - 1; ++i) {
        _par.push_back(par(energyPeaks.at(i), energyPeaks.at(i + 1)));
    }
    std::sort(_par.begin(), _par.end());
}

PiecewiseLinearFunction::Par PiecewiseLinearFunction::par(const EnergyPeak &prev, const EnergyPeak &next)
{
    Par par;
    par.slope = ( next.energy() - prev.energy() ) / ( next.channel() - prev.channel() );
    par.intercept = next.energy() - par.slope * next.channel();
    par.node = next.channel();
    return par;
}

