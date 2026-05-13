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

template<typename T>
class Calibration
{
public:
    Calibration(const std::string &fileName, const ChannelMap &map, std::vector<T> &events)
        : fileName_(fileName), map_(map), _events(events)
    {
        _idxsGamma = map.getIdxsByType(Channel::GAMMA);
        _idxsAlpha = map.getIdxsByType(Channel::ALPHA);
        timePeaksFinder_ = std::make_unique<TimePeaksFinder>(map_);
        _histogramManager = std::make_unique<HistogramManager>();
    }
    virtual ~Calibration() {}

protected:
    static inline constexpr int BINS_TIME{800};
    static inline constexpr int BINS_CHANNEL{400};
    static inline constexpr int BINS_ENERGY{640};

    static inline constexpr double XLOW_TIME{-200.0};
    static inline constexpr double XLOW_CHANNEL{0.0};
    static inline constexpr double XLOW_ENERGY{0.0};

    static inline constexpr double XUP_TIME{200.0};
    static inline constexpr double XUP_CHANNEL{4.0e3};
    static inline constexpr double XUP_ENERGY{8.0e3};

    const std::vector<T> _events;
    std::string fileName_;
    std::unique_ptr<TimePeaksFinder> timePeaksFinder_;
    const ChannelMap map_;
    std::unique_ptr<HistogramManager> _histogramManager;
    virtual std::vector<T> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha) = 0;
    virtual void fillHistTime(const std::vector<T> &events, TH1 *h, double) = 0;

//    virtual void fillHistTimeWithEnergyCut(const std::vector<T> &events,
//                                   TH1 *h,
//                                   double offsetT,
//                                   double minE,
//                                   double maxE,
//                                   bool exclude,
//                                   TF1 f);

//    virtual void fillHistEnergyTime(const std::vector<T> &events,
//                            TH2 *h,
//                            double offsetT,
//                            TF1 f);

//    void fillHistChannel(const std::vector<dec_ev_2p_t> &events, TH1 *h, double minT, double maxT, bool exclude);
//    void fillHistEnergy(const std::vector<dec_ev_2p_t> &events, TH1 *h, double minT, double maxT, bool exclude, TF1 f);

//    virtual void processTime() = 0;
//    void processTimeWithEnergyCut();
//    void processGammaCh();
//    void processGammaEnergy();
//    void processGammaEnergyTime();

    std::vector<std::vector<EnergyPeak>>  _energyPeaks;

    std::vector<int> _idxsGamma;
    std::vector<int> _idxsAlpha;

};

#endif // CALIBRATION_H
