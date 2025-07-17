#include "polynomialfunction.h"

#include "TF1.h"
#include "TGraphErrors.h"

PolynomialFunction::PolynomialFunction(const std::vector<EnergyPeak> &energyPeaks)
{
    _par = par(energyPeaks);
}

PolynomialFunction::Par PolynomialFunction::par(const std::vector<EnergyPeak> &energyPeaks)
{
    Par par;
    TF1 f("f", "pol3" , energyPeaks.at(0).channel(),  energyPeaks.at(energyPeaks.size() - 1).channel());
    TGraph g(static_cast<int>(energyPeaks.size()));

    for (size_t i{0}; i < energyPeaks.size(); ++i) {
        g.SetPoint(static_cast<int>(i), energyPeaks.at(i).channel(), energyPeaks.at(i).energy());
    }
    g.Fit(&f, "RQN0");

    for (auto i{0}; i < f.GetNpar(); ++i) {
        par.p.push_back(f.GetParameter(i));
    }

    return par;
}
