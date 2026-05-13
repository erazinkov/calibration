#ifndef CALIBRATION2P_H
#define CALIBRATION2P_H

#include <functional>
#include <future>

#include <TH1.h>
#include <TMath.h>

#include "adcm_df.h"
#include "channelmap.h"

#include "timepeaksfinder.h"
#include "energypeak.h"
#include "histogrammanager.h"
#include "calibration.h"

class Calibration2p : public Calibration<dec_ev_2p_t>
{
public:
    Calibration2p(const std::string &fileName, const ChannelMap &map, std::vector<dec_ev_2p_t> &events)
        : Calibration(fileName, map, events) {
        process();
    };
//    ~Calibration2p();

private:
    std::vector<dec_ev_2p_t> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) override;
    void process();
    void fillHistTime(const std::vector<dec_ev_2p_t> &events, TH1 *h, double);

    void fillHistTimeWithEnergyCut(const std::vector<dec_ev_2p_t> &events,
                                   TH1 *h,
                                   double offsetT,
                                   double minE,
                                   double maxE,
                                   bool exclude,
                                   TF1 f);

    void fillHistEnergyTime(const std::vector<dec_ev_2p_t> &events,
                            TH2 *h,
                            double offsetT,
                            TF1 f);

    void fillHistChannel(const std::vector<dec_ev_2p_t> &events, TH1 *h, double minT, double maxT, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_2p_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f);


    void processTime();
    void processTimeWithEnergyCut();
    void processGammaCh();
    void processGammaEnergy();
    void processGammaEnergyTime();

};

#endif // CALIBRATION2P_H
