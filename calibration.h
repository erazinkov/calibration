#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <functional>

#include <TH1.h>
#include <TMath.h>

#include "adcm_df.h"
#include "channelmap.h"

class Calibration
{
public:
    Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events);
    void process();

    static inline constexpr int BINS_TIME{400};
    static inline constexpr int BINS_CHANNEL{400};
    static inline constexpr int BINS_ENERGY{640};

    static inline constexpr double XLOW_TIME{-100.0};
    static inline constexpr double XLOW_CHANNEL{0.0};
    static inline constexpr double XLOW_ENERGY{0.0};

    static inline constexpr double XUP_TIME{100.0};
    static inline constexpr double XUP_CHANNEL{4.0e3};
    static inline constexpr double XUP_ENERGY{8.0e3};

private:

    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t ig, u_int8_t ia);

    void fillHistTime(const std::vector<dec_ev_t> &, TH1 *, double);
    void fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude, TF1 f);

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
    void clearHists(std::vector<TH1 *> &hists);
    void deleteHists(std::vector<std::vector<TH1 *>> &hists);
    void deleteHists(std::vector<TH1 *> &hists);

    void processTimeStamp();
    void processTime();
    void processGammaCh();
    void processGammaEnergy();

    std::vector<std::vector<double>> _timePeaksPos;
    std::vector<std::vector<double>> _par;
    unsigned long _nGamma;
    unsigned long _nAlpha;

    void calculateTimePeaksPos(std::vector<std::vector<TH1 *> > &hists);
    double calculateTimePeakPos(TH1 *hist) const;

    class TimePeakFitFunctionObject
    {
    public:
        TimePeakFitFunctionObject(){}

        double operator() (double *x, double *par) {
           double arg_1{0.0}, arg_2{0.0}, arg_3{0.0}, arg_4{0.0};
           if (par[2] != 0.0 && par[5] != 0.0 && par[8] != 0.0)
           {
               arg_1 = ( x[0] - par[1] ) / par[2];
               arg_2 = ( x[0] - ( par[1] + par[4] ) ) / par[5];
               arg_3 = ( x[0] - ( par[1] + par[7] ) ) / par[8];
               arg_4 = x[0];
           }

           double fitval{
               par[0] * TMath::Exp( -0.5 * arg_1 * arg_1 ) +
               par[3] * TMath::Exp( -0.5 * arg_2 * arg_2 ) +
               par[6] * TMath::Exp( -0.5 * arg_3 * arg_3 ) +
               par[9] + par[10] * arg_4
           };

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
