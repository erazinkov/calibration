#ifndef PEAKFINDER_H
#define PEAKFINDER_H

#include <TMath.h>
#include <TH1.h>

class PeakFinder
{
public:
    PeakFinder();
    double getFerrum847PosApprox(TH1 *h, double r = 0.1);
    double getFerrum847Pos(TH1 *h, double appPos);
    double getFerrum1238Pos(TH1 *h, double appPos);
    double getHydrogenPos(TH1 *h, double appPos);
    double getCarbonPos(TH1 *h, double appPos);
private:
    double _calib;
    double _offset;

    double getE(double ch) { return ch * _calib + _offset; }
    double getdE(double dCh) { return dCh * _calib; }
    double getCh(double e) { return (e - _offset) / _calib; }
    double getdCh(double de) { return de / _calib; }

};

#endif // PEAKFINDER_H
