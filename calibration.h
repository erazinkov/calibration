#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <functional>
#include <future>

#include <TH1.h>
#include <TMath.h>

#include "adcm_df.h"
#include "channelmap.h"

#include "timepeaksfinder.h"
#include "energypeak.h"
#include "histogrammanager.h"

class Calibration
{
public:
    Calibration(const std::string &fileName, const ChannelMap &map, std::vector<dec_ev_t> &events);
    ~Calibration();
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
    std::string fileName_;
//    std::unique_ptr<TimePeaksFinder> _timePeaksFinder;
    std::unique_ptr<TimePeaksFinder> timePeaksFinder_;
    std::unique_ptr<HistogramManager> _histogramManager;
    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha);

    void fillHistTime(const std::vector<dec_ev_t> &events, TH1 *h, double);

    void fillHistTimeWithEnergyCut(const std::vector<dec_ev_t> &events,
                                   TH1 *h,
                                   double offsetT,
                                   double minE,
                                   double maxE,
                                   bool exclude,
                                   TF1 f);

    void fillHistEnergyTime(const std::vector<dec_ev_t> &events,
                            TH2 *h,
                            double offsetT,
                            TF1 f);

    void fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f);

    void processTime();
    void processTimeWithEnergyCut();
    void processGammaCh();
    void processGammaEnergy();
    void processGammaEnergyTime();

    std::vector<std::vector<EnergyPeak>>  _energyPeaks;

    std::vector<int> _idxsGamma;
    std::vector<int> _idxsAlpha;


};

#endif // CALIBRATION_H
