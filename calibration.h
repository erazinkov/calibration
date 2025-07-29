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

class Calibration
{
public:
    Calibration(const ChannelMap &map, std::vector<dec_ev_t> &events);
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
    std::unique_ptr<TimePeaksFinder> _timePeaksFinder;
    const ChannelMap _map;
    const std::vector<dec_ev_t> _events;

    std::vector<dec_ev_t> selectedEvents(uint8_t idxGamma, u_int8_t idxAlpha);

    void fillHistTime(const std::vector<dec_ev_t> &events, TH1 *h, double);
    void fillHistChannel(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude);
    void fillHistEnergy(const std::vector<dec_ev_t> &events, TH1 *h, double min, double max, bool exclude, TF1 f);

    void drawHistsToFile(const std::string &psName, const std::vector<std::vector<std::shared_ptr<TH1>> > &hists) const;
    std::vector<std::vector<std::shared_ptr<TH1>>> prepareHists(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                std::vector<int> &idxsGamma,
                                                                std::vector<int> &idxsAlpha);
    std::vector<std::shared_ptr<TH1>> prepareHists(const std::string &histName,
                                                                int nBinsX,
                                                                double xLow,
                                                                double xUp,
                                                                std::vector<int> &idxs);
    void processTime();
    void processGammaCh();
    void processGammaEnergy();

    std::vector<std::vector<EnergyPeak>>  _energyPeaks;

    std::vector<int> _idxsGamma;
    std::vector<int> _idxsAlpha;


};

#endif // CALIBRATION_H
