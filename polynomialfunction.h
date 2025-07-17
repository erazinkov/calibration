#ifndef POLYNOMIALFUNCTION_H
#define POLYNOMIALFUNCTION_H

#include "energypeak.h"
#include <vector>
#include <cmath>

class PolynomialFunction
{
public:
    PolynomialFunction(const std::vector<EnergyPeak> &energyPeaks);

    double operator() (double *x, double *) {
       double arg{x[0]};
       double val{0.0};
       for (size_t i{0}; i < _par.p.size(); ++i) {
           val += _par.p.at(i) * std::pow(arg, i);
       }
       return val;
   }
private:
    struct Par
    {
        std::vector<double> p;
    };

    Par _par;

    Par par(const std::vector<EnergyPeak> &energyPeaks);
};

#endif // POLYNOMIALFUNCTION_H
