#ifndef PEAKFINDER_H
#define PEAKFINDER_H

#include <TMath.h>
#include <TH1.h>
#include <TF1.h>

class PeakFinder
{
public:
    PeakFinder();
    void process(std::vector<TH1 *> &histsSg, std::vector<TH1 *> &histsRc);

    std::vector<std::vector<double>> getPar();

private:
    double _calib;
    double _offset;

    double getE(double ch) { return ch * _calib + _offset; }
    double getdE(double dCh) { return dCh * _calib; }
    double getCh(double e) { return (e - _offset) / _calib; }
    double getdCh(double de) { return de / _calib; }

    double getFerrum847PosApprox(TH1 *h, double r = 0.12);
    double getFerrum847Pos(TH1 *h);
    double getFerrum1238Pos(TH1 *h);
    double getHydrogenPos(TH1 *h);
    double getCarbonPos(TH1 *h, double appPos);
    double getOxygenPos(TH1 *h, double appPos);
    double getFerrum7631Pos(TH1 *h, double appPos);

    std::vector<TF1 *> _f;

};

#endif // PEAKFINDER_H
