#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <functional>

#include <TH1.h>
#include <TMath.h>

#include "adcm_df.h"
#include "channelmap.h"
#include "filloptions.h"

class Calibration
{
public:
    Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events);
    void process();

private:

    enum class Range {

    };

    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t ig, u_int8_t ia);

    void fillHistTime(const std::vector<dec_ev_t> &, TH1 *, double);
    void fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude);

    void drawHistsToFile(const std::string &psName, const std::vector<std::vector<TH1 *> > &hists) const;
    void prepareHists(const std::string &histName,
                      int nBinsX,
                      double xLow,
                      double xUp,
                      std::vector<std::vector<TH1 *>> &hists);
    void prepareHists(const std::string &histName,
                      int nBinsX,
                      double xLow,
                      double xUp,
                      std::vector<TH1 *> &hists);
    void clearHists(std::vector<std::vector<TH1 *>> &hists);
    void deleteHists(std::vector<std::vector<TH1 *>> &hists);


    void processTimeStamp();
    void processTime();
    void processGammaCh();

    std::vector<std::vector<double>> _timePeaksPos;
    unsigned long _nGamma;
    unsigned long _nAlpha;

    void calculateTimePeaksPos(std::vector<std::vector<TH1 *> > &hists);
    double calculateTimePeakPos(TH1 *hist) const;

    class TimePeakFitFunctionObject
    {
    public:
        TimePeakFitFunctionObject(){}

        double operator() (double *x, double *par) {
           double arg{0};
           if (par[2] != 0.0)
           {
               arg = ( x[0] - par[1] ) / par[2];
           }
           double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
           return fitval;
       }
    };
    TimePeakFitFunctionObject _timePeakFitFunctionObject;
    class AmpPeakFitFunctionObject
    {
    public:
        AmpPeakFitFunctionObject(){}

        double operator() (double *x, double *par) {
           double arg{0};
           if (par[2] != 0.0)
           {
               arg = ( x[0] - par[1] ) / par[2];
           }
           double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
           return fitval;
       }
    };
    AmpPeakFitFunctionObject _ampPeakFitFunctionObject;
};

#endif // CALIBRATION_H
