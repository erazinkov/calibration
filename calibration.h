#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "adcm_df.h"
#include "channelmap.h"

#include "TH1.h"

class Calibration
{
public:
    Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events);
    void process();

private:
    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t ig, u_int8_t ia);
    void fillHist(const std::vector<dec_ev_t> &events, TH1 *h, double(Calibration::*f)(const dec_ev_t &event));
    void fillHistsAsync(std::vector<std::vector<TH1 *>> hists, double(Calibration::*f)(const dec_ev_t &event));
    void drawHistsToFile(const std::string &psName, std::vector<std::vector<TH1 *>> hists);
    void prepareHists(const std::string &histName,
                      int nBinsX,
                      double xLow,
                      double xUp,
                      std::vector<std::vector<TH1 *>> &hists);
    void clearHists(std::vector<std::vector<TH1 *>> &hists);
    void processTime();
    void processGammaAmp();

    double valueTime(const dec_ev_t &event);
    double valueGammaAmp(const dec_ev_t &event);

    std::vector<std::vector<double>> _timePeaksPos;
    unsigned long _nGamma;
    unsigned long _nAlpha;

};

#endif // CALIBRATION_H
